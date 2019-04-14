/*
 * File: SecondFS.cpp
 * Project: fs
 * File Created: Monday, 25th March 2019 8:17:08 pm
 * Author: kagaya (kagaya85@outlook.com)
 * -----
 * Last Modified: Monday, 25th March 2019 8:22:12 pm
 * Modified By: kagaya (kagaya85@outlook.com>)
 */

#include "SecondFS.h"
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
        
            default:
                break;
        }
    } 
}

/*
 * 实现一个命令提示符 
 */
int SecondFS::prompt()
{
    string command;
    
    cout << "root@localhost# ";
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
    else
        cout << "Command " << command << " not found" << endl;

    return -1;    
}

void SecondFS::mkdir()
{

}

