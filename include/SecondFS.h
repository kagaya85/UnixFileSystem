/*
 * File: SecondFS.h
 * Project: include
 * File Created: Monday, 25th March 2019 8:17:36 pm
 * Author: kagaya (kagaya85@outlook.com)
 * -----
 * Last Modified: Monday, 25th March 2019 8:21:58 pm
 * Modified By: kagaya (kagaya85@outlook.com>)
 */


#ifndef SECOND
#define SECOND
#include "FileSystem.h"
#include "Defines.h"
#include <cstring>
#include <string>
#include <vector>
/**
 * Shell的实现
 */
class SecondFS
{
public:
    enum Command
    {
        Creat,
        Open,
        Read,
        Write,
        Lseek,
        Close,
        Mkdir,
        Ls,
        Cd,
        Help,
        Load,
        Exit
    };

public:
    SecondFS();
    ~SecondFS();
    
    int prompt();
    std::vector<std::string> split(const std::string& s, const std::string& c);

    /* command */
    int MyCreat(std::string filename);
    void MyOpen(std::string filename, int mode);
    void MyRead(int fd, int count);
    void MyWrite(int fd, int count, std::string text);
    void MyLseek(int fd, int offset, int mode);
    void MyClose(int fd);
    void MyMkdir(std::string dir);
    void MyLs();
    void MyCd(std::string dir);
    void MyLoad(std::string filename);
    void MyHelp();
};


#endif // !SECOND
