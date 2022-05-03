/*file system implementation */

#include "fs.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

void fs_initialize_free_block_bitmap(FileSystem *fs);


/*Format Disk
* wrtie superblock with magin number, number of blocks, number of inode blocks, number of inodes
* Clear all remaining blocks 
* Do not format a mounted disk 
*/

bool fs_format(Disk *disk){

    //do not format a mounted disk
    if(disk->mounted) 
        return false;

    Block block; 

    // set superblock
    block.super.magic_number = MAGIC_NUMBER; 
    block.super.blocks = disk->blocks;
    block.super.inode_blocks = (uint32_t) (block.super.blocks * 0.1)+1;
    block.super.inodes = INODES_PER_BLOCK* block.super.inode_blocks;
    
    //write superblock
    if(disk_write(disk, 0, block.data)==-1) //because its a union this information is stored in the same memory space
            return false;                   //it will write the whole superblock to the disk and we can access this informatio using the union
  

    //clear the inode table --> the inode table is the 10% of disk blocks set aside for inode blocks (these blocks store the inode table)
    //the inode table contains 8 fields 

    /* enter block i which contains 128 inodes to clear (i represents the 4KB we will write to so it will get overwritten)*/
    /* starts at 1 because superblock at i = 0 */

    for(uint32_t i =1; i<=block.super.inode_blocks; i++){ 
        Block inode; 
        disk_read(disk, i, inode.data);
        
        //goes through the 128 inodes of block i 
        //view as a inode block containing 128 inodes 
        for(uint32_t j = 0; j<INODES_PER_BLOCK; j++){
            inode.inodes[j].size = 0;
            inode.inodes[j].valid = 0;
            inode.inodes[j].indirect =0; 
            inode.inodes[j].name = "";
            inode.inodes[j].pass = "";
            //take the inode at j index in block i  
            //go through 5 direct pointers 
            for(uint32_t k = 0; k<POINTERS_PER_INODE; k++)
                inode.inodes[j].direct[k] = 0; //sets address to 0 
            
        }
        //write to block i on the disk (an inode block)
        if(disk_write(disk, i, inode.data)==-1)
            return false;
    }
    return true;
}

/* Mount given FS to the given Disk 
* Read and check superblock
* Record FS disk attribute 
* set disk mount status
* copy super to FS meta data attribute
* initalize bitmap 
*/
bool fs_mount(FileSystem *fs, Disk *disk) {
     
     //we can't mount a fs to a disk with a fs
     if(disk->mounted)
        return false;
    
    Block block;

    //read and check superblock
    disk_read(disk, 0, block.data);

    if(block.super.magic_number != MAGIC_NUMBER || 
        block.super.blocks != disk->blocks || 
        block.super.inode_blocks != ((uint32_t) (block.super.blocks * 0.1)+1) ||  //don't use disk attribute because we already checked disk->blocks
        block.super.inodes != (block.super.inode_blocks * INODES_PER_BLOCK) ){
        return false;
    }

    //record fs disk attribute
    fs->disk = disk;

    //set disk mount status
    disk->mounted = true;

    //copy super to FS meta data 
    fs->meta_data = block.super;

    //initalize bitmap 
    fs_initialize_free_block_bitmap(fs);

    return true;
}

void    fs_unmount(FileSystem *fs){
    fs->disk = NULL;

}

