/*
 * File: SecondFS.h
 * Project: include
 * File Created: Monday, 25th March 2019 8:17:36 pm
 * Author: kagaya (kagaya85@outlook.com)
 * -----
 * Last Modified: Monday, 25th March 2019 8:21:58 pm
 * Modified By: kagaya (kagaya85@outlook.com>)
 */

/*
 * 实现命令行窗口对myDisk.img文件进行基本文件操作
 * 实现基本的系统调用
 */
#include "FileSystem.h"

enum Command
{
    Creat,
    Open,
    Read,
    Write,
    Lseek,
    Close,
    Mkdir
};

void mkdir();