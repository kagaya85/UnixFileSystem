#include "../include/FileSystem.h"
#include "Kernel.h"
#include "BufferManager.h"
#include <time.h>


/*==============================class Mount===================================*/
Mount::Mount()
{
	this->m_dev = -1;
	this->m_spb = NULL;
	this->m_inodep = NULL;
	this->db_addr = NULL;
	this->ib_addr = NULL;
}

Mount::~Mount()
{
	this->m_dev = -1;
	this->m_inodep = NULL;
	//释放内存SuperBlock副本
	if(this->m_spb != NULL)
	{
		delete this->m_spb;
		this->m_spb = NULL;
	}

	if(this->db_addr != NULL)
	{
		delete this->db_addr;
		this->db_addr = NULL;
	}

	if(this->ib_addr != NULL)
	{
		delete this->ib_addr;
		this->ib_addr = NULL;
	}
}


/*==============================class FileSystem===================================*/
FileSystem::FileSystem()
{
	//nothing to do here
}

FileSystem::~FileSystem()
{
	//nothing to do here
}

void FileSystem::Initialize()
{
	this->m_BufferManager = &Kernel::Instance().GetBufferManager();
	this->updlock = 0;
}

void FileSystem::LoadSuperBlock()
{
	User& u = Kernel::Instance().GetUser();
	BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
	Buf* pBuf;

	int* p = (int *)&g_spb;

	pBuf = bufMgr.Bread(DiskDriver::ROOTDEV, Constant::SUPER_BLOCK_SECTOR_NUMBER);
	Utility::DWordCopy((int *)pBuf->b_addr, p, 32);	// 我的super block 只有128个字节
	bufMgr.Brelse(pBuf);

	if (User::ErrorCode::MYNOERROR != u.u_error)
	{
		Utility::Panic("Load SuperBlock Error....!\n");
	}

	this->m_Mount[0].m_dev = DiskDriver::ROOTDEV;
	this->m_Mount[0].m_spb = &g_spb;

	g_spb.s_flock = 0;
	g_spb.s_ilock = 0;
	g_spb.s_ronly = 0;
	g_spb.s_time = time(0);
}

SuperBlock* FileSystem::GetFS(short dev)
{
	SuperBlock* sb;
	
	/* 遍历系统装配块表，寻找设备号为dev的设备中文件系统的SuperBlock */
	for(int i = 0; i < Constant::NMOUNT; i++)
	{
		if(this->m_Mount[i].m_spb != NULL && this->m_Mount[i].m_dev == dev)
		{
			/* 获取SuperBlock的地址 */
			sb = this->m_Mount[i].m_spb;
			return sb;
		}
	}

	Utility::Panic("No File System!");
	return NULL;
}

void FileSystem::Update()
{
	int i;
	SuperBlock* sb;
	Buf* pBuf;

	/* 另一进程正在进行同步，则直接返回 */
	if(this->updlock)
	{
		return;
	}

	/* 设置Update()函数的互斥锁，防止其它进程重入 */
	this->updlock++;

	/* 同步SuperBlock到磁盘 */
	for(i = 0; i < Constant::NMOUNT; i++)
	{
		if(this->m_Mount[i].m_spb != NULL)	/* 该Mount装配块对应某个文件系统 */
		{
			sb = this->m_Mount[i].m_spb;

			/* 如果该SuperBlock内存副本没有被修改，直接管理inode和空闲盘块被上锁或该文件系统是只读文件系统 */
			if(sb->s_fmod == 0 || sb->s_ilock != 0 || sb->s_flock != 0 || sb->s_ronly != 0)
			{
				continue;
			}

			/* 清SuperBlock修改标志 */
			sb->s_fmod = 0;
			/* 写入SuperBlock最后存访时间 */
			sb->s_time = time(0);

			/* 
			 * 为将要写回到磁盘上去的SuperBlock申请一块缓存，由于缓存块大小为512字节，
			 * SuperBlock大小为1024字节，占据2个连续的扇区，所以需要2次写入操作。
			 */
			int* p = (int *)sb;

			/* 将要写入到设备dev上的SUPER_BLOCK_SECTOR_NUMBER + j扇区中去 */
			pBuf = this->m_BufferManager->GetBlk(this->m_Mount[i].m_dev, Constant::SUPER_BLOCK_SECTOR_NUMBER);

			/* 将SuperBlock中第0 - 128字节写入缓存区 */
			Utility::DWordCopy(p, (int *)pBuf->b_addr, 32);

			/* 将缓冲区中的数据写到磁盘上 */
			this->m_BufferManager->Bwrite(pBuf);
		}
	}
	
	/* 同步修改过的内存Inode到对应外存Inode */
	g_InodeTable.UpdateInodeTable();

	/* 清除Update()函数锁 */
	this->updlock = 0;

	/* 将延迟写的缓存块写到磁盘上 */
	this->m_BufferManager->Bflush(DiskDriver::NODEV);

	/* 将bitmap写回磁盘 */
	this->SaveBitmap(DiskDriver::DATA_BITMAP_BLOCK);
	this->SaveBitmap(DiskDriver::INODE_BITMAP_BLOCK);
}

