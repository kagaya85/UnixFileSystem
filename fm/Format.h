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

#include <fcntl.h>
#include <cstdio>
#include <sys/types.h>    
#include <sys/stat.h>
#include <unistd.h>

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
    int f_init_num; // 初始化目录文件数量
public:
    Format(int diskSize);
    ~Format();
    void InitSuperBolck();
    void InitDiskInode();
    void InitBitmap();
    void InitData();
};


#endif // FORMAT_H