/* intialize the free block bitmap when mounting system
*  fs->free_blocks is the bitmap
*/
void  fs_initialize_free_block_bitmap(FileSystem *fs){

    // go through inode table 
    // if not zero than change the value to true meaning its in use 
    //use true as in use because by default set it to false/0 

    Block block; 

    fs->free_blocks = malloc(sizeof(uint32_t)*fs->meta_data.blocks); //allocate specific size in memory of array

    //mark inode blocks and superblock(start at 0) as being used
    for(uint32_t i =0; i<fs->meta_data.inode_blocks; i++){
        fs->free_blocks[i] = true;
    }

    for(uint32_t i =1; i<=fs->meta_data.inode_blocks; i++){  //go through each inode block because an blocks in use will be pointed to by an inode
        Block inode;

        disk_read(fs->disk, i, inode.data); //read from i inode block

         //goes through the 128 inodes of block i 
        for(uint32_t j = 0; j<INODES_PER_BLOCK; j++){

           if((inode.inodes[j].valid)!=0){ //if the inode it being used if not all the direct pointers / indirect pointers will also be 0 

                //go through 5 direct pointers 
                for(uint32_t k = 0; k<POINTERS_PER_INODE; k++){
                    if(inode.inodes[j].direct[k] > fs->meta_data.blocks)  //if the block it points to is greater than the number of blocks on the disk 
                        inode.inodes[j].direct[k] = 0;

                    if(inode.inodes[j].direct[k]) //if anything other than 0  
                        fs->free_blocks[inode.inodes[j].direct[k]] = true; //set the block # to true
                    
                }  

                //go through indirect pointers
                if(inode.inodes->indirect!=0){  //if its 0 it won't point to a block containing other pointers
                    fs->free_blocks[inode.inodes[j].indirect] = true; //the block it points to is not free
                    
                    Block indirect;
                    disk_read(fs->disk, inode.inodes[j].indirect, indirect.data); //read the block so we can go through it and find how many other blocks it points to
                    for(int k =0; k<POINTERS_PER_BLOCK; k++)
                        if(indirect.pointers[k])
                            fs->free_blocks[indirect.pointers[k]] = true;
                    
                }   
            } 
        } 
    }
}

/* Create --> allocate an inode in the FileSystem 
* search inode table for free inode
* reserve free inode in inode table 
* record updates to Inode table to disk 
* return inode number
*/

ssize_t fs_create(FileSystem *fs, char *name) { 
    //if the fs is not mounted to a disk
    if(!fs->disk->mounted)
        return -1;

      //locate free node in inode table located in the i blocks allocated to the inode_blocks
      for(uint32_t i =1; i<=fs->meta_data.inode_blocks; i++){ 
       
        Block b; 

        disk_read(fs->disk, i, b.data);

        //goes through the 128 inodes of block i to find empty inode
        for(uint32_t j = 0; j<INODES_PER_BLOCK; j++){
            if((b.inodes[j].valid)==0){ //its empty 
                b.inodes[j].valid = true;
                b.inodes[j].size = 0;
                b.inodes[j].pass = create_pass(); 
                b.inodes[j].name = name;
    
                b.inodes[j].indirect = 0;
                for(int k = 0; k<POINTERS_PER_INODE; k++)
                    b.inodes[j].direct[k] = 0;

                disk_write(fs->disk, i, b.data); //write the updated information about the inode
                // printf("creating: %d with name %s %ld\n", (INODES_PER_BLOCK*(i-1) + j), name, b.inodes[j].size); //for debugging prints out return val

                return (INODES_PER_BLOCK*(i-1) + j); //returns inode location in the table not just individual block
            }
        
        }
    }
    
    return -1;
}

/**
 * Remove Inode and associated data from FileSystem by doing the following:
 *  1. Load and check status of Inode.
 *  2. Release any direct blocks.
 *  3. Release any indirect blocks.
 *  4. Mark Inode as free in Inode table.
**/
bool fs_remove(FileSystem *fs, char *name) {
    
    size_t inode_number;

    Inode inode;

    if(load_inode(fs, name, &inode_number, &inode)){
        //update information
        check_pass(inode.pass, 3); //checks user pass before allowing access
        // printf("removing %s with number %lu\n", inode.name, inode_number);
        inode.valid = false; 
        inode.size = 0;

        //free direct blocks;
        for(uint32_t k = 0; k<POINTERS_PER_INODE; k++){
            fs->free_blocks[inode.direct[k]]=false;
            inode.direct[k] = 0; 
        }  

        //indrect 
        if(inode.indirect){ //if it had any indirect pointers remove them
            Block indirect;
            disk_read(fs->disk, inode.indirect, indirect.data);
            fs->free_blocks[inode.indirect]=false;
            inode.indirect=0;

            for(uint32_t i = 0; i < POINTERS_PER_BLOCK; i++) {
                if(indirect.pointers[i]) 
                    fs->free_blocks[indirect.pointers[i]] = false;
            }
        }  
            //write out the information
            Block block;
            disk_read(fs->disk, inode_number / INODES_PER_BLOCK + 1, block.data);
            block.inodes[inode_number % INODES_PER_BLOCK] = inode;
            block.inodes[inode_number % INODES_PER_BLOCK].size =0;
            disk_write(fs->disk, inode_number / INODES_PER_BLOCK + 1, block.data);

            return true;
    }
    else{
        printf("failed to remove %s\n", name);
        return false;
    }

}

