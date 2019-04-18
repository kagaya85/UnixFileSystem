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
        Cd
    };

public:
    SecondFS();
    ~SecondFS();
    
    int prompt();
    vector<string> split(const string& s, const string& c);

    /* command */
    void creat();
    void open();
    void read();
    void write();
    void lseek();
    void close();
    void mkdir();
    void ls();
    void cd();
};


#endif // !SECOND