Inode* FileSystem::IAlloc(short dev)
{
	SuperBlock* sb;
	Buf* pBuf;
	Inode* pNode;
	User& u = Kernel::Instance().GetUser();
	int ino;	/* 分配到的空闲外存Inode编号 */

	/* 获取相应设备的SuperBlock内存副本 */
	sb = this->GetFS(dev);

	/* 
	 * 上面部分已经保证，除非系统中没有可用外存Inode，
	 * 否则空闲Inode索引表中必定会记录可用外存Inode的编号。
	 */
	while(true)
	{
		/* 从bitmap获取空闲外存Inode编号 */
		ino = this->AllocFreeBit(this->m_Mount[0].ib_addr);
		if(ino <= 0)
		{
			std::cerr << "inode alloc error" << std::endl;
			exit(-1);
		}
		/* 将空闲Inode读入内存 */
		pNode = g_InodeTable.IGet(dev, ino);
		/* 未能分配到内存inode */
		if(NULL == pNode)
		{
			return NULL;
		}

		/* 如果该Inode空闲,清空Inode中的数据 */
		if(0 == pNode->i_mode)
		{
			pNode->Clean();
			/* 设置SuperBlock被修改标志 */
			sb->s_fmod = 1;
			sb->s_nifree--;
			return pNode;
		}
		else	/* 如果该Inode已被占用，使用bitmap理论上不应该有这种情况 */	
		{
			g_InodeTable.IPut(pNode);
			this->setBitmap(this->m_Mount[0].ib_addr, ino, 1);	// 将对应位置1
			continue;	/* while循环 */
		}
	}
	return NULL;	/* GCC likes it! */
}

void FileSystem::IFree(short dev, int number)
{
	SuperBlock* sb;

	sb = this->GetFS(dev);	/* 获取相应设备的SuperBlock内存副本 */
	
	/* 
	 * 如果超级块直接管理的空闲Inode表上锁，
	 * 则释放的外存Inode散落在磁盘Inode区中。
	 */
	if(sb->s_ilock)
	{
		return;
	}
	
	this->setBitmap(this->m_Mount[0].ib_addr, number, 0);	// 将对应位置0
	sb->s_nifree++;

	/* 设置SuperBlock被修改标志 */
	sb->s_fmod = 1;
}

Buf* FileSystem::Alloc(short dev)
{
	int blkno;	/* 分配到的空闲磁盘块编号 */
	SuperBlock* sb;
	Buf* pBuf;
	User& u = Kernel::Instance().GetUser();

	/* 获取SuperBlock对象的内存副本 */
	sb = this->GetFS(dev);

	/* 
	 * 如果空闲磁盘块索引表正在被上锁，表明有其它进程
	 * 正在操作空闲磁盘块索引表，因而对其上锁。这通常
	 * 是由于其余进程调用Free()或Alloc()造成的。
	 */
	if(sb->s_flock)
	{
		/* 进入睡眠直到获得该锁才继续 */
		// u.u_procp->Sleep((unsigned long)&sb->s_flock, ProcessManager::PINOD);
		std::cerr << "FileSystem::Alloc sb->s_flock is true!" << std::endl;
		exit(-1);
	}

	/* 从bitmap获取空闲磁盘块编号 */
	blkno = AllocFreeBit(this->m_Mount[0].db_addr);
	/* 
	 * 若获取磁盘块编号小于0，则表示已分配尽所有的空闲磁盘块。
	 * 或者分配到的空闲磁盘块编号不属于数据盘块区域中(由BadBlock()检查)，
	 * 都意味着分配空闲磁盘块操作失败。
	 */
	if(blkno < 0)	// 0号磁盘块在用 0号inode不用
	{
		sb->s_ndfree = 0;
		std::cerr << "No Space On "<< dev << " !" << std::endl;
		u.u_error = User::ErrorCode::MYENOSPC;
		return NULL;
	}
	if( this->BadBlock(sb, dev, blkno) )
	{
		return NULL;
	}
	sb->s_ndfree--;


	/* 普通情况下成功分配到一空闲磁盘块 */
	pBuf = this->m_BufferManager->GetBlk(dev, blkno);	/* 为该磁盘块申请缓存 */
	this->m_BufferManager->ClrBuf(pBuf);	/* 清空缓存中的数据 */
	sb->s_fmod = 1;	/* 设置SuperBlock被修改标志 */

	return pBuf;
}

