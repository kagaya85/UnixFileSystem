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

#define MIN_DISK_SIZE 4096
#define DISK_FILE_NAME "myDisk.img"
/*
 * 格式化myDisk.img
 */ 

class Format
{
private:
    int f_fd;
    int f_dsize; // 磁盘大小 Byte
    const int f_minSize = MIN_DISK_SIZE; // 最小磁盘大小 Byte
    const int f_bsize = 4096;  // 盘块大小
public:
    Format(int diskSize);
    ~Format();
    void InitSuperBolck();
    void InitDiskInode();
    void InitData();
};

Format::Format(int diskSize)
{
    if(diskSize < f_minSize)
        f_dsize = f_minSize;
    else
        f_dsize = diskSize;

    f_fd = open(DISK_FILE_NAME, O_BINARY|O_CREAT|O_WRONLY);
}

Format::~Format()
{
    close(f_fd);
}

void Format::InitSuperBolck()
{

}

void Format::InitDiskInode()
{

}

void Format::InitData()
{

}

#endif // FORMAT_H