bool load_inode(FileSystem *fs, char *name, size_t *inode_number, Inode *inode){

    if(!fs->disk->mounted)
        return false;
    
    for(uint32_t i =1; i<=fs->meta_data.inode_blocks; i++){ 
        Block b; 
        disk_read(fs->disk, i, b.data);

        //goes through the 128 inodes of block i to find inode with name matching the name given
        for(uint32_t j = 0; j<INODES_PER_BLOCK; j++){
            if(b.inodes[j].valid){
                if(strcmp(b.inodes[j].name, name)==0){ //if the names match  
                    *inode_number = (INODES_PER_BLOCK*(i-1) + j);
                    // printf("%s matches %s with number %lu\n", name, b.inodes[j].name, *inode_number); 
                    // // int blocknum = *inode_number / INODES_PER_BLOCK +1;
                    // // int inode_in_block_number = *inode_number % INODES_PER_BLOCK; 

                    // // Block block;

                    // // disk_read(fs->disk, blocknum, block.data); //read in the needed inode block

                    if(b.inodes[j].valid){ //if the inode is being used
                        *inode = b.inodes[j]; //return the particular inode we want to modify
                        return true;
                    }
            }

        }
    }
    }
    return false;

}

ssize_t fs_stat(FileSystem *fs, char *name){
    Inode inode;
    size_t inode_number;
     if(load_inode(fs, name, &inode_number, &inode)){
         return inode.size;
     }
}

void reading(FileSystem *fs, char **data, ssize_t *length, size_t *offset, uint32_t block){
    
    disk_read(fs->disk, block, *data); //reads whole block
    printf("%s ", *data);
    //update variables
    *length -= (BLOCK_SIZE + (int)*offset);
    *data = *data + *offset;
    *offset = 0;

}

/**
 * Read from the specified Inode into the data buffer exactly length bytes beginning from the specified offset by doing the following 
 * 1. Load Inode Information
 * 2. Continuously read blocks and caopy data to buffer
 * 3. read from direct blocks than indirect blocks 
 */
