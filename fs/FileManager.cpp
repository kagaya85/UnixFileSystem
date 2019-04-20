#include "FileManager.h"
#include "Kernel.h"
#include "Utility.h"
#include "File.h"

/*  定义内存Inode表的实例 */
InodeTable g_InodeTable;
/* 全局对象g_OpenFileTable */
OpenFileTable g_OpenFileTable;

/*==========================class FileManager===============================*/
FileManager::FileManager()
{
	//nothing to do here
}

FileManager::~FileManager()
{
	//nothing to do here
}

void FileManager::Initialize()
{
	this->m_FileSystem = &Kernel::Instance().GetFileSystem();

	this->m_InodeTable = &g_InodeTable;

	this->m_InodeTable->Initialize();

	this->m_OpenFileTable = &g_OpenFileTable;
}

/*
 * 功能：打开文件
 * 效果：建立打开文件结构，内存i节点开锁 、i_count 为正数（i_count ++）
 * */
void FileManager::Open()
{
	Inode* pInode;
	User& u = Kernel::Instance().GetUser();

	pInode = this->NameI(NextChar, FileManager::OPEN);	/* 0 = Open, not create */
	/* 没有找到相应的Inode */
	if ( NULL == pInode )
	{
		return;
	}
	this->Open1(pInode, u.u_arg[1], 0);
}

/*
 * 功能：创建一个新的文件
 * 效果：建立打开文件结构，内存i节点开锁 、i_count 为正数（应该是 1）
 * */
void FileManager::Creat()
{
	Inode* pInode;
	User& u = Kernel::Instance().GetUser();
	unsigned int newACCMode = u.u_arg[1] & (Inode::IRWXU|Inode::IRWXG|Inode::IRWXO);

	/* 搜索目录的模式为1，表示创建；若父目录不可写，出错返回 */
	pInode = this->NameI(NextChar, FileManager::CREATE);
	/* 没有找到相应的Inode，或NameI出错 */
	if ( NULL == pInode )
	{
		if(u.u_error)
			return;
		/* 创建Inode */
		pInode = this->MakNode( newACCMode & (~Inode::ISVTX) );
		/* 创建失败 */
		if ( NULL == pInode )
		{
			std::cerr << "Creat Inode Error." << std::endl;
			return;
		}

		/* 
		 * 如果所希望的名字不存在，使用参数trf = 2来调用open1()。
		 * 不需要进行权限检查，因为刚刚建立的文件的权限和传入参数mode
		 * 所表示的权限内容是一样的。
		 */
		this->Open1(pInode, File::FWRITE, 2);
	}
	else
	{
		/* 如果NameI()搜索到已经存在要创建的文件，则清空该文件（用算法ITrunc()）。UID没有改变
		 * 原来UNIX的设计是这样：文件看上去就像新建的文件一样。然而，新文件所有者和许可权方式没变。
		 * 也就是说creat指定的RWX比特无效。
		 * 邓蓉认为这是不合理的，应该改变。
		 * 现在的实现：creat指定的RWX比特有效 */
		this->Open1(pInode, File::FWRITE, 1);
		pInode->i_mode |= newACCMode;
	}
}

