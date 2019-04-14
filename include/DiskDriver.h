/*
 * 主要负责对模拟的磁盘文件进行基本的IO读写操作 
 */
#ifndef DISKDRIVER
#define DISKDRIVER

#include <stdlib.h>
#include <Buf.h>
#include <io.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include "Defines.h"

/**
 * 块设备表devtab定义 
 */
class Devtab
{
public:
	Devtab();
	~Devtab();
	
public:
	int	d_active;
	int	d_errcnt;
	Buf* b_forw;	// 指向最后IO扇区
	Buf* b_back;
	Buf* d_actf;	// IO请求队列队首
	Buf* d_actl;
};

/**
 * 提供块设备随机读写操作
 */
class DiskDriver
{
private:
    int d_diskfileFd;   // 磁盘文件fd
public:
	enum BITMAP_TYPE {DATA_BITMAP_BLOCK, INODE_BITMAP_BLOCK};

	static const int ROOTDEV = 0;			/* 根目录物理盘块号 */
	static const int DATA_BITMAP_BLOCK = 1;	/* 数据 bitmap 物理盘块号 */
	static const int INODE_BITMAP_BLOCK = 2;	/* inode bitmap 物理盘块号 */

	Devtab*	d_tab;		/* 指向块设备表的指针 */

public:
    DiskDriver();
    ~DiskDriver();
	int Initialize();
    int ReadFromDisk(Buf* bp);
    int WriteToDisk(Buf* bp);
};

#endif DISKDRIVER