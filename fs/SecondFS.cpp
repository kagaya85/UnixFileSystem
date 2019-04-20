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
    int ret;   // 指令序号
    SecondFS fs;

    while(true)
    {
        ret = fs.prompt();
        if (ret == SecondFS::Exit)
            break;
    } 

    return 0;
}

/**
 * 使用kernel初始化文件系统
 */
SecondFS::SecondFS()
{
	cout << "SecondFS Loading..." << endl;
	Kernel::Instance().Initialize();	
	Kernel::Instance().GetFileSystem().LoadSuperBlock();
	Kernel::Instance().GetFileSystem().LoadBitmap(DiskDriver::DATA_BITMAP_BLOCK);
	Kernel::Instance().GetFileSystem().LoadBitmap(DiskDriver::INODE_BITMAP_BLOCK);
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
    BufferManager& bfm = Kernel::Instance().GetBufferManager();
    User& u = Kernel::Instance().GetUser();
    FileManager& fm = Kernel::Instance().GetFileManager();

    bfm.Bflush(DiskDriver::ROOTDEV);    // 将缓存刷如磁盘
    g_InodeTable.UpdateInodeTable();        // 同步inode
    // 关闭所有文件
    for(int i = 0; i < OpenFiles::NOFILES; i++)
    {
        u.u_arg[0] = i;
        u.u_error = User::MYNOERROR;

        fm.Close();
    }
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

    if (command == "creat") {
        this->creat(argv[1]);
        return SecondFS::Creat;
    } else if (command == "open") {
        int mode = 0;
        mode = File::FREAD | File::FWRITE;  // 读写打开
        
        this->open(argv[1], mode);
        return SecondFS::Open;
    } else if (command == "read") {
        int fd = atoi(argv[1].c_str());
        int count = atoi(argv[2].c_str());
        
        if (fd <= 0 && argv[1] != "0") {
            cerr << "fd error." << endl;
        }

        this->read(fd, count);
        return SecondFS::Read;
    } else if (command == "write") {
        int fd = atoi(argv[1].c_str());
        int count = argv[2].length();   // 不写入 \0
        
        if (fd <= 0 && argv[1] != "0") {
            cerr << "fd error." << endl;
        }

        this->write(fd, count, argv[2]);
        return SecondFS::Write;
    } else if (command == "lseek") {
        int fd = atoi(argv[1].c_str());
        int offset = atoi(argv[2].c_str());
        int mode = atoi(argv[3].c_str());
        
        if (fd <= 0 && argv[1] != "0") {
            cerr << "fd error." << endl;
        }
        
        this->lseek(fd, offset, mode);
        return SecondFS::Lseek;
    } else if (command == "close") {
        int fd = atoi(argv[1].c_str());

        this->close(fd);
        return SecondFS::Close;
    } else if (command == "mkdir") {
        this->mkdir(argv[1]);
        return SecondFS::Mkdir;
    } else if (command == "ls") {
        this->ls();
        return SecondFS::Ls;
    } else if (command == "cd") {
        this->cd(argv[1]);
        return SecondFS::Cd;
    } else if (command == "exit"){
        return SecondFS::Exit;
    } else if (command == "help"){
        this->help();
        return SecondFS::Help;
    } else {
        cout << "Command " << command << " not found" << endl;
        this->help();
    }

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

void SecondFS::creat(string filename)
{
    User& u = Kernel::Instance().GetUser();
    FileManager& fm = Kernel::Instance().GetFileManager();

    char* pstr= new(nothrow) char[filename.length() + 1];
    strcpy(pstr, filename.c_str());
    u.u_dirp = pstr;    
    u.u_arg[0] = (long long)pstr;   // 文件名字符串读写指针
    u.u_arg[1] = Inode::IRWXU | Inode::IRWXG;   // 770
    u.u_error = User::MYNOERROR;

    fm.Creat();

    if (u.u_error)
    {
        cerr << "Creat file failed." << endl;
    }
    else
    {
        cout << "Creat file success: fd = " << u.u_ar0[User::EAX] << endl;
    }
    return;
}

void SecondFS::open(string filename, int mode)
{
    User& u = Kernel::Instance().GetUser();
    FileManager& fm = Kernel::Instance().GetFileManager();
    Inode* pInode = NULL;
    
    char* pstr= new(nothrow) char[filename.length() + 1];
    strcpy(pstr, filename.c_str());
    u.u_dirp = pstr;    
    u.u_arg[0] = (long long)pstr;   // 文件名字符串读写指针
    u.u_arg[1] = mode;  // 打开mode
    u.u_error = User::MYNOERROR;

    fm.Open();

    if (u.u_error)
    {
        cerr << "Open file failed." << endl;
    }
    else
    {
        cout << "Open file success: fd = " << u.u_ar0[User::EAX] << endl;
    }
    return;
}

void SecondFS::read(int fd, int count)
{
    User& u = Kernel::Instance().GetUser();
    FileManager& fm = Kernel::Instance().GetFileManager();
    unsigned char buf[1024] = {0};

    u.u_arg[0] = fd;
    u.u_arg[1] = (long long)buf;    // 目标缓冲区
    u.u_arg[2] = count; // 读写字节数
    u.u_error = User::MYNOERROR;

    fm.Read();
    if (u.u_error)
    {
        cerr << "Read file error." << endl;
    }
    else
    {
        cout << "Read file success: " << endl << buf << endl;
    }
    return;
}

void SecondFS::write(int fd, int count, string text)
{
    User& u = Kernel::Instance().GetUser();
    FileManager& fm = Kernel::Instance().GetFileManager();
    unsigned char buf[1024] = {0};

    memcpy(buf, text.c_str(), text.length());

    u.u_arg[0] = fd;
    u.u_arg[1] = (long long)buf;    // 目标缓冲区
    u.u_arg[2] = count; // 读写字节数
    u.u_error = User::MYNOERROR;

    fm.Write();
    if (u.u_error)
    {
        cerr << "Write file error." << endl;
    }
    else
    {
        cout << "Write file success." << endl;
    }
    return;
}   

void SecondFS::lseek(int fd, int offset, int mode)
{
    User& u = Kernel::Instance().GetUser();
    FileManager& fm = Kernel::Instance().GetFileManager();

    u.u_arg[0] = fd;
    u.u_arg[1] = offset;
    u.u_arg[2] = mode; // 0-从文件开始 1-当前位置 2-文件末尾位置 
    u.u_error = User::MYNOERROR;

    fm.Seek();
    if (u.u_error)
    {
        cerr << "Seek file error." << endl;
    }
    else
    {
        cout << "Seek file success." << endl;
    }
    return;
}

void SecondFS::close(int fd)
{
    User& u = Kernel::Instance().GetUser();
    FileManager& fm = Kernel::Instance().GetFileManager();

    u.u_arg[0] = fd;
    u.u_error = User::MYNOERROR;

    fm.Close();
    if (u.u_error)
    {
        cerr << "Close file error." << endl;
    }
    else
    {
        cout << "Close file success." << endl;
    }
    return;
}   

void SecondFS::mkdir(std::string dir)
{
    User& u = Kernel::Instance().GetUser();
    FileManager& fm = Kernel::Instance().GetFileManager();
    Inode* pInode = NULL;
	unsigned int newACCMode = Inode::IFDIR | (Inode::IRWXU|Inode::IRWXG); // 770
    
    char* pstr= new(nothrow) char[dir.length() + 1];
    strcpy(pstr, dir.c_str());
    u.u_dirp = pstr;    
    u.u_arg[0] = (long long)pstr;
    u.u_error = User::MYNOERROR;
    
    pInode = fm.NameI(fm.NextChar, FileManager::CREATE);
    if ( NULL == pInode )
    {
        if(u.u_error)
		{
            cerr << "Error code: " << hex << u.u_error << endl;
	        cout.unsetf(ios::hex);
            return;
        }
		/* 创建Inode */
		pInode = fm.MakNode( newACCMode & (~Inode::ISVTX) );

	    // 解锁inode
        pInode->Prele();
        /* 创建失败 */
        if ( NULL == pInode )
		{
			std::cerr << "Creat Inode Error" << std::endl;
			return;
		}
        else
        {
            // 写目录文件 初始的两个目录项
            DirectoryEntry dent[2];
            dent[0].m_ino = pInode->i_number;
            strcpy(dent[0].m_name, ".");
            dent[1].m_ino = u.u_pdir->i_number;
            strcpy(dent[1].m_name, "..");
            u.u_IOParam.m_Base = (unsigned char*)dent; // 写入内容地址
            u.u_IOParam.m_Count = sizeof(dent);    // 写入字节数
            u.u_IOParam.m_Offset = 0;   // 写入偏移位置
            pInode->WriteI();
        }
    }
    else
    {
        cout << "mkdir: cannot create directory \'" << dir << "\': File exists";
    }

    delete pstr;
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
            cerr << "cd: not a directory: " << dir << endl;
        else if(u.u_error == User::MYENOENT)
            cerr << "cd: no such file or directory: " << dir << endl;
    }

    delete pstr;
    return;
}

void SecondFS::help()
{
    cout << "=============SecondFileSystem================" << endl;
    cout << "| creat filename                            |" << endl;
    cout << "| open filename                             |" << endl;
    cout << "| read fd count(bytes num)                  |" << endl;
    cout << "| write fd count(bytes num) text            |" << endl;
    cout << "| lseek fd offset mode(0-begin 1-cur 2-end) |" << endl;
    cout << "| close fd                                  |" << endl;
    cout << "| mkdir dirname                             |" << endl;
    cout << "| ls                                        |" << endl;
    cout << "| cd path                                   |" << endl;
    cout << "| help                                      |" << endl;
    cout << "=============================================" << endl;
}
