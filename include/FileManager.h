#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "FileSystem.h"

/* 
 * 文件管理类(FileManager)
 * 封装了文件系统的各种系统调用在核心态下处理过程，
 * 如对文件的Open()、Close()、Read()、Write()等等
 * 封装了对文件系统访问的具体细节。
 */

class DirectoryEntry
{
	/* static members */
public:
	static const int DIRSIZ = 28;	/* 目录项中路径部分的最大字符串长度 */

	/* Functions */
public:
	/* Constructors */
	DirectoryEntry(){};
	/* Destructors */
	~DirectoryEntry(){};

	/* Members */
public:
	int m_ino;		/* 目录项中Inode编号部分 */
	char m_name[DIRSIZ];	/* 目录项中路径名部分 */
};

#endif
