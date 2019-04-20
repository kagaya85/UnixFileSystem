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
        Exit
    };

public:
    SecondFS();
    ~SecondFS();
    
    int prompt();
    std::vector<std::string> split(const std::string& s, const std::string& c);

    /* command */
    void creat(std::string filename);
    void open(std::string filename, int mode);
    void read();
    void write();
    void lseek();
    void close();
    void mkdir(std::string dir);
    void ls();
    void cd(std::string dir);
};


#endif // !SECOND