void FileSystem::Free(short dev, int blkno)
{
	SuperBlock* sb;
	Buf* pBuf;
	User& u = Kernel::Instance().GetUser();

	sb = this->GetFS(dev);

	/* 
	 * 尽早设置SuperBlock被修改标志，以防止在释放
	 * 磁盘块Free()执行过程中，对SuperBlock内存副本
	 * 的修改仅进行了一半，就更新到磁盘SuperBlock去
	 */
	sb->s_fmod = 1;

	/* 如果空闲磁盘块索引表被上锁，则睡眠等待解锁 */
	while(sb->s_flock)
	{
		std::cerr << "FileSystem::Free sb->s_flock is true!" << std::endl;
		exit(-1);
	}

	/* 检查释放磁盘块的合法性 */
	if(this->BadBlock(sb, dev, blkno))
	{
		return;
	}

	this->setBitmap(this->m_Mount[0].db_addr, blkno, 0);	
	sb->s_ndfree++;	/* SuperBlock中空闲盘快数+1 */
	sb->s_fmod = 1;
}

Mount* FileSystem::GetMount(Inode *pInode)
{
	/* 遍历系统的装配块表 */
	for(int i = 0; i <= Constant::NMOUNT; i++)
	{
		Mount* pMount = &(this->m_Mount[i]);

		/* 找到内存Inode对应的Mount装配块 */
		if(pMount->m_inodep == pInode)
		{
			return pMount;
		}
	}
	return NULL;	/* 查找失败 */
}

void FileSystem::SaveBitmap(int type)
{
	DiskDriver& driver = Kernel::Instance().GetDiskDriver();
	Buf tmp;
	if(type == DiskDriver::INODE_BITMAP_BLOCK)
	{
		tmp.b_addr = this->m_Mount[0].ib_addr;
		tmp.b_blkno = DiskDriver::INODE_BITMAP_BLOCK;
		driver.WriteToDisk(&tmp);
	}
	else if(type == DiskDriver::INODE_BITMAP_BLOCK)
	{
		tmp.b_addr = this->m_Mount[0].db_addr;
		tmp.b_blkno = DiskDriver::DATA_BITMAP_BLOCK;
		driver.WriteToDisk(&tmp);
	}
	else
	{
		std::cerr << "Bitmap save type error" << std::endl;
		exit(-1);
	}
	
}

unsigned char* FileSystem::LoadBimap(int type)
{
	DiskDriver& driver = Kernel::Instance().GetDiskDriver();
	Buf tmp;
	if(type == DiskDriver::INODE_BITMAP_BLOCK)
	{
		// inode bitmap
		if(this->m_Mount[0].ib_addr == NULL)
		{
			this->m_Mount[0].ib_addr = new(std::nothrow) unsigned char[Constant::BLOCK_SIZE];
			if(this->m_Mount[0].ib_addr == NULL)
			{
				std::cerr << "Load bitmap error" << std::endl;
				exit(-1);
			}
		}
		tmp.b_addr = this->m_Mount[0].ib_addr;
		tmp.b_blkno = DiskDriver::INODE_BITMAP_BLOCK;
		driver.ReadFromDisk(&tmp);
	}
	else if(type == DiskDriver::DATA_BITMAP_BLOCK)
	{
		// data bitmap
		if(this->m_Mount[0].db_addr == NULL)
		{
			this->m_Mount[0].db_addr = new(std::nothrow) unsigned char[Constant::BLOCK_SIZE];
			if(this->m_Mount[0].db_addr == NULL)
			{
				std::cerr << "Load bitmap error" << std::endl;
				exit(-1);
			}
		}
		tmp.b_addr = this->m_Mount[0].db_addr;
		tmp.b_blkno = DiskDriver::DATA_BITMAP_BLOCK;
		driver.ReadFromDisk(&tmp);
	}
	else
	{
		std::cerr << "Bitmap load type error" << std::endl;
		exit(-1);
	}
}