/* 
* trf == 0由open调用
* trf == 1由creat调用，creat文件的时候搜索到同文件名的文件
* trf == 2由creat调用，creat文件的时候未搜索到同文件名的文件，这是文件创建时更一般的情况
* mode参数：打开文件模式，表示文件操作是 读、写还是读写
*/
void FileManager::Open1(Inode* pInode, int mode, int trf)
{
	User& u = Kernel::Instance().GetUser();

	/* 
	 * 对所希望的文件已存在的情况下，即trf == 0或trf == 1进行权限检查
	 * 如果所希望的名字不存在，即trf == 2，不需要进行权限检查，因为刚建立
	 * 的文件的权限和传入的参数mode的所表示的权限内容是一样的。
	 */
	if (trf != 2)
	{
		if ( mode & File::FREAD )
		{
			/* 检查读权限 */
			this->Access(pInode, Inode::IREAD);
		}
		if ( mode & File::FWRITE )
		{
			/* 检查写权限 */
			this->Access(pInode, Inode::IWRITE);
			/* 系统调用去写目录文件是不允许的 */
			if ( (pInode->i_mode & Inode::IFMT) == Inode::IFDIR )
			{
				u.u_error = User::MYEISDIR;
			}
		}
	}

	if ( u.u_error )
	{
		this->m_InodeTable->IPut(pInode);
		return;
	}

	/* 在creat文件的时候搜索到同文件名的文件，释放该文件所占据的所有盘块 */
	if ( 1 == trf )
	{
		pInode->ITrunc();
	}

	/* 解锁inode! 
	 * 线性目录搜索涉及大量的磁盘读写操作，期间进程会入睡。
	 * 因此，进程必须上锁操作涉及的i节点。这就是NameI中执行的IGet上锁操作。
	 * 行至此，后续不再有可能会引起进程切换的操作，可以解锁i节点。
	 */
	pInode->Prele();

	/* 分配打开文件控制块File结构 */
	File* pFile = this->m_OpenFileTable->FAlloc();
	if ( NULL == pFile )
	{
		this->m_InodeTable->IPut(pInode);
		return;
	}
	/* 设置打开文件方式，建立File结构和内存Inode的勾连关系 */
	pFile->f_flag = mode & (File::FREAD | File::FWRITE);
	pFile->f_inode = pInode;

	/* 特殊设备打开函数 */
	// pInode->OpenI(mode & File::FWRITE);

	/* 为打开或者创建文件的各种资源都已成功分配，函数返回 */
	if ( u.u_error == 0 )
	{
		return;
	}
	else	/* 如果出错则释放资源 */
	{
		/* 释放打开文件描述符 */
		int fd = u.u_ar0[User::EAX];
		if(fd != -1)
		{
			u.u_ofiles.SetF(fd, NULL);
			/* 递减File结构和Inode的引用计数 ,File结构没有锁 f_count为0就是释放File结构了*/
			pFile->f_count--;
		}
		this->m_InodeTable->IPut(pInode);
	}
}

void FileManager::Close()
{
	User& u = Kernel::Instance().GetUser();
	int fd = u.u_arg[0];

	/* 获取打开文件控制块File结构 */
	File* pFile = u.u_ofiles.GetF(fd);
	if ( NULL == pFile )
	{
		return;
	}

	/* 释放打开文件描述符fd，递减File结构引用计数 */
	u.u_ofiles.SetF(fd, NULL);
	this->m_OpenFileTable->CloseF(pFile);
}

void FileManager::Seek()
{
	File* pFile;
	User& u = Kernel::Instance().GetUser();
	int fd = u.u_arg[0];

	pFile = u.u_ofiles.GetF(fd);
	if ( NULL == pFile )
	{
		return;  /* 若FILE不存在，GetF有设出错码 */
	}

	/* 管道文件不允许seek */
	if ( pFile->f_flag & File::FPIPE )
	{
		u.u_error = User::MYESPIPE;
		return;
	}

	int offset = u.u_arg[1];

	/* 如果u.u_arg[2]在3 ~ 5之间，那么长度单位由字节变为512字节 */
	if ( u.u_arg[2] > 2 )
	{
		offset = offset << 9;
		u.u_arg[2] -= 3;
	}

	switch ( u.u_arg[2] )
	{
		/* 读写位置设置为offset */
		case 0:
			pFile->f_offset = offset;
			break;
		/* 读写位置加offset(可正可负) */
		case 1:
			pFile->f_offset += offset;
			break;
		/* 读写位置调整为文件长度加offset */
		case 2:
			pFile->f_offset = pFile->f_inode->i_size + offset;
			break;
	}
}

void FileManager::Dup()
{
	File* pFile;
	User& u = Kernel::Instance().GetUser();
	int fd = u.u_arg[0];

	pFile = u.u_ofiles.GetF(fd);
	if ( NULL == pFile )
	{
		return;
	}

	int newFd = u.u_ofiles.AllocFreeSlot();
	if ( newFd < 0 )
	{
		return;
	}
	/* 至此分配新描述符newFd成功 */
	u.u_ofiles.SetF(newFd, pFile);
	pFile->f_count++;
}

void FileManager::FStat()
{
	File* pFile;
	User& u = Kernel::Instance().GetUser();
	int fd = u.u_arg[0];

	pFile = u.u_ofiles.GetF(fd);
	if ( NULL == pFile )
	{
		return;
	}

	/* u.u_arg[1] = pStatBuf */
	this->Stat1(pFile->f_inode, u.u_arg[1]);
}

void FileManager::Stat()
{
	Inode* pInode;
	User& u = Kernel::Instance().GetUser();

	pInode = this->NameI(FileManager::NextChar, FileManager::OPEN);
	if ( NULL == pInode )
	{
		return;
	}
	this->Stat1(pInode, u.u_arg[1]);
	this->m_InodeTable->IPut(pInode);
}

