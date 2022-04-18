#include "disk.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

Disk d; Disk *disk = &d;

/*internal*/
bool disk_sanity_check(size_t disk_blocks, size_t blocknum, const char *data);

/*opens  disk at the path with the specified number of blocks
* 1. Allocates Disk Structure and sets appropriate attributes
* 2. Opens file descriptor to path
* 3. Truncates file to desired length 
* 4. returns a pointer to newly allocated andconfigured Disk Structure (NULL if fails)
*/
Disk* disk_open(const char *path, size_t blocks){

    disk->blocks=blocks; disk->reads=0; disk->writes=0;
    int total = blocks * BLOCK_SIZE;
   
    //open file descriptor 
    disk->fd = open(path, O_RDWR|O_CREAT); 
    if(disk->fd < 0)
        printf("Unable to open '%s'\n", path);

    //truncates file to desired length 5
    if(ftruncate(disk->fd, total) < 0)
         printf("Truncation Error for '%s'\n", path);

    //if works check size of file 
    // struct stat st; fstat(disk->fd, &st);
    // printf("the file has %ld bytes\nthe file has %ld blocks\n", st.st_size, disk->blocks);

    return disk;
}

/* close disk structure
* 1. close fd
* 2. report number of disk reads and writes
* 3. releasing disk structure memory 
*/


void disk_close(Disk *disk)
{   
   if(close(disk->fd))
        printf("the disk failed to close");
   
   disk->reads=0; disk->writes=0;
}

/*Read Function
* Reads data from disk at specifed block into data buffer by 
* doing the following:
* 1. Sanity Check
* 2. Seeking to specified block
* 3. Reading from block to data buffer (must be BLOCK_SIZE)
* returns Number of Bytes read
*/
ssize_t disk_read(Disk *disk, size_t block, char *data){
   
    bool san = disk_sanity_check(disk->blocks, block, data);
    if(!san){
        printf(" passed to read function\n");
        return -1;
    }

    if(lseek(disk->fd, BLOCK_SIZE*block, SEEK_SET)<0){
        printf("lseek problem\n");
        return -1;
    }   
    ssize_t re = read(disk->fd, data, BLOCK_SIZE);

    disk->reads++;
    return 0;
}

ssize_t disk_write(Disk *disk, size_t block, const char *data){

    bool san = disk_sanity_check(disk->blocks, block, data);
    if(!san){
        printf(" passed to write function\n");
        return -1;
    }

    if(lseek(disk->fd, BLOCK_SIZE*block, SEEK_SET)<0){
        printf("lseek problem\n");
        return -1;
    }

    size_t re = write(disk->fd, data, BLOCK_SIZE);

    disk->writes++;
    return 0;
}

/* Make sure conditions are valid for each function
 *  1. Check for valid disk.
 *  2. Check for valid block.
 *  3. Check for valid data.
 **/

bool disk_sanity_check(size_t disk_blocks, size_t block, const char *data) {
    if(block>disk_blocks){
        printf("Block is greater with size: %lu whereas block size is: %lu --> ", block, disk_blocks);
        return false;
    }
   
    if(data==NULL){
        printf("NULL Data\n");
        return false;
    }
    return true;
}


// int main(){
  
//    Disk *d = disk_open("data.txt", 10);
//    printf("%zu\n", disk->blocks);

//     char w[BLOCK_SIZE] = "hi";
//     disk_write(disk, 0, w);

//    char s[BLOCK_SIZE];
//     disk_read(disk, 0, s);
//     printf("this is what was read %s\n", s);

//    disk_close(disk);
  
 
// } 