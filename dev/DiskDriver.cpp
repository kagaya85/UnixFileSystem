#include "DiskDriver.h"
#include "FileSystem.h"
#include <cstdio>
#include <sys/types.h>    
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>

using namespace std;
/* 定义块设备表devtab的实例。为系统中ATA硬盘设置一个块设备表。*/
Devtab g_Atab;

Devtab::Devtab()
{
	this->d_active = 0;
	this->d_errcnt = 0;
	this->b_forw = NULL;
	this->b_back = NULL;
	this->d_actf = NULL;
	this->d_actl = NULL;
}

Devtab::~Devtab()
{
	//nothing to do here
}


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
    else
    {
        cout << "Disk File Loaded." << endl;
    }
    
    this->d_tab = &g_Atab;
	if(this->d_tab != NULL)
	{
		this->d_tab->b_forw = (Buf *)this->d_tab;
		this->d_tab->b_back = (Buf *)this->d_tab;
	}
}

/**
 * 从disk读入到bp所控制的缓存块
 */
int DiskDriver::ReadFromDisk(Buf* bp)
{
    unsigned char* b_addr = bp->b_addr;
    int bln = bp->b_blkno;
    int ret;
    lseek(d_diskfileFd, bln * Constant::BLOCK_SIZE, SEEK_SET);
    if((ret = read(d_diskfileFd, b_addr, Constant::BLOCK_SIZE)) < Constant::BLOCK_SIZE)
    {
        cerr << "Read Block " << bln << " Error." << endl;
        perror("Error");
        exit(-1);
    }
    else
    {
#ifdef DEBUG
		std::cout << "Block " << bp->b_dev << ':' << bln << " load " << ret << " Bytes." << std::endl;
#endif
    }
    
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