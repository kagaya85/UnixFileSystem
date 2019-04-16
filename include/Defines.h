
#ifndef DEFINES
#define DEFINES

#define Error -1
#define DISK_FILE_NAME "myDisk.img"

/**
 * 目录项结构体 32 Bytes 
 */
struct DirItem 
{
	int inode_num;	// inode号
	char name[28];	// 文件名
};

/**
 * 定义一些常量
 */
class Constant{
public:
	static const int NMOUNT = 5;			/* 系统中用于挂载子文件系统的装配块数量 */
	static const int MIN_DISK_SIZE = 4096;	// 磁盘最少盘块数量 256*4k=1M  16M~128M
	static const int BLOCK_SIZE = 4096;	// 盘块大小 BYTE 4K
	static const int SUPER_BLOCK_SECTOR_NUMBER = 0;	/* 定义SuperBlock位于磁盘上的扇区号 */
	static const int ROOTINO = 0;			/* 文件系统根目录外存Inode编号 */
	static const int INODE_NUMBER_PER_SECTOR = 64;		/* 外存INode对象长度为64字节，每个磁盘块可以存放4096/64 = 64个外存Inode */
	static const int INODE_ZONE_START_SECTOR = 3;		/* 外存Inode区位于磁盘上的起始扇区号 */
};


#endif // !DEFINES
