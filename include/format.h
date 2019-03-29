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
#include <cstdio>

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

Format::Format(int diskSize)
{
    if(diskSize < FileSystem::MIN_DISK_SIZE)
        f_dsize = FileSystem::MIN_DISK_SIZE;
    else
        f_dsize = diskSize;

    int remain = f_dsize - 3;
    
    /* 分配初始Inode Bitmap
     * 每个目录分别占用1个inode
     * 0号inode空白
     * 共需5+1个inode, 5个数据块存放目录文件
     */
    f_init_num = 5;

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
    spb.s_ndfree = f_dzone_size - f_init_num;    // 直接管理的空闲盘快数
    spb.s_ninode = f_izone_size * FileSystem::INODE_NUMBER_PER_SECTOR - f_init_num - 1;   // 直接管理的空闲外存inode数量， -1 为0号inode

    spb.s_flock = 0;    // 封锁空闲盘块索引表标志
    spb.s_ilock = 0;    // 封锁空闲Inode表标志   

    spb.s_fmod = 0;     // 内存中super block被修改标志
    spb.s_ronly = 0;    // 文件系统只读标志
    spb.s_time = 0xAABBCCDD;     // 最近一次更新时间
    spb.s_reserved[21] = 0xFFFFFFFF;    // 填充 

    if(write(f_fd, &spb, FileSystem::BLOCK_SIZE) < FileSystem::BLOCK_SIZE)
    {
        perror("SuperBlock init error!");
    }

}

void Format::InitBitmap()
{
    /* 分配初始Inode Bitmap
     * 包含根目录root以及4个子目录
     * 每个目录分别占用1个inode
     * 0号inode空白
     * 共需5+1个inode, 5个数据块存放目录文件
     */
    char ione[2] = {0xFC, 0x00}; // inodemap预分配6bit
    char done[2] = {0xF8, 0x00}; // datamap预分配5bit
    char zero[4094] = {0};
    
    // lseek(f_fd, FileSystem::BLOCK_SIZE, SEEK_SET);

    // bitmap 清零 
    if(write(f_fd, &ione, sizeof(ione)) < sizeof(ione))
    {
        perror("Bitmap init 1 error");
    }
    if(write(f_fd, zero, sizeof(zero)) < sizeof(zero))
    {
        perror("Bitmap init 2 error");
    }

    if(write(f_fd, &done, sizeof(done)) < sizeof(done))
    {
        perror("Bitmap init 3 error");
    }
    if(write(f_fd, zero, sizeof(zero)) < sizeof(zero))
    {
        perror("Bitmap init 4 error");
    }

}

void Format::InitDiskInode()
{
    /* 初始化5+!个外存inode */
    DiskInode inode;
    
    /* 初始配置 */
    inode.d_mode = Inode::IRWXU | Inode::IFDIR;

    inode.d_nlink = 0;

    inode.d_uid = 0;
    inode.d_gid = 0;

    inode.d_atime = 0;  
    inode.d_mtime = 0;

    inode.d_size = 0;   // 目录文件的size指目录下所有文件的总大小？


    lseek(f_fd, 3 * FileSystem::BLOCK_SIZE, SEEK_SET);

    /* 0号inode不分配盘块, 代表不存在的inode */
    if(write(f_fd, &inode, sizeof(inode)) < 64)
    {
        perror("inode 0 init error");
    }
    /* 5个目录inode */
    for(int i = 0; i < 5; i++)
    {
        inode.d_addr[0] = i;
        if(write(f_fd, &inode, sizeof(inode)) < 64)
        {
            perror("inode init error");
        }
    }

}

void Format::InitData()
{
    // 对0 ~ 4号盘块写入目录文件
    // datazone 初始盘块
    int base_offset = 2 + f_izone_size;
    DirItem root[6] = {
        {".", 1},
        {"..", 1},
        {"bin", 2},
        {"etc", 3},
        {"home", 4},
        {"dev", 5}
    };

    DirItem sub_dir[2] = {
        {".", 2},
        {"..", 1}
    };

    lseek(f_fd, (base_offset + 1) * FileSystem::BLOCK_SIZE, SEEK_SET);
    if(write(f_fd, &root, sizeof(root)) < sizeof(root))
    {
        perror("root dir block write error");
    }

    for(int i = 2; i <= 5; i++)
    {
        lseek(f_fd, (base_offset + i) * FileSystem::BLOCK_SIZE, SEEK_SET);
        sub_dir[1].inode_num = i;   // 子目录的当前inode号
        if(write(f_fd, &sub_dir, sizeof(sub_dir)) < sizeof(sub_dir))
        {
            perror("sub_dir block write error");
        }    
    }
}

#endif // FORMAT_H