void FileManager::Stat1(Inode* pInode, unsigned long statBuf)
{
	Buf* pBuf;
	BufferManager& bufMgr = Kernel::Instance().GetBufferManager();

	pInode->IUpdate(time(NULL));
	pBuf = bufMgr.Bread(pInode->i_dev, Constant::INODE_ZONE_START_SECTOR + pInode->i_number / Constant::INODE_NUMBER_PER_SECTOR );

	/* 将p指向缓存区中编号为inumber外存Inode的偏移位置 */
	unsigned char* p = pBuf->b_addr + (pInode->i_number % Constant::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode);
	Utility::DWordCopy( (int *)p, (int *)statBuf, sizeof(DiskInode)/sizeof(int) );

	bufMgr.Brelse(pBuf);
}

void FileManager::Read()
{
	/* 直接调用Rdwr()函数即可 */
	this->Rdwr(File::FREAD);
}

void FileManager::Write()
{
	/* 直接调用Rdwr()函数即可 */
	this->Rdwr(File::FWRITE);
}

void FileManager::Rdwr( enum File::FileFlags mode )
{
	File* pFile;
	User& u = Kernel::Instance().GetUser();

	/* 根据Read()/Write()的系统调用参数fd获取打开文件控制块结构 */
	pFile = u.u_ofiles.GetF(u.u_arg[0]);	/* fd */
	if ( NULL == pFile )
	{
		/* 不存在该打开文件，GetF已经设置过出错码，所以这里不需要再设置了 */
		/*	u.u_error = User::EBADF;	*/
		return;
	}


	/* 读写的模式不正确 */
	if ( (pFile->f_flag & mode) == 0 )
	{
		u.u_error = User::MYEACCES;
		return;
	}

	u.u_IOParam.m_Base = (unsigned char *)u.u_arg[1];	/* 目标缓冲区首址 */
	u.u_IOParam.m_Count = u.u_arg[2];		/* 要求读/写的字节数 */
	u.u_segflg = 0;		/* User Space I/O，读入的内容要送数据段或用户栈段 */

	/* 普通文件读写 ，或读写特殊文件。对文件实施互斥访问，互斥的粒度：每次系统调用。
	为此Inode类需要增加两个方法：NFlock()、NFrele()。
	这不是V6的设计。read、write系统调用对内存i节点上锁是为了给实施IO的进程提供一致的文件视图。*/
	pFile->f_inode->NFlock();
	/* 设置文件起始读位置 */
	u.u_IOParam.m_Offset = pFile->f_offset;
	if ( File::FREAD == mode )
	{
		pFile->f_inode->ReadI();
	}
	else
	{
		pFile->f_inode->WriteI();
	}

	/* 根据读写字数，移动文件读写偏移指针 */
	pFile->f_offset += (u.u_arg[2] - u.u_IOParam.m_Count);
	pFile->f_inode->NFrele();

	/* 返回实际读写的字节数，修改存放系统调用返回值的核心栈单元 */
	u.u_ar0[User::EAX] = u.u_arg[2] - u.u_IOParam.m_Count;
}


