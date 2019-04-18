/*
 * File: SecondFS.cpp
 * Project: fs
 * File Created: Monday, 25th March 2019 8:17:08 pm
 * Author: kagaya (kagaya85@outlook.com)
 * -----
 * Last Modified: Thursday, 18th April 2019 2:10:17 pm
 * Modified By: kagaya (kagaya85@outlook.com>)
 */

#include "SecondFS.h"
#include "Kernel.h"
#include <iostream>


using namespace std;

extern InodeTable g_InodeTable;

int main()
{
    int inum;   // 指令序号
    SecondFS fs;

    while(true)
    {
        inum = fs.prompt();
    } 
}

/**
 * 使用kernel初始化文件系统
 */
SecondFS::SecondFS()
{
	cout << "SecondFS Loading..." << endl;
	Kernel::Instance().Initialize();	
	Kernel::Instance().GetFileSystem().LoadSuperBlock();
    cout << "SecondFS Loaded......OK." << endl;
	/*  初始化rootDirInode和用户当前工作目录，以便NameI()正常工作 */
	FileManager& fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.rootDirInode = g_InodeTable.IGet(DiskDriver::ROOTDEV, Constant::ROOTINO);
    fileMgr.rootDirInode->i_flag &= (~Inode::ILOCK);
    cout << "rootDirInode Loaded." << endl;
#ifdef DEBUG
    fileMgr.rootDirInode->IInfo();
#endif
	User& us = Kernel::Instance().GetUser();
	us.u_cdir = g_InodeTable.IGet(DiskDriver::ROOTDEV, Constant::ROOTINO);
	us.u_cdir->i_flag &= (~Inode::ILOCK);
    cout << "u_cdir inode Loaded." << endl;
#ifdef DEBUG
    us.u_cdir->IInfo();
#endif
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
    string command, line, ch;
	vector<string> argv;
    User& u = Kernel::Instance().GetUser();
    ch = " ";

    while(true)
    {
        cout << "[kagaya@localhost](" << u.u_curdir << ")# ";
        getline(cin, line);
        if(line.length() != 0)
            break;
    }
        
    argv = split(line, ch);
    command = argv[0];
    
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
    else if (command == "ls") {
        this->ls();
        return SecondFS::Ls;
    } else if (command == "cd") {
        this->cd(argv[1]);
        return SecondFS::Cd;
    } else
        cout << "Command " << command << " not found" << endl;

    return -1;    
}

vector<string> SecondFS::split(const string& s, const string& c)
{ 
    vector<string> v;
    std::string::size_type pos1, pos2;
    size_t len = s.length();
    pos2 = s.find(c);
    pos1 = 0;
    while(std::string::npos != pos2)
    {
        v.emplace_back(s.substr(pos1, pos2-pos1));
 
        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != len)
        v.emplace_back(s.substr(pos1));
    
    return v;
}


/* command */
void SecondFS::mkdir()
{
    return;
}

void SecondFS::ls()
{
	User& u = Kernel::Instance().GetUser();
	BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
    Inode* pInode;
    Buf* pBuf = NULL;
    DirectoryEntry dent;
    int count = 0;

	pInode = u.u_cdir;  // 当前目录Inode

    u.u_IOParam.m_Offset = 0;   // 设置为已读取的字节数
		/* 设置为目录项个数 ，含空白的目录项*/
    u.u_IOParam.m_Count = pInode->i_size / (DirectoryEntry::DIRSIZ + 4);

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
        u.u_IOParam.m_Count--; 

        /* 如果是空闲目录项，记录该项位于目录文件中偏移量 */
        if ( 0 == dent.m_ino )
        {
            /* 跳过空闲目录项 */
            // cout << 1 << endl;
            continue;
        }

        if (dent.m_name[0] == '.')
        {
            // 默认跳过隐藏文件 . ..
            // cout << 2 << endl;
            continue;
        }
        else
        {
            count++;
            if(count % 6 == 0)
                cout << endl;
            cout << dent.m_name << ' ';
        }
    }
    cout << endl;
    return;
}

void SecondFS::cd(string dir)
{
    User& u = Kernel::Instance().GetUser();
    FileManager& fm = Kernel::Instance().GetFileManager();

    char* pstr= new(nothrow) char[dir.length() + 1];
    strcpy(pstr, dir.c_str());
    u.u_dirp = pstr;    
    u.u_arg[0] = (long long)pstr;
    u.u_error = User::MYNOERROR;
    fm.ChDir();
    if(u.u_error)
    {
        if(u.u_error == User::MYENOTDIR)
            cerr << "cd: not a directory:" << dir << endl;
        else if(u.u_error == User::MYENOENT)
            cerr << "cd: no such file or directory:" << dir << endl;
    }

    delete pstr;
    return;
}
