#ifndef KERNEL_H
#define KERNEL_H

#include "BufferManager.h"
#include "FileManager.h"
#include "FileSystem.h"
#include "DiskDriver.h"
#include "User.h"
#include "Utility.h"
/*
 * Kernel类用于封装所有内核相关的全局类实例对象，
 * 例如PageManager, ProcessManager等。
 * 
 * Kernel类在内存中为单体模式，保证内核中封装各内核
 * 模块的对象都只有一个副本。
 */
class Kernel
{
public:
	// static const unsigned long USER_ADDRESS = 0x400000 - 0x1000 + 0xc0000000;	/* 0xC03FF000 */
	// static const unsigned long USER_PAGE_INDEX = 1023;		/* USER_ADDRESS对应页表项在PageTable中的索引 */

public:
	Kernel();
	~Kernel();
	static Kernel& Instance();
	void Initialize();		/* 该函数完成初始化内核大部分数据结构的初始化 */

	BufferManager& GetBufferManager();
	FileSystem& GetFileSystem();
	FileManager& GetFileManager();
	DiskDriver& GetDiskDriver();
	User& GetUser();

private:
	void InitMemory();
	void InitBuffer();
	void InitFileSystem();

private:
	static Kernel instance;		/* Kernel单体类实例 */
	static int DiskfileFd;	// 磁盘文件号
	BufferManager* m_BufferManager;
	FileSystem* m_FileSystem;
	FileManager* m_FileManager;
	DiskDriver* m_DiskDriver;
	User* m_User;
};

Kernel Kernel::instance;
DiskDriver g_DiskDriver;
BufferManager g_BufferManager;
FileSystem g_FileSystem;
FileManager g_FileManager;
User g_User;
/* 系统全局超级块SuperBlock对象 */
SuperBlock g_spb;
/*  定义内存Inode表的实例 */
InodeTable g_InodeTable;

#endif