/* 返回NULL表示目录搜索失败，否则是根指针，指向文件的内存打开i节点 ，上锁的内存i节点  */
Inode* FileManager::NameI( char (*func)(), enum DirectorySearchMode mode )
{
	Inode* pInode;
	Buf* pBuf;
	char curchar;
	char* pChar;
	int freeEntryOffset;	/* 以创建文件模式搜索目录时，记录空闲目录项的偏移量 */
	User& u = Kernel::Instance().GetUser();
	BufferManager& bufMgr = Kernel::Instance().GetBufferManager();

	/* 
	 * 如果该路径是'/'开头的，从根目录开始搜索，
	 * 否则从进程当前工作目录开始搜索。
	 */
	pInode = u.u_cdir;
	if ( '/' == (curchar = (*func)()) )
	{
		pInode = this->rootDirInode;
	}

	/* 检查该Inode是否正在被使用，以及保证在整个目录搜索过程中该Inode不被释放 */
	this->m_InodeTable->IGet(pInode->i_dev, pInode->i_number);

	/* 允许出现////a//b 这种路径 这种路径等价于/a/b */
	while ( '/' == curchar )
	{
		curchar = (*func)();
	}
	/* 如果试图更改和删除当前目录文件则出错 */
	if ( '\0' == curchar && mode != FileManager::OPEN )
	{
		u.u_error = User::MYENOENT;
		goto out;
	}

	/* 外层循环每次处理pathname中一段路径分量 */
	while (true)
	{
		/* 如果出错则释放当前搜索到的目录文件Inode，并退出 */
		if ( u.u_error != User::MYNOERROR )
		{
			break;	/* goto out; */
		}

		/* 整个路径搜索完毕，返回相应Inode指针。目录搜索成功返回。 */
		if ( '\0' == curchar )
		{
#ifdef DEBUG
			std::cout << "Search return ok!" << std::endl;
#endif
			return pInode;
		}

		/* 如果要进行搜索的不是目录文件，释放相关Inode资源则退出 */
		if ( (pInode->i_mode & Inode::IFMT) != Inode::IFDIR )
		{
			u.u_error = User::MYENOTDIR;
			break;	/* goto out; */
		}

		/* 进行目录搜索权限检查,IEXEC在目录文件中表示搜索权限 */
		if ( this->Access(pInode, Inode::IEXEC) )
		{
			u.u_error = User::MYEACCES;
			break;	/* 不具备目录搜索权限，goto out; */
		}

		/* 
		 * 将Pathname中当前准备进行匹配的路径分量拷贝到u.u_dbuf[]中，
		 * 便于和目录项进行比较。
		 */
		pChar = &(u.u_dbuf[0]);
		while ( '/' != curchar && '\0' != curchar && u.u_error == User::MYNOERROR )
		{
			if ( pChar < &(u.u_dbuf[DirectoryEntry::DIRSIZ]) )
			{
				*pChar = curchar;
				pChar++;
			}
			curchar = (*func)();
		}
		/* 将u_dbuf剩余的部分填充为'\0' */
		while ( pChar < &(u.u_dbuf[DirectoryEntry::DIRSIZ]) )
		{
			*pChar = '\0';
			pChar++;
		}

		/* 允许出现////a//b 这种路径 这种路径等价于/a/b */
		while ( '/' == curchar )
		{
			curchar = (*func)();
		}

		if ( u.u_error != User::MYNOERROR )
		{
			break; /* goto out; */
		}

		/* 内层循环部分对于u.u_dbuf[]中的路径名分量，逐个搜寻匹配的目录项 */
		u.u_IOParam.m_Offset = 0;
		/* 设置为目录项个数 ，含空白的目录项*/
		u.u_IOParam.m_Count = pInode->i_size / (DirectoryEntry::DIRSIZ + 4);
		freeEntryOffset = 0;
		pBuf = NULL;
#ifdef DEBUG
		std::cout << "Start find dir: " << u.u_dbuf << std::endl;
#endif
		while (true)
		{
			/* 对目录项已经搜索完毕 */
			if ( 0 == u.u_IOParam.m_Count )
			{
#ifdef DEBUG
				std::cout << "Dir: " << u.u_dbuf << " not find." << std::endl;
#endif
				if ( NULL != pBuf )
				{
					bufMgr.Brelse(pBuf);
				}
				/* 如果是创建新文件 */
				if ( FileManager::CREATE == mode && curchar == '\0' )
				{
					/* 判断该目录是否可写 */
					if ( this->Access(pInode, Inode::IWRITE) )
					{
						u.u_error = User::MYEACCES;
						goto out;	/* Failed */
					}

					/* 将父目录Inode指针保存起来，以后写目录项WriteDir()函数会用到 */
					u.u_pdir = pInode;

					if ( freeEntryOffset )	/* 此变量存放了空闲目录项位于目录文件中的偏移量 */
					{
						/* 将空闲目录项偏移量存入u区中，写目录项WriteDir()会用到 */
						u.u_IOParam.m_Offset = freeEntryOffset - (DirectoryEntry::DIRSIZ + 4);
					}
					else  /*问题：为何if分支没有置IUPD标志？  这是因为文件的长度没有变呀*/
					{
						pInode->i_flag |= Inode::IUPD;
					}
					/* 找到可以写入的空闲目录项位置，NameI()函数返回 */
					return NULL;
				}
				
				/* 目录项搜索完毕而没有找到匹配项，释放相关Inode资源，并推出 */
				u.u_error = User::MYENOENT;
				goto out;
			}

			/* 已读完目录文件的当前盘块，需要读入下一目录项数据盘块 */
			if ( 0 == u.u_IOParam.m_Offset % Inode::BLOCK_SIZE )
			{
				if ( NULL != pBuf )
				{
					bufMgr.Brelse(pBuf);
				}
				/* 计算要读的物理盘块号 */
				int phyBlkno = pInode->Bmap(u.u_IOParam.m_Offset / Inode::BLOCK_SIZE );
				pBuf = bufMgr.Bread(pInode->i_dev, phyBlkno );
			}

			/* 没有读完当前目录项盘块，则读取下一目录项至u.u_dent */
			int* src = (int *)(pBuf->b_addr + (u.u_IOParam.m_Offset % Inode::BLOCK_SIZE));
			Utility::DWordCopy( src, (int *)&u.u_dent, sizeof(DirectoryEntry)/sizeof(int) );

			u.u_IOParam.m_Offset += (DirectoryEntry::DIRSIZ + 4);
			u.u_IOParam.m_Count--;

			/* 如果是空闲目录项，记录该项位于目录文件中偏移量 */
			if ( 0 == u.u_dent.m_ino )
			{
				if ( 0 == freeEntryOffset )
				{
					freeEntryOffset = u.u_IOParam.m_Offset;
				}
				/* 跳过空闲目录项，继续比较下一目录项 */
				continue;
			}
#ifdef DEBUG
			std::cout << "Compare with " << u.u_dent.m_name << std::endl;
#endif
			int i;
			for ( i = 0; i < DirectoryEntry::DIRSIZ; i++ )
			{
				if ( u.u_dbuf[i] != u.u_dent.m_name[i] )
				{
					break;	/* 匹配至某一字符不符，跳出for循环 */
				}
				else if ( u.u_dbuf[i] == '\0')
				{
					// 防止 \0 后出现随机字符导致匹配失败
					i = DirectoryEntry::DIRSIZ;
					break;
				}
			}

			if( i < DirectoryEntry::DIRSIZ )
			{
				/* 不是要搜索的目录项，继续匹配下一目录项 */
				continue;
			}
			else
			{
				/* 目录项匹配成功，回到外层While(true)循环 */
#ifdef DEBUG
				std::cout << "Dir: " << u.u_dbuf << " finded." << std::endl;
#endif
				break;
			}
		}

		/* 
		 * 从内层目录项匹配循环跳至此处，说明pathname中
		 * 当前路径分量匹配成功了，还需匹配pathname中下一路径
		 * 分量，直至遇到'\0'结束。
		 */
		if ( NULL != pBuf )
		{
			bufMgr.Brelse(pBuf);
		}

		/* 如果是删除操作，则返回父目录Inode，而要删除文件的Inode号在u.u_dent.m_ino中 */
		if ( FileManager::DELETE == mode && '\0' == curchar )
		{
			/* 如果对父目录没有写的权限 */
			if ( this->Access(pInode, Inode::IWRITE) )
			{
				u.u_error = User::MYEACCES;
				break;	/* goto out; */
			}
			return pInode;
		}

		/* 
		 * 匹配目录项成功，则释放当前目录Inode，根据匹配成功的
		 * 目录项m_ino字段获取相应下一级目录或文件的Inode。
		 */
		short dev = pInode->i_dev;
		this->m_InodeTable->IPut(pInode);
		pInode = this->m_InodeTable->IGet(dev, u.u_dent.m_ino);
		/* 回到外层While(true)循环，继续匹配Pathname中下一路径分量 */
#ifdef DEBUG
		pInode->IInfo();
#endif
		if ( NULL == pInode )	/* 获取失败 */
		{
			return NULL;
		}
	}
out:
	this->m_InodeTable->IPut(pInode);
	return NULL;
}

