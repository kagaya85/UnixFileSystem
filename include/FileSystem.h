#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "INode.h"
#include "Buf.h"
#include "BufferManager.h"
#include "SuperBlock.h"
#include "Defines.h"

/*
 * 文件系统装配块(Mount)的定义。
 * 装配块用于实现子文件系统与
 * 根文件系统的连接。
 */
class Mount
{
	/* Functions */
public:
	/* Constructors */
	Mount();
	/* Destructors */
	~Mount();
	
	/* Members */
public:
	short 		m_dev;		/* 文件系统设备号 */
	SuperBlock* m_spb;		/* 指向文件系统的Super Block对象在内存中的副本 */
	Inode*		m_inodep;	/* 指向挂载子文件系统的内存INode */
	
	unsigned char* db_addr;	/* 指向数据 bitmap的缓冲区的首地址 */
	unsigned char* ib_addr;	/* 指向inode bitmap的缓冲区的首地址 */
};

/*
 * 文件系统类(FileSystem)管理文件存储设备中
 * 的各类存储资源，磁盘块、外存INode的分配、
 * 释放。
 */
class FileSystem
{
public:
	/* static consts */
	static const int NMOUNT = 5;			/* 系统中用于挂载子文件系统的装配块数量 */

	static const int MIN_DISK_SIZE = 4096;	// 磁盘最少盘块数量 256*4k=1M  16M~128M
	static const int BLOCK_SIZE = 4096;	// 盘块大小 BYTE 4K

	static const int SUPER_BLOCK_SECTOR_NUMBER = 0;	/* 定义SuperBlock位于磁盘上的扇区号 */

	static const int ROOTINO = 0;			/* 文件系统根目录外存Inode编号 */

	static const int INODE_NUMBER_PER_SECTOR = 64;		/* 外存INode对象长度为64字节，每个磁盘块可以存放4096/64 = 64个外存Inode */
	static const int INODE_ZONE_START_SECTOR = 3;		/* 外存Inode区位于磁盘上的起始扇区号 */
	// static const int INODE_ZONE_SIZE = 512 - INODE_ZONE_START_SECTOR;		/* 磁盘上外存Inode区占据的扇区数 */

	// static const int DATA_ZONE_START_SECTOR = 512;		/* 数据区的起始扇区号 */
	// static const int DATA_ZONE_END_SECTOR = 4096 - 1;	/* 数据区的结束扇区号 */
	// static const int DATA_ZONE_SIZE = 4096 - DATA_ZONE_START_SECTOR;	/* 数据区占据的扇区数量 */

	/* Functions */
public:
	/* Constructors */
	FileSystem();
	/* Destructors */
	~FileSystem();
	/* 
	 * @comment 初始化成员变量
	 */
	void Initialize();

	/* 
	* @comment 系统初始化时读入SuperBlock
	*/
	void LoadSuperBlock();

	/* 
	 * @comment 根据文件存储设备的设备号dev获取
	 * 该文件系统的SuperBlock
	 */
	SuperBlock* GetFS(short dev);
	/* 
	 * @comment 将SuperBlock对象的内存副本更新到
	 * 存储设备的SuperBlock中去
	 */
	void Update();

	/* 
	 * @comment  在存储设备dev上分配一个空闲
	 * 外存INode，一般用于创建新的文件。
	 */
	Inode* IAlloc(short dev);
	/* 
	 * @comment  释放存储设备dev上编号为number
	 * 的外存INode，一般用于删除文件。
	 */
	void IFree(short dev, int number);

	/* 
	 * @comment 在存储设备dev上分配空闲磁盘块
	 */
	Buf* Alloc(short dev);
	/* 
	 * @comment 释放存储设备dev上编号为blkno的磁盘块
	 */
	void Free(short dev, int blkno);

	/* 
	 * @comment 查找文件系统装配表，搜索指定Inode对应的Mount装配块
	 */
	Mount* GetMount(Inode* pInode);

	/**
	 * bitmap写入磁盘
	 */
	void SaveBitmap(int bmp);

	/**
	 * 读入bitmap到缓存
	 */
	unsigned char* LoadBimap(int bmp);

	/**
	 * 找到第一个为0得bit位，置1，返回块号，失败返回-1
	 */
	int AllocFreeBit(unsigned char* bitmap);

	/**
	 * 将指定块号对应得bit位值位
	 */
	void setBitmap(unsigned char* bitmap, int num, bool bit);

private:
	/* 
	 * @comment 检查设备dev上编号blkno的磁盘块是否属于
	 * 数据盘块区
	 */
	bool BadBlock(SuperBlock* spb, short dev, int blkno);

public:
	Mount m_Mount[NMOUNT];		/* 文件系统装配块表，Mount[0]用于根文件系统 */

private:
	BufferManager* m_BufferManager;		/* FileSystem类需要缓存管理模块(BufferManager)提供的接口 */

	int updlock;				/* Update()函数的锁，该函数用于同步内存各个SuperBlock副本以及，
								被修改过的内存Inode。任一时刻只允许一个进程调用该函数 */
};

/* 
 * 内存Inode表(class InodeTable)
 * 负责内存Inode的分配和释放。
 */
class InodeTable
{
	/* static consts */
public:
	static const int NINODE	= 100;	/* 内存Inode的数量 */
	
	/* Functions */
public:
	/* Constructors */
	InodeTable();
	/* Destructors */
	~InodeTable();
	
	/* 
	 * @comment 初始化对g_FileSystem对象的引用
	 */
	void Initialize();
	/* 
	 * @comment 根据指定设备号dev，外存Inode编号获取对应
	 * Inode。如果该Inode已经在内存中，对其上锁并返回该内存Inode，
	 * 如果不在内存中，则将其读入内存后上锁并返回该内存Inode
	 */
	Inode* IGet(short dev, int inumber);
	/* 
	 * @comment 减少该内存Inode的引用计数，如果此Inode已经没有目录项指向它，
	 * 且无进程引用该Inode，则释放此文件占用的磁盘块。
	 */
	void IPut(Inode* pNode);

	/* 
	 * @comment 将所有被修改过的内存Inode更新到对应外存Inode中
	 */
	void UpdateInodeTable();
	
	/* 
	 * @comment 检查设备dev上编号为inumber的外存inode是否有内存拷贝，
	 * 如果有则返回该内存Inode在内存Inode表中的索引
	 */
	int IsLoaded(short dev, int inumber);
	/* 
	 * @comment 在内存Inode表中寻找一个空闲的内存Inode
	 */
	Inode* GetFreeInode();
	
	/* Members */
public:
	Inode m_Inode[NINODE];		/* 内存Inode数组，每个打开文件都会占用一个内存Inode */

	FileSystem* m_FileSystem;	/* 对全局对象g_FileSystem的引用 */
}; 


#endif