ssize_t fs_read(FileSystem *fs, char *name, size_t inode_number, char *data, ssize_t length, size_t offset) {
   
    size_t original = length;

    Inode inode;
    if(load_inode(fs, name, &inode_number, &inode)){
        check_pass(inode.pass, 0); //checks user pass before allowing access
        printf("\nReading from: %s (#%lu)\n\n", inode.name, inode_number);
        int block = inode_number / INODES_PER_BLOCK +1; 
        int inode_in_block_number = inode_number % INODES_PER_BLOCK; 
        int sizeofinode = inode.size;
        
        if(sizeofinode<=(int)offset){
            return 0; //nothing to read because the size of the node contains how many bytes the file is, 
                      //if offset is too many bytes into the file nothing has been written there
         } else if(length + (int)offset > sizeofinode)
            length = sizeofinode - offset;     //if the number of bytes needing to be read from a spefific offset exceeds the file size 
            //length + offset = size of inode  //reduce the length to the end of the file so we do not read out of memory 
        
        //find where reading starts from 
        if(offset<POINTERS_PER_INODE*BLOCK_SIZE){ //start in direct pointers
            //which pointer to start reading from 
            uint32_t directnum = offset/BLOCK_SIZE; //which direct block 
            offset = offset % BLOCK_SIZE;       //where in that block 

            if(inode.direct[directnum]){

                reading(fs, &data, &length, &offset, inode.direct[directnum]);
                directnum++;

            
                while(length > 0 && directnum < POINTERS_PER_INODE && inode.direct[directnum]){
                    reading(fs, &data, &length, &offset, inode.direct[directnum++]);
                }

                if(length <= 0)
                    return original;
                
               
                else{
                    //if we went through all of the direct blocks and there is still more to read we go through indirect
                   
                    if(inode.indirect){ //if it has more to be read
                        //read in indirect pointer block 
                        Block indirect;
                        disk_read(fs->disk, inode.indirect, indirect.data);

                        //go through indirect pointers 
                        
                        size_t indirectnum =0;
                        while(length > 0 && indirectnum < POINTERS_PER_BLOCK){
                            if(indirect.pointers[indirectnum])
                                reading(fs, &data, &length, &offset, indirect.pointers[indirectnum++]);
                            else
                                break;
                        }

                        free(data);

                        if(length<=0)
                            return original;
                        
                        else
                            return original - length;
                        

                    }else  {
                        free(data);
                        return original - length;
                    }
                }
                

            }
            else{ //no data to be read
            free(data);
                return 0;
            }
            
        }else{

            //offset begins in indirect blocks 

            if(inode.indirect) //if the indirect block contains values
            {
                offset = offset - POINTERS_PER_INODE * BLOCK_SIZE;
                uint32_t indirectblock = offset / BLOCK_SIZE;
                offset%=BLOCK_SIZE;

                Block indirect;
                disk_read(fs->disk, inode.indirect, indirect.data);

                //iterate through indirect blocks until length < 0

                if(length >0 && indirect.pointers[indirectblock])
                    reading(fs, &data, &length, &offset, indirect.pointers[indirectblock]);

                indirectblock++;

                for(uint32_t i = indirectblock; i < POINTERS_PER_BLOCK; i++) {
                        if(indirect.pointers[i] && length > 0) {
                            reading(fs, &data, &length, &offset, indirect.pointers[i]);
                        }
                        else break;
                }
                free(data);
                if(length<=0)
                    return original;
                else
                    return original - length;

            }
        }


        }else {//no data to be read
                free(data);
                return 0;
        } 
    free(data);
    return -1;
}

/**
 * Write to the specified Inode from the data buffer exactly length bytes
 * beginning from the specified offset by doing the following:
 *  1. Load Inode information.
 *  2. Continuously copy data from buffer to blocks.
 *  Note: Data is read from direct blocks first, and then from indirect blocks.
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to write data to.
 * @param       data            Buffer with data to copy
 * @param       length          Number of bytes to write.
 * @param       offset          Byte offset from which to begin writing.
 * @return      Number of bytes read (-1 on error).
 **/

size_t allocate_block(FileSystem *fs){
   if(!fs->disk->mounted) 
        return 0;
    Block b;
    disk_read(fs->disk, 1, b.data);
   for(size_t i = fs->meta_data.inode_blocks+1; i<fs->meta_data.blocks; i++){
       if(!fs->free_blocks[i]){
           fs->free_blocks[i] = true;
           return i;
       }
   }
   return 0;
}

void writing(FileSystem *fs, char **data, ssize_t *length, size_t *offset, uint32_t block){
    if(*offset >0){
        Block read;
        disk_read(fs->disk, block, read.data);
        strcat(read.data, *data);
        strcpy(*data, read.data);
    }
    ssize_t ret = disk_write(fs->disk, block, *data); //reads whole block

    //update variables
    ssize_t sub = (BLOCK_SIZE + *offset);
    *length = *length - sub;
    // printf("\twriting:: %s --> to block %lu\n", *data, block);
    // printf("\nlength %ld\n\n", *length);
    *data = *data + BLOCK_SIZE;
    // printf("\n new data to write %s\n\n", *data);
    *offset = 0;
    
}