char FileManager::NextChar()
{
	User& u = Kernel::Instance().GetUser();
	
	/* u.u_dirp指向pathname中的字符 */
	return *u.u_dirp++;
}

/* 由creat调用。
 * 为新创建的文件写新的i节点和新的目录项
 * 返回的pInode是上了锁的内存i节点，其中的i_count是 1。
 *
 * 在程序的最后会调用 WriteDir，在这里把属于自己的目录项写进父目录，修改父目录文件的i节点 、将其写回磁盘。
 *
 */
Inode* FileManager::MakNode( unsigned int mode )
{
	Inode* pInode;
	User& u = Kernel::Instance().GetUser();

	/* 分配一个空闲DiskInode，里面内容已全部清空 */
	pInode = this->m_FileSystem->IAlloc(u.u_pdir->i_dev);
	if( NULL ==	pInode )
	{
		std::cerr << "No enough Inode" << std::endl;
		return NULL;
	}

	pInode->i_flag |= (Inode::IACC | Inode::IUPD);
	pInode->i_mode = mode;
	pInode->i_nlink = 1;
	pInode->i_uid = u.u_uid;
	pInode->i_gid = u.u_gid;
#ifdef DEBUG
	std::cout << "Create a new Inode." << std::endl;
	pInode->IInfo();
#endif
	/* 将目录项写入u.u_dent，随后写入目录文件 */
	this->WriteDir(pInode);
	return pInode;
}

