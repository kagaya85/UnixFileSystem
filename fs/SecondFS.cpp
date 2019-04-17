/*
 * File: SecondFS.cpp
 * Project: fs
 * File Created: Monday, 25th March 2019 8:17:08 pm
 * Author: kagaya (kagaya85@outlook.com)
 * -----
 * Last Modified: Monday, 25th March 2019 8:22:12 pm
 * Modified By: kagaya (kagaya85@outlook.com>)
 */

#include "../include/SecondFS.h"
#include "Kernel.h"
#include <iostream>
#include <string>

using namespace std;

extern InodeTable g_InodeTable;

int main()
{
    int inum;   // 指令序号
    SecondFS fs;

    while(true)
    {
        inum = fs.prompt();
        switch (inum)
        {
            case SecondFS::Mkdir:
                fs.mkdir();
                break;
            case SecondFS::Ls:
                fs.ls();
                break;
            default:
                break;
        }
    } 
}

/**
 * 使用kernel初始化文件系统
 */
SecondFS::SecondFS()
{
	Kernel::Instance().Initialize();	
	Kernel::Instance().GetFileSystem().LoadSuperBlock();
	cout << "secondFS Loaded......OK\n" << endl;

	/*  初始化rootDirInode和用户当前工作目录，以便NameI()正常工作 */
	FileManager& fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.rootDirInode = g_InodeTable.IGet(DiskDriver::ROOTDEV, Constant::ROOTINO);
	fileMgr.rootDirInode->i_flag &= (~Inode::ILOCK);

	User& us = Kernel::Instance().GetUser();
	us.u_cdir = g_InodeTable.IGet(DiskDriver::ROOTDEV, Constant::ROOTINO);
	us.u_cdir->i_flag &= (~Inode::ILOCK);
	Utility::StringCopy("/", us.u_curdir);
}

SecondFS::~SecondFS()
{
    /* nothing to do */
}

/*
 * 实现一个命令提示符 
 */
int SecondFS::prompt()
{
    string command;
	User& u = Kernel::Instance().GetUser();
    cout << "[kagaya@localhost]" << u.u_curdir << "# ";
    getline(cin, command);
    
    if(command == "creat")
        return SecondFS::Creat;
    else if(command == "open")
        return SecondFS::Open;
    else if(command == "read")
        return SecondFS::Read;
    else if(command == "write")
        return SecondFS::Write;
    else if(command == "lseek")
        return SecondFS::Lseek;
    else if(command == "close")
        return SecondFS::Close;
    else if(command == "mkdir")
        return SecondFS::Mkdir;
    else if(command == "ls")
        return SecondFS::Ls;
    else
        cout << "Command " << command << " not found" << endl;

    return -1;    
}

void SecondFS::mkdir()
{
    return;
}

void SecondFS::ls()
{
	User& u = Kernel::Instance().GetUser();
	BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
    Inode* pInode;
    Buf* pBuf;
    DirectoryEntry dent;
    int count = 0;

	pInode = u.u_cdir;  // 当前目录Inode
    u.u_IOParam.m_Offset = 0;
		/* 设置为目录项个数 ，含空白的目录项*/
    u.u_IOParam.m_Count = pInode->i_size / (DirectoryEntry::DIRSIZ + 4);
    pBuf = NULL;
    
    while (true)
    {
        /* 对目录项已经遍历完毕 */
        if (0 == u.u_IOParam.m_Count) {
            if (NULL != pBuf) {
                bufMgr.Brelse(pBuf);
            }
            break;
        }

        /* 已读完目录文件的当前盘块，需要读入下一目录项数据盘块 */
        if (0 == u.u_IOParam.m_Offset % Inode::BLOCK_SIZE) {
            if (NULL != pBuf) {
                bufMgr.Brelse(pBuf);
            }
            /* 计算要读的物理盘块号 */
            int phyBlkno = pInode->Bmap(u.u_IOParam.m_Offset / Inode::BLOCK_SIZE);
            pBuf = bufMgr.Bread(pInode->i_dev, phyBlkno);
        }

        /* 读取下一目录项至dent */
        int* src = (int*)(pBuf->b_addr + (u.u_IOParam.m_Offset % Inode::BLOCK_SIZE));
        Utility::DWordCopy(src, (int*)&dent, sizeof(DirectoryEntry) / sizeof(int));
        u.u_IOParam.m_Offset += (DirectoryEntry::DIRSIZ + 4);
        u.u_IOParam.m_Count--;  // ???????????

        /* 如果是空闲目录项，记录该项位于目录文件中偏移量 */
        if ( 0 == dent.m_ino )
        {
            /* 跳过空闲目录项 */
            continue;
        }
        else
        {
            cout << dent.m_name;
            count++;
            if(count % 5 == 0)
                cout << endl;
        }
    }

	// g_InodeTable.IPut(pInode);

    return;
}

