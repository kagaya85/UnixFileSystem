# UnixFileSystem
Using `cpp` to simulate the file system of Unix

## Overview

This is my curriculum design of *Operate System*, a part of the source code is from UNIX V6++

* Develop Environment: WSL Ubuntu 18.04.2 LTS on Windows 10 x86_64
* Contribute a simple file system includes FS structure and basical FS operations
* Using a IMG file to imitate the hard driver
* Code for Learning

## Guide

This project mainly has two executable program:

* `format`

  Please use this to format `myDisk.img` into UNIX V6++ Volume:

  * Super Block

  * Data Bitmap

  * Inode Bitmap

  * Inode Space

  * Data Space

    | Super Block | Data Bitmap | Inode Bitmap | Inode Space                    | Data Space    |
    | ----------- | ----------- | ------------ | ------------------------------ | ------------- |
    | Block 0     | Block 1     | Block 2      | Block 3 ~ (Dynamic allocation) | < (4M ~ 128M) |

  **Some details**

  * Block Size: 4096 Bytes
  * Super Block: 1024 Bytes
  * Data Bitmap: 1 Block → 4 * 1024 * 8 Blocks → 128 MB，so this FS contains 128MB space at **Most**
  * Inode Bitmap: 1 Block → 4 * 1024 * 8 Inodes
  * Inode Space: 1 Block contains 64 Inodes and every 4 Blocks will allocate a Inode, so every megabyte of data will allocate a block of Inodes
  * Data Space: Rest of the Space 
  * Disk Size: 4096 Blocks (16M) at **Least** and 134217728 Blocks (128M) at **Most**

  

  **Then**

  After format `myDisk.img`，it will contain a root directory and four sub directory

  * bin
  * etc
  * home
  * dev

  ![directory structure](img/directory.jpg)

* `secondFS`

  This is basic command prompt window

  Basic system instructions:

  * creat
  * open
  * read
  * write
  * lseek(seek)
  * close
  * mkdir
  * others...

# Structure

* dev 磁盘读写相关
  * `BufferManager` buffer管理
  * `DiskDriver` 最基本的磁盘读写操作
* fs 文件系统
  * `FileManager` 文件系统一些指令的调用实现
  * `FileSystem` 文件系统的初始化，管理文件存储设备中的各类存储资源，磁盘块、外存`INode`的分配、释放。
  * `Inode` `Inode`相关
  * `SecondFS` 实现shell窗口
  * `SuperBlock` 超级块相关

* Kernel
  * Kernel 通过kernel类中的静态变量来对一些全局对象、变量进行管理

> fs暂时的实现想法：
>
> 在kernel中实例化各个类
>
> 有关磁盘读写的操作（文件读写）一律在DiskDriver中实现
>
> 先不通过缓存读写，先实现直接读写文件