void FileManager::WriteDir( Inode* pInode )
{
	User& u = Kernel::Instance().GetUser();

	/* 设置目录项中Inode编号部分 */
	u.u_dent.m_ino = pInode->i_number;

	/* 设置目录项中pathname分量部分 */
	for ( int i = 0; i < DirectoryEntry::DIRSIZ; i++ )
	{
		u.u_dent.m_name[i] = u.u_dbuf[i];
	}

	u.u_IOParam.m_Count = DirectoryEntry::DIRSIZ + 4;
	u.u_IOParam.m_Base = (unsigned char *)&u.u_dent;
	u.u_segflg = 1;

	/* 将目录项写入父目录文件 */
	u.u_pdir->WriteI();
	this->m_InodeTable->IPut(u.u_pdir);
}

void FileManager::SetCurDir(char* pathname)
{
	User& u = Kernel::Instance().GetUser();
	char* pChar = NULL;
	char curchar = pathname[0];
	/* 路径不是从根目录'/'开始，则在现有u.u_curdir后面加上当前路径分量 */
	if ( pathname[0] != '/' )
	{
		int length = Utility::StringLength(u.u_curdir);
		while(true)
		{
			/* 
			* 将Pathname中当前准备进行匹配的路径分量拷贝到u.u_dbuf[]中，
			*/
			pChar = &(u.u_dbuf[0]);
			while ( '/' != curchar && '\0' != curchar)
			{
				if ( pChar < &(u.u_dbuf[DirectoryEntry::DIRSIZ]) )
				{
					*pChar = curchar;
					pChar++;
				}
				curchar = *(++pathname);
			}
			/* 将u_dbuf剩余的部分填充为'\0' */
			while ( pChar < &(u.u_dbuf[DirectoryEntry::DIRSIZ]) )
			{
				*pChar = '\0';
				pChar++;
			}
			/* 允许出现////a//b 这种路径 这种路径等价于/a/b */
			while ( '/' == curchar )
			{
				curchar = *(pathname++);
			}

			if (u.u_dbuf[0] == '.' && u.u_dbuf[1] == '\0')	// cd .
				continue;	// 路径不变，取下一个分路径
			else if(u.u_dbuf[0] == '.' && u.u_dbuf[1] == '.' && u.u_dbuf[2] == '\0') // cd ..
			{
				// 退回到上级路径
				while(u.u_curdir[length - 1] != '/')
					length--;
				if (length > 1)	// 如果遇到的不是根目录，则要去掉 '/‘
					length--;
				
				continue;	// 寻找下一分路径
			}

			if ( u.u_dbuf[0] && u.u_curdir[length - 1] != '/' )	// 有路径追加 且 当前路径不以 / 结尾，补充 /
			{
				u.u_curdir[length] = '/';
				length++;
			}

			Utility::StringCopy(u.u_dbuf, u.u_curdir + length);
			if (curchar == '\0')
				break;	// end
		}
	}
	else	/* 如果是从根目录'/'开始，则取代原有工作目录 */
	{
		Utility::StringCopy(pathname, u.u_curdir);
	}
}

/*
 * 返回值是0，表示拥有打开文件的权限；1表示没有所需的访问权限。文件未能打开的原因记录在u.u_error变量中。
 */
int FileManager::Access( Inode* pInode, unsigned int mode )
{
	User& u = Kernel::Instance().GetUser();

	/* 对于写的权限，必须检查该文件系统是否是只读的 */
	if ( Inode::IWRITE == mode )
	{
		if( this->m_FileSystem->GetFS(pInode->i_dev)->s_ronly != 0 )
		{
			u.u_error = User::MYEROFS;
			std::cerr << "File system read only" << std::endl;
			return 1;
		}
	}
	/* 
	 * 对于超级用户，读写任何文件都是允许的
	 * 而要执行某文件时，必须在i_mode有可执行标志
	 */
	if ( u.u_uid == 0 )
	{
		if ( Inode::IEXEC == mode && ( pInode->i_mode & (Inode::IEXEC | (Inode::IEXEC >> 3) | (Inode::IEXEC >> 6)) ) == 0 )
		{
			u.u_error = User::MYEACCES;
			return 1;
		}
		return 0;	/* Permission Check Succeed! */
	}
	if ( u.u_uid != pInode->i_uid )
	{
		mode = mode >> 3;
		if ( u.u_gid != pInode->i_gid )
		{
			mode = mode >> 3;
		}
	}
	if ( (pInode->i_mode & mode) != 0 )
	{
		return 0;
	}

	u.u_error = User::MYEACCES;
	return 1;
}

Inode* FileManager::Owner()
{
	Inode* pInode;
	User& u = Kernel::Instance().GetUser();

	if ( (pInode = this->NameI(NextChar, FileManager::OPEN)) == NULL )
	{
		return NULL;
	}

	if ( u.u_uid == pInode->i_uid || u.SUser() )
	{
		return pInode;
	}

	this->m_InodeTable->IPut(pInode);
	return NULL;
}

