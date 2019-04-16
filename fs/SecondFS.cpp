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
	Inode* pInode;

	pInode = u.u_cdir;  // 当前目录Inode

    return;
}

