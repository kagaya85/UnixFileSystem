#include "DiskDriver.h"
#include "FileSystem.h"
#include <cstdio>
#include <sys/types.h>    
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>

using namespace std;

DiskDriver::DiskDriver()
{
    /* nothin to do */
}

DiskDriver::~DiskDriver()
{
    close(d_diskfileFd);
}

/**
 * 初始化
 */
int DiskDriver::Initialize()
{
    if(access(DISK_FILE_NAME, F_OK) < 0)
    {
        cerr << "\"" << DISK_FILE_NAME << "\" dose not exit" << endl;
        exit(-1);
    }

    d_diskfileFd = open(DISK_FILE_NAME, O_RDWR);
    if(d_diskfileFd < 0)
    {
        cerr << "Open disk file error" << endl;
        exit(-1);
    }
}

/**
 * 从disk读入到bp所控制的缓存块
 */
int DiskDriver::ReadFromDisk(Buf* bp)
{
    unsigned char* b_addr = bp->b_addr;
    int bln = bp->b_blkno;
    lseek(d_diskfileFd, bln * Constant::BLOCK_SIZE, SEEK_SET);
    read(d_diskfileFd, b_addr, Constant::BLOCK_SIZE);
}

/**
 * 将bp控制的缓存块写入disk
 */
int DiskDriver::WriteToDisk(Buf* bp)
{
    unsigned char* b_addr = bp->b_addr;
    int bln = bp->b_blkno;
    lseek(d_diskfileFd, bln * Constant::BLOCK_SIZE, SEEK_SET);
    write(d_diskfileFd, b_addr, Constant::BLOCK_SIZE);
}