void FileManager::ChMod()
{
	Inode* pInode;
	User& u = Kernel::Instance().GetUser();
	unsigned int mode = u.u_arg[1];

	if ( (pInode = this->Owner()) == NULL )
	{
		return;
	}
	/* clear i_mode字段中的ISGID, ISUID, ISTVX以及rwxrwxrwx这12比特 */
	pInode->i_mode &= (~0xFFF);
	/* 根据系统调用的参数重新设置i_mode字段 */
	pInode->i_mode |= (mode & 0xFFF);
	pInode->i_flag |= Inode::IUPD;

	this->m_InodeTable->IPut(pInode);
	return;
}

void FileManager::ChOwn()
{
	Inode* pInode;
	User& u = Kernel::Instance().GetUser();
	short uid = u.u_arg[1];
	short gid = u.u_arg[2];

	/* 不是超级用户或者不是文件主则返回 */
	if ( !u.SUser() || (pInode = this->Owner()) == NULL )
	{
		return;
	}
	pInode->i_uid = uid;
	pInode->i_gid = gid;
	pInode->i_flag |= Inode::IUPD;

	this->m_InodeTable->IPut(pInode);
}

void FileManager::ChDir()
{
	Inode* pInode;
	User& u = Kernel::Instance().GetUser();

	pInode = this->NameI(FileManager::NextChar, FileManager::OPEN);
	if ( NULL == pInode )
	{
		return;
	}
	/* 搜索到的文件不是目录文件 */
	if ( (pInode->i_mode & Inode::IFMT) != Inode::IFDIR )
	{
		u.u_error = User::MYENOTDIR;
		this->m_InodeTable->IPut(pInode);
		return;
	}
	if ( this->Access(pInode, Inode::IEXEC) )
	{
		this->m_InodeTable->IPut(pInode);
		return;
	}
	this->m_InodeTable->IPut(u.u_cdir);
	u.u_cdir = pInode;
	pInode->Prele();

	this->SetCurDir((char *)u.u_arg[0] /* pathname */);
}

void FileManager::Link()
{
	Inode* pInode;
	Inode* pNewInode;
	User& u = Kernel::Instance().GetUser();

	pInode = this->NameI(FileManager::NextChar, FileManager::OPEN);
	/* 打卡文件失败 */
	if ( NULL == pInode )
	{
		return;
	}
	/* 链接的数量已经最大 */
	if ( pInode->i_nlink >= 255 )
	{
		u.u_error = User::MYEMLINK;
		/* 出错，释放资源并退出 */
		this->m_InodeTable->IPut(pInode);
		return;
	}
	/* 对目录文件的链接只能由超级用户进行 */
	if ( (pInode->i_mode & Inode::IFMT) == Inode::IFDIR && !u.SUser() )
	{
		/* 出错，释放资源并退出 */
		this->m_InodeTable->IPut(pInode);
		return;
	}

	/* 解锁现存文件Inode,以避免在以下搜索新文件时产生死锁 */
	pInode->i_flag &= (~Inode::ILOCK);
	/* 指向要创建的新路径newPathname */
	u.u_dirp = (char *)u.u_arg[1];
	pNewInode = this->NameI(FileManager::NextChar, FileManager::CREATE);
	/* 如果文件已存在 */
	if ( NULL != pNewInode )
	{
		u.u_error = User::MYEEXIST;
		this->m_InodeTable->IPut(pNewInode);
	}
	if ( User::MYNOERROR != u.u_error )
	{
		/* 出错，释放资源并退出 */
		this->m_InodeTable->IPut(pInode);
		return;
	}
	/* 检查目录与该文件是否在同一个设备上 */
	if ( u.u_pdir->i_dev != pInode->i_dev )
	{
		this->m_InodeTable->IPut(u.u_pdir);
		u.u_error = User::MYEXDEV;
		/* 出错，释放资源并退出 */
		this->m_InodeTable->IPut(pInode);
		return;
	}

	this->WriteDir(pInode);
	pInode->i_nlink++;
	pInode->i_flag |= Inode::IUPD;
	this->m_InodeTable->IPut(pInode);
}

