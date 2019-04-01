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
    while(true)
    {
        inum = prompt();
        switch (inum)
        {
            case Mkdir:
                mkdir();
                break;
        
            default:
                break;
        }
    } 
}

/*
 * 实现一个命令提示符 
 */
int prompt()
{
    string command;
    
    cout << "root@localhost# ";
    getline(cin, command);
    
    if(command == "creat")
        return Creat;
    else if(command == "open")
        return Open;
    else if(command == "read")
        return Read;
    else if(command == "write")
        return Write;
    else if(command == "lseek")
        return Lseek;
    else if(command == "close")
        return Close;
    else if(command == "mkdir")
        return Mkdir;
    else
        cout << "Command " << command << " not found" << endl;

    return -1;    
}

void mkdir()
{

}