int AllocFreeBit(unsigned char* bitmap)
{
	unsigned char* p = bitmap;
	unsigned long long* lp = (unsigned long long*)p;

	int i, j;
	for(i = 0; i < (Constant::BLOCK_SIZE / sizeof(long long)); i++, lp++)
	{
		if(~(*lp) != 0)
			break;	// 有空位
	}

	if(i == (Constant::BLOCK_SIZE / sizeof(long long)))
		return -1;

	// 找出空字节
	p = (unsigned char*)lp;
	for(j = 0; i < sizeof(long long); j++, p++)
	{
		if(~(*p) != 0)
			break;
	}

	if(j == sizeof(long long))
		return -1;

	int offset = 0;

	for(offset = 0; offset < sizeof(char); offset++)
	{
		if((*p & (1 << offset)) == 0)
			return j * 8 * 8 + i * 8 + offset;
	}

	std::cerr << "Alloc free bit error" << std::endl;
	return -1;
}

void setBitmap(unsigned char* bitmap, int num, bool bit)
{
	unsigned char* p = bitmap;
	int offset;
	
	p += (num / sizeof(unsigned char));
	offset = num % sizeof(unsigned char);
	if(bit)
		*p |= (1 << offset);
	else
		*p &= ~(1 << offset);

	return;
}

bool FileSystem::BadBlock(SuperBlock *spb, short dev, int blkno)
{
	return 0;
}

/*==============================class InodeTable===================================*/

InodeTable::InodeTable()
{
	//nothing to do here
}

InodeTable::~InodeTable()
{
	//nothing to do here
}

void InodeTable::Initialize()
{
	/* 获取对g_FileSystem的引用 */
	this->m_FileSystem = &Kernel::Instance().GetFileSystem();
}

Inode* InodeTable::IGet(short dev, int inumber)
{
	Inode* pInode;
	User& u = Kernel::Instance().GetUser();

	while(true)
	{
		/* 检查指定设备dev中编号为inumber的外存Inode是否有内存拷贝 */
		int index = this->IsLoaded(dev, inumber);
		if(index >= 0)	/* 找到内存拷贝 */
		{
			pInode = &(this->m_Inode[index]);
			/* 如果该内存Inode被上锁 */
			if( pInode->i_flag & Inode::ILOCK )
			{
				/* 增设IWANT标志，然后睡眠 */
				pInode->i_flag |= Inode::IWANT;
				
				/* 回到while循环，需要重新搜索，因为该内存Inode可能已经失效 */
				continue;
			}

			/* 如果该内存Inode用于连接子文件系统，查找该Inode对应的Mount装配块 */
			if( pInode->i_flag & Inode::IMOUNT )
			{
				Mount* pMount = this->m_FileSystem->GetMount(pInode);
				if(NULL == pMount)
				{
					/* 没有找到 */
					Utility::Panic("No Mount Tab...");
				}
				else
				{
					/* 将参数设为子文件系统设备号、根目录Inode编号 */
					dev = pMount->m_dev;
					inumber = Constant::ROOTINO;
					/* 回到while循环，以新dev，inumber值重新搜索 */
					continue;
				}
			}

			/* 
			 * 程序执行到这里表示：内存Inode高速缓存中找到相应内存Inode，
			 * 增加其引用计数，增设ILOCK标志并返回之
			 */
			pInode->i_count++;
			pInode->i_flag |= Inode::ILOCK;
			return pInode;
		}
		else	/* 没有Inode的内存拷贝，则分配一个空闲内存Inode */
		{
			pInode = this->GetFreeInode();
			/* 若内存Inode表已满，分配空闲Inode失败 */
			if(NULL == pInode)
			{
				std::cerr << "Inode Table Overflow !" << std::endl;
				u.u_error = User::ErrorCode::MYENFILE;
				return NULL;
			}
			else	/* 分配空闲Inode成功，将外存Inode读入新分配的内存Inode */
			{
				/* 设置新的设备号、外存Inode编号，增加引用计数，对索引节点上锁 */
				pInode->i_dev = dev;
				pInode->i_number = inumber;
				pInode->i_flag = Inode::ILOCK;
				pInode->i_count++;
				pInode->i_lastr = -1;

				BufferManager& bm = Kernel::Instance().GetBufferManager();
				/* 将该外存Inode读入缓冲区 */
				Buf* pBuf = bm.Bread(dev, Constant::INODE_ZONE_START_SECTOR + inumber / Constant::INODE_NUMBER_PER_SECTOR );

				/* 如果发生I/O错误 */
				if(pBuf->b_flags & Buf::B_ERROR)
				{
					/* 释放缓存 */
					bm.Brelse(pBuf);
					/* 释放占据的内存Inode */
					this->IPut(pInode);
					return NULL;
				}

				/* 将缓冲区中的外存Inode信息拷贝到新分配的内存Inode中 */
				pInode->ICopy(pBuf, inumber);
				/* 释放缓存 */
				bm.Brelse(pBuf);
				return pInode;
			}
		}
	}
	return NULL;	/* GCC likes it! */
}