void FileManager::UnLink()
{
	Inode* pInode;
	Inode* pDeleteInode;
	User& u = Kernel::Instance().GetUser();

	pDeleteInode = this->NameI(FileManager::NextChar, FileManager::DELETE);
	if ( NULL == pDeleteInode )
	{
		return;
	}
	pDeleteInode->Prele();

	pInode = this->m_InodeTable->IGet(pDeleteInode->i_dev, u.u_dent.m_ino);
	if ( NULL == pInode )
	{
		Utility::Panic("unlink -- iget");
	}
	/* 只有root可以unlink目录文件 */
	if ( (pInode->i_mode & Inode::IFMT) == Inode::IFDIR && !u.SUser() )
	{
		this->m_InodeTable->IPut(pDeleteInode);
		this->m_InodeTable->IPut(pInode);
		return;
	}
	/* 写入清零后的目录项 */
	u.u_IOParam.m_Offset -= (DirectoryEntry::DIRSIZ + 4);
	u.u_IOParam.m_Base = (unsigned char *)&u.u_dent;
	u.u_IOParam.m_Count = DirectoryEntry::DIRSIZ + 4;
	
	u.u_dent.m_ino = 0;
	pDeleteInode->WriteI();

	/* 修改inode项 */
	pInode->i_nlink--;
	pInode->i_flag |= Inode::IUPD;

	this->m_InodeTable->IPut(pDeleteInode);
	this->m_InodeTable->IPut(pInode);
}

void FileManager::MkNod()
{
	Inode* pInode;
	User& u = Kernel::Instance().GetUser();

	/* 检查uid是否是root，该系统调用只有uid==root时才可被调用 */
	if ( u.SUser() )
	{
		pInode = this->NameI(FileManager::NextChar, FileManager::CREATE);
		/* 要创建的文件已经存在,这里并不能去覆盖此文件 */
		if ( pInode != NULL )
		{
			u.u_error = User::MYEEXIST;
			this->m_InodeTable->IPut(pInode);
			return;
		}
	}
	else
	{
		/* 非root用户执行mknod()系统调用返回User::EPERM */
		u.u_error = User::MYEPERM;
		return;
	}
	/* 没有通过SUser()的检查 */
	if ( User::MYNOERROR != u.u_error )
	{
		return;	/* 没有需要释放的资源，直接退出 */
	}
	pInode = this->MakNode(u.u_arg[1]);
	if ( NULL == pInode )
	{
		return;
	}
	/* 所建立是设备文件 */
	if ( (pInode->i_mode & (Inode::IFBLK | Inode::IFCHR)) != 0 )
	{
		pInode->i_addr[0] = u.u_arg[2];
	}
	this->m_InodeTable->IPut(pInode);
}
/*==============================class OpenFileTable===================================*/

OpenFileTable::OpenFileTable()
{
	//nothing to do here
}

OpenFileTable::~OpenFileTable()
{
	//nothing to do here
}
/*作用：进程打开文件描述符表中找的空闲项之下标写入 u_ar0[EAX]*/
File* OpenFileTable::FAlloc()
{
	int fd;
	User& u = Kernel::Instance().GetUser();

	/* 在进程打开文件描述符表中获取一个空闲项 */
	fd = u.u_ofiles.AllocFreeSlot();

	if(fd < 0)	/* 如果寻找空闲项失败 */
	{
		return NULL;
	}

	for(int i = 0; i < OpenFileTable::NFILE; i++)
	{
		/* f_count==0表示该项空闲 */
		if(this->m_File[i].f_count == 0)
		{
			/* 建立描述符和File结构的勾连关系 */
			u.u_ofiles.SetF(fd, &this->m_File[i]);
			/* 增加对file结构的引用计数 */
			this->m_File[i].f_count++;
			/* 清空文件读、写位置 */
			this->m_File[i].f_offset = 0;
			return (&this->m_File[i]);
		}
	}

	std::cerr << "No Free File Struct\n" << std::endl;
	u.u_error = User::MYENFILE;
	return NULL;
}

void OpenFileTable::CloseF(File *pFile)
{
	Inode* pNode;

	/* 管道类型 */
	if(pFile->f_flag & File::FPIPE)
	{
		pNode = pFile->f_inode;
		pNode->i_mode &= ~(Inode::IREAD | Inode::IWRITE);
	}

	if(pFile->f_count <= 1)
	{
		/* 
		 * 如果当前进程是最后一个引用该文件的进程，
		 * 对特殊块设备、字符设备文件调用相应的关闭函数
		 */
		pFile->f_inode->CloseI(pFile->f_flag & File::FWRITE);
		g_InodeTable.IPut(pFile->f_inode);
	}

	/* 引用当前File的进程数减1 */
	pFile->f_count--;
}

/*==========================class DirectoryEntry===============================*/
DirectoryEntry::DirectoryEntry()
{
	this->m_ino = 0;
	this->m_name[0] = '\0';
}

DirectoryEntry::~DirectoryEntry()
{
	//nothing to do here
}

