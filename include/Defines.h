
#ifndef DEFINES
#define DEFINES

#define Error -1
#define DISK_FILE_NAME "myDisk.img"

/**
 * 目录项结构体 32 Bytes 
 */
struct DirItem 
{
	char name[28];	// 文件名
	int inode_num;	// inode号
};

#endif // !DEFINES
