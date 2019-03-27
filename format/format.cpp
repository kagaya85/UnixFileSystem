/*
 * File: format.cpp
 * Project: format
 * File Created: Monday, 25th March 2019 8:14:32 pm
 * Author: kagaya (kagaya85@outlook.com)
 * -----
 * Last Modified: Monday, 25th March 2019 8:22:16 pm
 * Modified By: kagaya (kagaya85@outlook.com>)
 */

#include "format.h"
#include <iostream>

using namespace std;

int main()
{
    int diskSize = MIN_DISK_SIZE;
    while (true)
    {
        cout << "Please Input Disk Size (BYTE default "<< MIN_DISK_SIZE <<" Bytes):";
        if(cin.peek() != '\n')
            cin >> diskSize;
        if(diskSize < MIN_DISK_SIZE)
            cout << "Disk size at least 4096 Bytes!!!" << endl;
        else
            break;        
    }
    

    Format Disk(diskSize);

    Disk.InitSuperBolck();
    Disk.InitDiskInode();
    Disk.InitData();

    cout << "Diskfile (" << DISK_FILE_NAME << ") foramt success" << endl;
    return 0;
}