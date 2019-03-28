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

using namespace std;

int main()
{
    /* 盘块数量限制为4096~134217728，对于磁盘大小为16M~128M */
    int diskSize = FileSystem::MIN_DISK_SIZE;   
    while (true)
    {
        cout << "Please Input Disk Block Number (default "<< FileSystem::MIN_DISK_SIZE <<" x 512 Bytes):";
        if(cin.peek() != '\n')
            cin >> diskSize;
        if(diskSize < FileSystem::MIN_DISK_SIZE)
            cout << "Disk size at least 4096 x 512 Bytes!!!" << endl;
        else
            break;        
    }
    

    Format Disk(diskSize);

    Disk.InitSuperBolck();
    Disk.InitBitmap();
    Disk.InitDiskInode();
    Disk.InitData();

    cout << "Diskfile (" << DISK_FILE_NAME << ") foramt success" << endl;
    return 0;
}