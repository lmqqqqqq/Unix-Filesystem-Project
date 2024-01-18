---
typora-root-url: ./img
---

# Unix-Filesystem-Project

## Introduction

This is a Unix-like file system, along with a corresponding command-line interface.

The project entails the specific implementation of the following components:

1. Unix V6++ Disk
   - Utilize a standard large file (such as `C:\myDisk.img`) to emulate the disk of Unix V6++
2. Disk File Structure
   - Structure and size of the SuperBlock
   - Structure of the Disk Inode, including the index structure
   - Design and implementation of the allocation algorithm for Disk Inode
   - Design and implementation of the allocation algorithm for the file data
3. File Directory Structure
   - Structure of the directory file
   - Design of directory retrieval algorithm
   - Design for adding, deleting, and modifying the directory structure
4.  File Open Structure
   - File open structure
   - Structure of the in-memory Inode
   - Design and implementation of the allocation algorithm for in-memory Inode
   - File opening process
5. File System
   - Implementation of file read and write operations
   - Implementation of other file operations
6. Disk Cache
   - Structure of cache control blocks
   - Structure of cache queues
   - Design and implementation of the allocation algorithm for cache queues
   - Implementation of read and write operations on the file using the cache

## Functionality 

|                 Command                 |                         Description                          |
| :-------------------------------------: | :----------------------------------------------------------: |
|                  help                   |            Display an overview of functionalities            |
|                 fformat                 |                    Format the file volume                    |
|                   ls                    |          List the contents of the current directory          |
|           mkdir &lt;dirname>            |                     Make a new directory                     |
|             cd &lt;dirname>             |                 Change the present directory                 |
|          fcreate &lt;filename>          |                        Create a file                         |
|          fdelete &lt;filename>          |                        Delete a file                         |
|           fopen &lt;filename>           |                        Open the file                         |
|             fclose &lt;fd>              |                        Close the file                        |
|        fread &lt;fd> &lt;nbytes>        | Read nbytes from the file, starting from the current file pointer |
|  fwrite &lt;fd> &lt;nbytes>&lt;string>  | Write nbytes of the string to the file at the current file pointer position |
| flseek &lt;fd> &lt;offset> &lt;ptrname> | Move the file pointer of the file by offset bytes in the direction specified by ptrname |
|        cp &lt;file1> &lt;file2>         |             Copy the contents of file1 to file2              |
|           ftree &lt;dirname>            |                  Display the directory tree                  |
|                   pwd                   |                   Display the current path                   |
|      Frename &lt;file1> &lt;file2>      |                    Rename file1 to file2                     |
|                  Exit                   |                 Exit the current file system                 |

## Data Struture Defination

1. Disk file structure 

   The disk file structure defined in this project is consistent with the disk file structure of Unix V6++. The structure, as shown in the diagram below, includes the SuperBlock, Disk Inode blocks, and file data blocks.

   ![](/Disk file structure.png)

   - SuperBlock

     Unix V6++ employs different management algorithms for various regions of the entire disk storage space, and all relevant information is stored in the SuperBlock. 

     ```cpp
     class SuperBlock
     {
     public:
     	SuperBlock() {};
     	~SuperBlock() {};
     public:
     	const static int MAX_NUMBER_FREE = 100;
     	const static int MAX_NUMBER_INODE = 100;
     public:
     	int		s_isize;					/* 外存Inode区占用的盘块数 */
     	int		s_fsize;					/* 盘块总数 */
     
     	int		s_nfree;					/* 直接管理的空闲盘块数量 */
     	int		s_free[MAX_NUMBER_FREE];	/* 直接管理的空闲盘块索引表 */
     
     	int		s_ninode;					/* 直接管理的空闲外存Inode数量 */
     	int		s_inode[MAX_NUMBER_INODE];	/* 直接管理的空闲外存Inode索引表 */
     
     	int		s_fmod;						/* 内存中super block副本被修改标志，意味着需要更新外存对应的Super Block */
     	int		padding[51];				/* 填充使SuperBlock块大小等于1024字节，占据2个扇区 */
     };
     
     ```

   - DiskInode

     DiskInode, serving as the external storage inode, is the region where file index information is stored. It is stored on the disk in a static form.

     ```cpp
     class DiskInode
     {
     public:
     	DiskInode() {};
     	~DiskInode() {};
     public:
     	unsigned int d_mode;	/* 状态的标志位，定义见enum INodeFlag */
     	int		d_nlink;		/* 文件联结计数，即该文件在目录树中不同路径名的数量 */
     	int		d_size;			/* 文件大小，字节为单位 */
     	int		d_addr[10];		/* 用于文件逻辑块号和物理块号转换的基本索引表 */
     	int		padding[3];		/* 填充使得DiskInode类占64个字节 */   
     };
     
     ```

   - File index structure

     Unix V6++ adopts a mixed index structure, classifying files into small, large, and huge based on size. Small files use a direct indexing method, occupying up to 6 data blocks; large files use a single indirect indexing method, occupying up to 6 + 2\*128 data blocks; huge files use a double indirect indexing method, occupying up to 6 + 2*128 + 2\*128\*128 data blocks.

     ```cpp
     static const int BLOCK_SIZE = 512;		/* 文件逻辑块大小: 512字节 */
     static const int ADDRESS_PER_INDEX_BLOCK = BLOCK_SIZE / sizeof(int);	/* 每个间接索引表(或索引块)包含的物理盘块号 */
     
     static const int SMALL_FILE_BLOCK = 6;	/* 小型文件：直接索引表最多可寻址的逻辑块号 */
     static const int LARGE_FILE_BLOCK = 128 * 2 + 6;	/* 大型文件：经一次间接索引表最多可寻址的逻辑块号 */
     static const int HUGE_FILE_BLOCK = 128 * 128 * 2 + 128 * 2 + 6;	/* 巨型文件：经二次间接索引最大可寻址文件逻辑块号 */
     
     static const int PIPSIZ = SMALL_FILE_BLOCK * BLOCK_SIZE;
     
     ```

2. Directory structure 

3. File open structure 

4. Cache structure
