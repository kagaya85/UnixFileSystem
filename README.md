# UnixFileSystem
Using `cpp` to simulate the file system of Unix

## Description

This is my curriculum design of *Operate System*, a part of the source code is from UNIX V6++

* Develop Environment: Ubuntu 18.04.2 LTS on Windows 10 x86_64
* Contribute a simple file system includes FS structure and basic FS operations
* Using a IMG file to imitate the hard driver
* Code for Learning

## Guide

This project mainly contains two executable program:

* `format`

  Please use this to format `myDisk.img` into UNIX V6++ Volume:

  * Super Block
  * Inode Space
  * Data Space

  ![Disk structure]("img/disk structure.jpg")

  After format `myDisk.img`，it will contain a root directory and four sub directory

  * bin
  * etc
  * home
  * dev

  ![directory structure]("img/directory+structure.jpg")

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
