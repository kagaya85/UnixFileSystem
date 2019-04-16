/*
 * File: format.cpp
 * Project: format
 * File Created: Monday, 25th March 2019 8:14:32 pm
 * Author: kagaya (kagaya85@outlook.com)
 * -----
 * Last Modified: Monday, 25th March 2019 8:22:16 pm
 * Modified By: kagaya (kagaya85@outlook.com>)
 */

#include "Format.h"
#include "Defines.h"
#include "SuperBlock.h"
#include "INode.h"
#include <iostream>
#include <iomanip>

using namespace std;

int main()
{
    /* 盘块数量限制为4096~134217728，对于磁盘大小为16M~128M */
    int diskSize = Constant::MIN_DISK_SIZE;   
    while (true)
    {
        cout << "Please Input Disk Block Number (default "<< Constant::MIN_DISK_SIZE <<" x 4096 Bytes):";
        if(cin.peek() != '\n')
            cin >> diskSize;
        if(diskSize < Constant::MIN_DISK_SIZE)
            cout << "Disk size at least 4096 x 4096 Bytes!!!" << endl;
        else
            break;        
    }
    

    Format Disk(diskSize);

    Disk.InitSuperBolck();
    Disk.InitBitmap();
    Disk.InitDiskInode();
    Disk.InitData();

    cout << "Diskfile (" << DISK_FILE_NAME << ") format success" << endl;
    return 0;
}

Format::Format(int diskSize)
{
    if(diskSize < Constant::MIN_DISK_SIZE)
        f_dsize = Constant::MIN_DISK_SIZE;
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

    f_fd = open(DISK_FILE_NAME, O_CREAT|O_WRONLY);
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
    spb.s_nifree = f_izone_size * Constant::INODE_NUMBER_PER_SECTOR - f_init_num - 1;   // 直接管理的空闲外存inode数量， -1 为0号inode

    spb.s_flock = 0;    // 封锁空闲盘块索引表标志
    spb.s_ilock = 0;    // 封锁空闲Inode表标志   

    spb.s_fmod = 0;     // 内存中super block被修改标志
    spb.s_ronly = 0;    // 文件系统只读标志
    spb.s_time = 0xAABBCCDD;     // 最近一次更新时间
    spb.s_reserved[21] = 0xFFFFFFFF;    // 填充 

    cout << "==========SuperBlock Info==========" << endl;
    cout << std::left << std::setw(30) << "Inode区占用盘块数：" << spb.s_isize << endl;
    cout << std::left << std::setw(30) << "data区占用盘块数：" << spb.s_dsize << endl;
    cout << std::left << std::setw(30) << "盘块总数：" << spb.s_fsize << endl;
    cout << std::left << std::setw(30) << "Inode区起始盘块：" << 3 << endl;
    cout << std::left << std::setw(30) << "数据区起始盘块：" << spb.s_dstart << endl;

    if(write(f_fd, &spb, Constant::BLOCK_SIZE) < Constant::BLOCK_SIZE)
    {
        perror("SuperBlock init error!");
    }

}

/* 
 * 分配初始Inode Bitmap
 * 包含根目录root以及4个子目录
 * 每个目录分别占用1个inode
 * 0号inode空白
 * 共需5+1个inode, 5个数据块存放目录文件
 */

void Format::InitBitmap()
{
    /**
     * 注意为小字序
     */
    char ione[2] = {0x3F, 0x00}; // inodemap预分配6bit
    char done[2] = {0x1F, 0x00}; // datamap预分配5bit
    char zero[4094] = {0};
    
    // lseek(f_fd, FileSystem::BLOCK_SIZE, SEEK_SET);

    // inode bitmap
    if(write(f_fd, &done, sizeof(done)) < sizeof(done))
    {
        perror("Bitmap init 3 error");
    }
    if(write(f_fd, zero, sizeof(zero)) < sizeof(zero))
    {
        perror("Bitmap init 4 error");
    }

    // data bitmap 
    if(write(f_fd, &ione, sizeof(ione)) < sizeof(ione))
    {
        perror("Bitmap init 1 error");
    }
    if(write(f_fd, zero, sizeof(zero)) < sizeof(zero))
    {
        perror("Bitmap init 2 error");
    }

}

void Format::InitDiskInode()
{
    /* 初始化5+1个外存inode */
    DiskInode inode;
    
    /* 初始配置 */
    inode.d_mode = Inode::IRWXU | Inode::IFDIR;
    inode.d_nlink = 0;
    inode.d_uid = 0;
    inode.d_gid = 0;
    inode.d_atime = 0;  
    inode.d_mtime = 0;
    inode.d_size = 0;   // 目录文件的size指目录下所有文件的总大小？


    lseek(f_fd, 3 * Constant::BLOCK_SIZE, SEEK_SET);

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
    // 对0 ~ 4号数据盘块写入目录文件
    // datazone 初始盘块号
    int base_offset = 3 + f_izone_size; // 3 (superblock databitmap inodebitmap) + inode区盘块数量
    DirItem root[6] = {
        {1, "."},
        {1, ".."},
        {2, "bin"},
        {3, "etc"},
        {4, "home"},
        {5, "dev"}
    };

    DirItem sub_dir[2] = {
        {0, "."},   // 当前目录inode默认无效
        {1, ".."}   // 上级目录均为root
    };

    // 写入root目录文件
    lseek(f_fd, base_offset * Constant::BLOCK_SIZE, SEEK_SET);
    if(write(f_fd, &root, sizeof(root)) < sizeof(root))
    {
        perror("root dir block write error");
    }

    for(int i = 2; i <= 5; i++)
    {
        lseek(f_fd, (base_offset + i - 1) * Constant::BLOCK_SIZE, SEEK_SET);
        sub_dir[0].inode_num = i;   // 子目录的当前inode号
        if(write(f_fd, &sub_dir, sizeof(sub_dir)) < sizeof(sub_dir))
        {
            perror("sub_dir block write error");
        }    
    }
}