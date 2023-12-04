# Unix-Filesystem-Project

## Introduction

This is a Unix-like file system, along with a corresponding command-line interface.

The project entails the specific implementation of the following components:

1. Unix V6++ Disk
   - Utilize a standard large file (such as `C:\myDisk.img`) to emulate the disk of UNIX V6++
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
|          fcreate &lt;filename>          |                Create a file named "filename"                |
|          fdelete &lt;filename>          |                Delete a file named "filename"                |
|           fopen &lt;filename>           |                Open the file named "filename"                |
|             fclose &lt;fd>              |                     Close the file  "fd"                     |
|        fread &lt;fd> &lt;nbytes>        | Read nbytes from the file "fd", starting from the current file pointer |
|  fwrite &lt;fd> &lt;nbytes>&lt;string>  | Write nbytes of the "string" to the file "fd" at the current file pointer position |
| flseek &lt;fd> &lt;offset> &lt;ptrname> | Move the file pointer of the file "fd" by offset bytes in the direction specified by ptrname |
|        cp &lt;file1> &lt;file2>         |             Copy the contents of file1 to file2            |
|           ftree &lt;dirname>            | Display the directory tree under the folder named “dirname”  |
|                   pwd                   |                   Display the current path                   |
|      Frename &lt;file1> &lt;file2>      |                    Rename file1 to file2                     |
|                  Exit                   |                 Exit the current file system                 |

