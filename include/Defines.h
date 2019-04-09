/* 
 * 目录项结构体 32 Bytes 
 */
struct DirItem 
{
	char name[28];	// 文件名
	int inode_num;	// inode号
};
