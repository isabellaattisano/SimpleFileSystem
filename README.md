# A Simple File System Implementation 

## *Additional Project Details*

Additional project details can be found on my website : [Project Details](https://isabellaattisano.github.io/bella-attisano/projects/fs/fs.html)

## *What is a File System?*

A file system is a data structure used by the operating system to store, retrieve, and organize data on a hard disk or other storage device by invoking the disk's internal functions. The goal of a file system is to provide an abstraction between the user and their stored data. Moreover, users can create files, directories, and specify file details without directly invoking disk functions. Then, when a user or user application invokes the file system interface, the file system will begin to access the data blocks of the disk to which it is mounted through using the core file system structures such as inodes and blocks. 

## *Project Overview*

In this project I endeavored to implement a very simple file system using C. It seeks to highlight the primary data structures and access methods used by the file system. My file system is a disk-based file system and uses a disk image file to simulate how the file system would interact with true hardware.  
