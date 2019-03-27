/*
 * File: format.h
 * Project: format
 * File Created: Monday, 25th March 2019 8:14:43 pm
 * Author: kagaya (kagaya85@outlook.com)
 * -----
 * Last Modified: Monday, 25th March 2019 8:21:45 pm
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
 * (0 - 1) (2 - 511) (512 - 4095+)
 * (SuperBlock) (InodeZone) (DataZone+)
 */ 

class Format
{
private:
    int f_fd;
    int f_dsize; // 磁盘大小 Byte

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

    spb.s_isize = FileSystem::INODE_ZONE_SIZE;    // inode区占用四个盘块
    spb.s_fsize = f_dsize;  // 盘块总数

    spb.s_nfree;    // 直接管理的空闲盘快数
    spb.s_free;     // 空闲盘块索引表

    spb.s_ninode;   // 直接管理的空闲外存inode数量
    spb.s_inode;      // 空闲ionde缩影

    spb.s_flock = 0;    // 封锁空闲盘块索引表标志
    spb.s_ilock = 0;    // 封锁空闲Inode表标志   

    spb.s_fmod;     // 内存中super block被修改标志
    spb.s_ronly = 0;    // 文件系统只读标志
    spb.s_time = 0xAABBCCDD;     // 最近一次更新时间
    spb.padding[46] = 0x473C2B1A;    // 填充 

    if(write(f_fd, &spb, 1024) < 1024)
    {
        std::cout << "SuperBlock init error!" << endl;
    }

}

void Format::InitBitmap()
{
    
}

void Format::InitDiskInode()
{

}

void Format::InitData()
{

}

#endif // FORMAT_H