/* close文件时会调用Iput
 *      主要做的操作：内存i节点计数 i_count--；若为0，释放内存 i节点、若有改动写回磁盘
 * 搜索文件途径的所有目录文件，搜索经过后都会Iput其内存i节点。路径名的倒数第2个路径分量一定是个
 *   目录文件，如果是在其中创建新文件、或是删除一个已有文件；再如果是在其中创建删除子目录。那么
 *   	必须将这个目录文件所对应的内存 i节点写回磁盘。
 *   	这个目录文件无论是否经历过更改，我们必须将它的i节点写回磁盘。
 * */
void InodeTable::IPut(Inode *pNode)
{
	/* 当前进程为引用该内存Inode的唯一进程，且准备释放该内存Inode */
	if(pNode->i_count == 1)
	{
		/* 
		 * 上锁，因为在整个释放过程中可能因为磁盘操作而使得该进程睡眠，
		 * 此时有可能另一个进程会对该内存Inode进行操作，这将有可能导致错误。
		 */
		pNode->i_flag |= Inode::ILOCK;

		/* 该文件已经没有目录路径指向它 */
		if(pNode->i_nlink <= 0)
		{
			/* 释放该文件占据的数据盘块 */
			pNode->ITrunc();
			pNode->i_mode = 0;
			/* 释放对应的外存Inode */
			this->m_FileSystem->IFree(pNode->i_dev, pNode->i_number);
		}

		/* 更新外存Inode信息 */
		pNode->IUpdate(time(NULL));

		/* 解锁内存Inode，并且唤醒等待进程 */
		pNode->Prele();
		/* 清除内存Inode的所有标志位 */
		pNode->i_flag = 0;
		/* 这是内存inode空闲的标志之一，另一个是i_count == 0 */
		pNode->i_number = -1;
	}

	/* 减少内存Inode的引用计数，唤醒等待进程 */
	pNode->i_count--;
	pNode->Prele();
}

void InodeTable::UpdateInodeTable()
{
	for(int i = 0; i < InodeTable::NINODE; i++)
	{
		/* 
		 * 如果Inode对象没有被上锁，即当前未被其它进程使用，可以同步到外存Inode；
		 * 并且count不等于0，count == 0意味着该内存Inode未被任何打开文件引用，无需同步。
		 */
		if( (this->m_Inode[i].i_flag & Inode::ILOCK) == 0 && this->m_Inode[i].i_count != 0 )
		{
			/* 将内存Inode上锁后同步到外存Inode */
			this->m_Inode[i].i_flag |= Inode::ILOCK;
			this->m_Inode[i].IUpdate(time(NULL));

			/* 对内存Inode解锁 */
			this->m_Inode[i].Prele();
		}
	}
}

int InodeTable::IsLoaded(short dev, int inumber)
{
	/* 寻找指定外存Inode的内存拷贝 */
	for(int i = 0; i < InodeTable::NINODE; i++)
	{
		if( this->m_Inode[i].i_dev == dev && this->m_Inode[i].i_number == inumber && this->m_Inode[i].i_count != 0 )
		{
			return i;	
		}
	}
	return -1;
}

Inode* InodeTable::GetFreeInode()
{
	for(int i = 0; i < InodeTable::NINODE; i++)
	{
		/* 如果该内存Inode引用计数为零，则该Inode表示空闲 */
		if(this->m_Inode[i].i_count == 0)
		{
			return &(this->m_Inode[i]);
		}
	}
	return NULL;	/* 寻找失败 */
}

