/*
 * File: format.h
 * Project: format
 * File Created: Monday, 25th March 2019 8:14:43 pm
 * Author: kagaya (kagaya85@outlook.com)
 * -----
 * Last Modified: Wednesday, 27th March 2019 9:05:12 pm
 * Modified By: kagaya (kagaya85@outlook.com>)
 */

/* 
 * 将myDisk.img文件按UNIX V6++ Volume格式化
 * 包含根目录以及bin、etc、home、dev四个子目录
 */

#ifndef FORMAT_H
#define FORMAT_H

#include "FileSystem.h"
#include <fcntl.h>
#include <io.h>
#include <iostream>

#define DISK_FILE_NAME "myDisk.img"
/*
 * 格式化myDisk.img
 * (0) (1) (2) (3 ~ ) ()
 * (SuperBlock) (DataBitmap) (InodeBitmap) (InodeZone) (DataBlocks+)
 */ 

class Format
{
private:
    int f_fd;
    int f_dsize; // 磁盘大小 Byte
    int f_izone_size; // Inode区 块数量
    int f_dzone_size;    // Data数据区块数量

public:
    Format(int diskSize);
    ~Format();
    void InitSuperBolck();
    void InitDiskInode();
    void InitBitmap();
    void InitData();
};

Format::Format(int diskSize)
{
    if(diskSize < FileSystem::MIN_DISK_SIZE)
        f_dsize = FileSystem::MIN_DISK_SIZE;
    else
        f_dsize = diskSize;

    // 每256个数据块分配一个inode块
    int remain = f_dsize - 3;

    f_izone_size = (remain / 257) + 1;  // 每256个数据块分配一个inode区块,对应64个inode
    f_dzone_size = remain - f_izone_size;

    f_fd = open(DISK_FILE_NAME, O_BINARY|O_CREAT|O_WRONLY);
}

Format::~Format()
{
    close(f_fd);
}

/*
 * 初始化超级快
 */

void Format::InitSuperBolck()
{
    int ret;
    SuperBlock spb; // 1024 Bytes

    spb.s_isize = f_izone_size; 
    spb.s_dsize = f_dzone_size;
    spb.s_fsize = f_dsize;  // 盘块总数

    spb.s_dstart = f_dsize - f_dzone_size;
    spb.s_ndfree = f_dzone_size;    // 直接管理的空闲盘快数
    spb.s_ninode = f_izone_size * FileSystem::INODE_NUMBER_PER_SECTOR;   // 直接管理的空闲外存inode数量

    spb.s_flock = 0;    // 封锁空闲盘块索引表标志
    spb.s_ilock = 0;    // 封锁空闲Inode表标志   

    spb.s_fmod = 0;     // 内存中super block被修改标志
    spb.s_ronly = 0;    // 文件系统只读标志
    spb.s_time = 0xAABBCCDD;     // 最近一次更新时间
    spb.s_reserved[21] = 0x473C2B1A;    // 填充 

    if(write(f_fd, &spb, FileSystem::BLOCK_SIZE) < FileSystem::BLOCK_SIZE)
    {
        std::cout << "SuperBlock init error!" << endl;
    }

}

void Format::InitBitmap()
{
    int zero[1024] = {0};
    
    // bitmap 清零 seek一下？
    if(write(f_fd, zero, FileSystem::BLOCK_SIZE) < FileSystem::BLOCK_SIZE)
    {
        std::cout << "Data Bitmap init error!" << endl;
    }
    if(write(f_fd, zero, FileSystem::BLOCK_SIZE) < FileSystem::BLOCK_SIZE)
    {
        std::cout << "Inode Bitmap init error!" << endl;
    }

    // 给目录分配5个inode和5个数据块

}

void Format::InitDiskInode()
{
    // 初始化5个inode
}

void Format::InitData()
{
    // 初始化5个目录文件
}

#endif // FORMAT_H