ssize_t fs_write(FileSystem *fs, char* name, size_t inode_number, char *data, ssize_t length, size_t offset) {
    if(!fs->disk->mounted)
        return -1;

    size_t original = length; 

    if(length+offset>(POINTERS_PER_BLOCK+POINTERS_PER_INODE)*BLOCK_SIZE){
        printf("failed\n");
        return -1;
    }
    Inode inode;
    Block b;
    if(load_inode(fs, name, &inode_number, &inode)){
        printf("\nWriting to: %s (#%lu)\n", inode.name, inode_number);
        int block = inode_number / INODES_PER_BLOCK +1; 
        disk_read(fs->disk, block, b.data);
        int inode_in_block_number = (inode_number % INODES_PER_BLOCK); 
        ssize_t size = fs_stat(fs, name);
        b.inodes[inode_in_block_number].size = b.inodes[inode_in_block_number].size + length;
        disk_write(fs->disk, block, b.data);
        
         //find where writing starts from 
        if(offset<POINTERS_PER_INODE*BLOCK_SIZE){ //start in direct pointers
            //which pointer to start reading from
            uint32_t directnum = offset/BLOCK_SIZE; //which direct block 
            offset = offset % BLOCK_SIZE;       //where in that block 

                while(length > 0 && directnum<POINTERS_PER_INODE){
                    if(!inode.direct[directnum]){   //if block not allocated we allocate block
                        size_t ret = allocate_block(fs);
                        if(!ret)   //no free blocks available (Fails)
                            return length-original;
                        
                        inode.direct[directnum] = ret;  //allocates a free block to the direct pointer
                        b.inodes[inode_in_block_number].direct[directnum] = ret;
                        disk_write(fs->disk, block, b.data);
                       
                    }

                    writing(fs, &data, &length, &offset, inode.direct[directnum++]);
                }     

                if (length > 0) { //continute to write into indirect blocks
                    if(!inode.indirect){
                        size_t ret = allocate_block(fs);
                        if(!ret)   //no free blocks available (Fails)
                            return length-original;

                        inode.indirect = ret;
                        b.inodes[inode_in_block_number].indirect = ret;
                        disk_write(fs->disk, block, b.data);
                    }

                       
                        disk_read(fs->disk, inode.indirect, b.data);
                        size_t indirectnum = 0;

                    while(length > 0 && indirectnum < POINTERS_PER_BLOCK){
                        if(!b.pointers[indirectnum]){
                            size_t ret = allocate_block(fs);
                            if(!ret)   //no free blocks available (Fails)
                                return length-original;
                            b.pointers[indirectnum] = ret;
                            disk_write(fs->disk, inode.indirect, b.data);
                        }

                        writing(fs, &data, &length, &offset, b.pointers[indirectnum++]);
                    }

                }

                    if(length<=0){
                            return original;
                    }
                        
                        else{
                            return original - length;
                        }


            } else { // start in indirect pointers 
                if(!inode.indirect) //if the indirect block contains values
                {
                    size_t ret = allocate_block(fs);
                        if(!ret)   //no free blocks available (Fails)
                            return length-original;
                        inode.indirect = ret;
                        b.inodes[inode_in_block_number].indirect = ret;
                        disk_write(fs->disk, block, b.data);
                }
                    offset = offset - POINTERS_PER_INODE * BLOCK_SIZE;
                    uint32_t indirectblock = offset / BLOCK_SIZE;
                    offset%=BLOCK_SIZE;

                    disk_read(fs->disk, inode.indirect, b.data);

                    //iterate through indirect blocks until length < 0

                     while(length > 0 && indirectblock < POINTERS_PER_BLOCK){
                         if(!b.pointers[indirectblock] || b.pointers[indirectblock] > fs->meta_data.blocks){
                            size_t ret = allocate_block(fs);
                            if(!ret)   //no free blocks available (Fails)
                                return length-original;
                            b.pointers[indirectblock] = ret;
                            disk_write(fs->disk, inode.indirect, b.data);
                        }
                        writing(fs, &data, &length, &offset, b.pointers[indirectblock++]);
                    }

                    if(length<=0)
                        return original;
                    else
                        return original - length;

                
            }
    }
    else 
        printf("failed to load inode\n");
       
    return -1;

}
