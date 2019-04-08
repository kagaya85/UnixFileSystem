#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "INode.h"

/* 
 * 目录项结构体 32 Bytes 
 */
struct DirItem 
{
	char name[28];	// 文件名
	int inode_num;	// inode号
};

/*
 * 文件系统存储资源管理块(Super Block)的定义。
 */
class SuperBlock
{
	/* Functions */
public:
	/* Constructors */
	SuperBlock();
	/* Destructors */
	~SuperBlock();
	
	/* Members */
public:
	int		s_isize;		/* Inode区占用的盘块数 */
	int		s_dsize;		/* data区占用盘块数 */
	int		s_fsize;		/* 盘块总数 */
	
	int		s_dstart;		/* 数据区起始盘块 */
	int		s_ndfree;		/* 直接管理的空闲数据块块数量 */
	int		s_ninode;		/* 直接管理的空闲外存Inode数量 */
	
	int		s_flock;		/* 封锁空闲盘块索引表标志 */
	int		s_ilock;		/* 封锁空闲Inode表标志 */
	
	int		s_fmod;			/* 内存中super block副本被修改标志，意味着需要更新外存对应的Super Block */
	int		s_ronly;		/* 本文件系统只能读出 */
	int		s_time;			/* 最近一次更新时间 */
	
	int		s_reserved[21];	/* 填充使SuperBlock块大小等于128字节，占据1个扇区 */
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

private:

public:
	Mount m_Mount[NMOUNT];		/* 文件系统装配块表，Mount[0]用于根文件系统 */

private:
	int updlock;				/* Update()函数的锁，该函数用于同步内存各个SuperBlock副本以及，
								被修改过的内存Inode。任一时刻只允许一个进程调用该函数 */
};

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
};

#endif
