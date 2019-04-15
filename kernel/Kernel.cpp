#include "Kernel.h"
#include <iostream>

using namespace std;

Kernel Kernel::instance;
DiskDriver g_DiskDriver;
BufferManager g_BufferManager;
FileSystem g_FileSystem;
FileManager g_FileManager;
User g_User;

Kernel::Kernel()
{
}

Kernel::~Kernel()
{
}

Kernel& Kernel::Instance()
{
	return Kernel::instance;
}

void Kernel::InitBuffer()
{
	this->m_BufferManager = &g_BufferManager;
	this->m_DiskDriver = &g_DiskDriver;

	cout << "Initialize Buffer..." << endl;
	this->GetBufferManager().Initialize();
	cout << "OK." << endl;

	cout << "Initialize Device Driver..." << endl;
	this->GetDiskDriver().Initialize();
	cout << "OK." << endl;
}

void Kernel::InitFileSystem()
{
	this->m_FileSystem = &g_FileSystem;
	this->m_FileManager = &g_FileManager;

	cout << "Initialize File System..." << endl;
	this->GetFileSystem().Initialize();
	cout << "OK." << endl;

	cout << "Initialize File Manager..." << endl;
	this->GetFileManager().Initialize();
	cout << "OK." << endl;
}

void Kernel::Initialize()
{
	this->m_User = &g_User;
	InitBuffer();
	InitFileSystem();
}

BufferManager& Kernel::GetBufferManager()
{
	return *(this->m_BufferManager);
}

FileSystem& Kernel::GetFileSystem()
{
	return *(this->m_FileSystem);
}

FileManager& Kernel::GetFileManager()
{
	return *(this->m_FileManager);
}

DiskDriver& Kernel::GetDiskDriver()
{
	return *(this->m_DiskDriver);
}

User& Kernel::GetUser()
{
	return *(this->m_User);
}
