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
  * Super Block: 128 Bytes
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

    定义的全局变量

    ```c++
    /* 系统全局超级块SuperBlock对象 */
    SuperBlock g_spb;
    /*  定义内存Inode表的实例 */
    InodeTable g_InodeTable;
    ```

  * `Inode` `Inode`相关

  * `SecondFS` 实现shell窗口，调用Kernel提供的初始化函数以及`FileManager`的文件操作接口

  * `SuperBlock` 超级块相关

* Kernel

  * Kernel.cpp定义的全局变量

    ```c++
    Kernel Kernel::instance;
    DiskDriver g_DiskDriver;
    BufferManager g_BufferManager;
    FileSystem g_FileSystem;
    FileManager g_FileManager;
    User g_User;
    ```

  * Kernel 通过kernel类中的静态变量来对一些全局对象、变量进行管理



> 2019年4月3日
>
> fs暂时的实现想法：
>
> 在kernel中实例化各个类
>
> 有关磁盘读写的操作（文件读写）一律在DiskDriver中实现
>
> 先不通过缓存读写，先实现直接读写文件
>
> 看来还是离不开user结构和（目录指针）utility结构（时间）
>
> 2019年4月15日
>
> 这一段时间内基本重写了各个类中的关键函数
>
> 修改主要结构体，重构了文件系统部分使其能够更好的独立运作
>
> 同时还是实现了缓存功能，虽然是模拟，但感觉文件系统还是不能离开缓存系统的支持
>
> 完成了format程序的基本功能，感觉接下来才是苦战啊
>
> 2019年4月16日
>
> 任务：
>
> 重写四个大类的初始化函数匹配磁盘文件
>
> `makefile`
>
> 编译纠错