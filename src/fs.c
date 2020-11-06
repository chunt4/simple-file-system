/* fs.c: SimpleFS file system */

#include "sfs/fs.h"
#include "sfs/logging.h"
#include "sfs/utils.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

/* External Functions */

/**
 * Debug FileSystem by doing the following:
 *
 *  1. Read SuperBlock and report its information.
 *
 *  2. Read Inode Table and report information about each Inode.
 *
 * @param       disk        Pointer to Disk structure.
 **/
void    fs_debug(Disk *disk) {
    Block block;
    // Read SuperBlock
    if (disk_read(disk, 0, block.data) == DISK_FAILURE) {
        return;
    }

    // Print SuperBlock Information
    printf("SuperBlock:\n");
    if (block.super.magic_number == MAGIC_NUMBER)
        printf("    magic number is valid\n");
    else
        printf("    magic number is not valid\n");
    printf("    %u blocks\n"         , block.super.blocks);
    printf("    %u inode blocks\n"   , block.super.inode_blocks);
    printf("    %u inodes\n"         , block.super.inodes);
    
    // Print Inode Information
    for (uint32_t k = 1; k <= block.super.inode_blocks; k++){
        Block block2;
        // Check if the inode block successfully reads
        if (disk_read(disk, k, block2.data) == BLOCK_SIZE){
            // Read Inodes
            for (uint32_t i = 0; i < INODES_PER_BLOCK; i++){
                // Check if inode is valid
                if (block2.inodes[i].valid==1){
                    // Print General Inode Information
                    printf("Inode %u:\n", i);
                    printf("    size: %u bytes\n", block2.inodes[i].size);
                    char buffer[BUFSIZ] = {NULL};
                    // Print direct blocks
                    for (uint32_t j = 0; j < POINTERS_PER_INODE; j++){
                        if (block2.inodes[i].direct[j] > 0)
                            sprintf(buffer, "%s %u", buffer, block2.inodes[i].direct[j]);
                    }
                    printf("    direct blocks:%s\n", buffer);
                    // Check and print indirect block
                    if (block2.inodes[i].indirect){
                        Block block3;
                        printf("    indirect block: %u\n", block2.inodes[i].indirect);
                    // Use indirect block number to disk read to get the indirect block, which is an array of pointers so you can iterate through it
                        char indirectbuf[BUFSIZ] = {NULL};
                        // Read indirect block and print pointers
                        if(disk_read(disk, block2.inodes[i].indirect, block3.data) == BLOCK_SIZE){
                            for (uint32_t j = 0; j < INODES_PER_BLOCK; j++){
                                if (block3.pointers[j]>0){
                                    sprintf(indirectbuf, "%s %u", indirectbuf, block3.pointers[j]);
                                }
                            }
                        }
                        printf("    indirect data blocks:%s\n", indirectbuf);
                    }
                }
            }
        }
    }
}

/**
 * Format Disk by doing the following:
 *
 *  1. Write SuperBlock (with appropriate magic number, number of blocks,
 *  number of inode blocks, and number of inodes).
 *
 *  2. Clear all remaining blocks.
 *
 * Note: Do not format a mounted Disk!
 *
 * @param       fs      Pointer to FileSystem structure.
 * @param       disk    Pointer to Disk structure.
 * @return      Whether or not all disk operations were successful.
 **/
bool    fs_format(FileSystem *fs, Disk *disk) {
    //printf("\n\n%d\n\n",disk->blocks);
    //printf("\n\n%d\n\n",fs->meta_data.blocks);
    if (fs->disk){
        return false;
    }
    Block block;
    block.super.magic_number = MAGIC_NUMBER;
    block.super.blocks = disk->blocks;
    if (disk->blocks%10 == 0){
        block.super.inode_blocks = disk->blocks/10;
    }
    else{
        block.super.inode_blocks = (int)disk->blocks/10 + 1;
    }
    block.super.inodes = block.super.inode_blocks*INODES_PER_BLOCK;

    if(disk_write(disk, 0, block.data)==DISK_FAILURE){
        return false;
    }
    
    for(uint32_t a = 1; a <= block.super.inode_blocks; a++){
        Block block2;
        //clear the inode blocks
        for (uint32_t d = 0; d < INODES_PER_BLOCK; d++){
            block2.inodes[d].valid = 0;
            block2.inodes[d].size = 0;
            for (uint32_t e = 0; e < POINTERS_PER_INODE; e++){
                block2.inodes[d].direct[e] = NULL;
            }
            block2.inodes[d].indirect = NULL;
        }
        if(disk_write(disk, a, block2.data)==DISK_FAILURE){
            return false;
        }
    }

    for (uint32_t b = block.super.inode_blocks+1; b < block.super.blocks; b++){
        Block block3;
        //clear the data blocks
        for (uint32_t c = 0; c < BLOCK_SIZE; c++){
            block3.data[c] = 0;
        }
        if(disk_write(disk, b, block3.data)==DISK_FAILURE){
            return false;
        }
    }

    return true;
}

/**
 * Mount specified FileSystem to given Disk by doing the following:
 *
 *  1. Read and check SuperBlock (verify attributes).
 *
 *  2. Verify and record FileSystem disk attribute. 
 *
 *  3. Copy SuperBlock to FileSystem meta data attribute
 *
 *  4. Initialize FileSystem free blocks bitmap.
 *
 * Note: Do not mount a Disk that has already been mounted!
 *
 * @param       fs      Pointer to FileSystem structure.
 * @param       disk    Pointer to Disk structure.
 * @return      Whether or not the mount operation was successful.
 **/
bool    fs_mount(FileSystem *fs, Disk *disk) {
    // Verify disk
    if(!disk){
        return false;
    }
    // Read SuperBlock
    Block block;
    if(disk_read(disk, 0, block.data)==DISK_FAILURE){
        //printf("\n\nHELLO FAILURE\n\n");
        return false;
    }
    if(fs->disk){
        return false;
    }
    fs->disk = disk;
    // Copy Superblock to meta_data
    fs->meta_data = block.super;

    bool *bitmap = calloc(block.super.blocks, sizeof(bool));
    for (int a = 0; a < block.super.blocks; a++){
        bitmap[a] = true;
    }
    bitmap[0]=false;
    //printf("\n\nSET UP BITMAP\n\n");
    for (uint32_t i = 1; i <= block.super.inode_blocks; i++){
        Block block2;
        //printf("\n\nINODE BLOCK %i\n\n", i);
        if(disk_read(disk, i, block2.data)==BLOCK_SIZE){
            for (uint32_t j = 0; j < INODES_PER_BLOCK; j++){
                //printf("\n\nValid: %i\n\n", block2.inodes[j].valid);
                if (block2.inodes[j].valid==1){
                    //printf("\n\nVALID INODE %u\n\n", j);
                    for (uint32_t k = 0; k < POINTERS_PER_INODE; k++){
                        if(block2.inodes[j].direct[k]>0){
                            //printf("\n\nDIRECT %u\n\n",block2.inodes[j].direct[k]);
                            bitmap[block2.inodes[j].direct[k]]=false;
                        }
                    }
                    //printf("\n\nBut does it make it here?\n\n");
                    if (block2.inodes[j].indirect){
                        Block block3;
                        bitmap[block2.inodes[j].indirect]=false;
                        if(disk_read(disk,block2.inodes[j].indirect,block3.data)==BLOCK_SIZE){
                            for (uint32_t m = 0; m < INODES_PER_BLOCK; m++){
                                if(block3.pointers[m]>0){
                                    bitmap[block3.pointers[m]]=false;
                                }
                            }
                        }
                    }
                }
            }
            bitmap[i]=false;
        }
    }
    fs->free_blocks = bitmap;
    /*printf("\n\n%i\n\n",fs->free_blocks[0]);
    printf("\n\n%i\n\n",fs->free_blocks[1]);
    printf("\n\n%i\n\n",fs->free_blocks[2]);
    printf("\n\n%i\n\n",fs->free_blocks[3]);
    printf("\n\n%i\n\n",fs->free_blocks[4]);*/

    return true;
}

/**
 * Unmount FileSystem from internal Disk by doing the following:
 *
 *  1. Set FileSystem disk attribute.
 *
 *  2. Release free blocks bitmap.
 *
 * @param       fs      Pointer to FileSystem structure.
 **/
void    fs_unmount(FileSystem *fs) {
    fs->disk = NULL;
    free(fs->free_blocks);
}

/**
 * Allocate an Inode in the FileSystem Inode table by doing the following:
 *
 *  1. Search Inode table for free inode.
 *
 *  2. Reserve free inode in Inode table.
 *
 * Note: Be sure to record updates to Inode table to Disk.
 *
 * @param       fs      Pointer to FileSystem structure.
 * @return      Inode number of allocated Inode.
 **/
ssize_t fs_create(FileSystem *fs) {
    return -1;
}

/**
 * Remove Inode and associated data from FileSystem by doing the following:
 *
 *  1. Load and check status of Inode.
 *
 *  2. Release any direct blocks.
 *
 *  3. Release any indirect blocks.
 *
 *  4. Mark Inode as free in Inode table.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to remove.
 * @return      Whether or not removing the specified Inode was successful.
 **/
bool    fs_remove(FileSystem *fs, size_t inode_number) {
    return false;
}

/**
 * Return size of specified Inode.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to remove.
 * @return      Size of specified Inode (-1 if does not exist).
 **/
ssize_t fs_stat(FileSystem *fs, size_t inode_number) {
    return -1;
}

/**
 * Read from the specified Inode into the data buffer exactly length bytes
 * beginning from the specified offset by doing the following:
 *
 *  1. Load Inode information.
 *
 *  2. Continuously read blocks and copy data to buffer.
 *
 *  Note: Data is read from direct blocks first, and then from indirect blocks.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to read data from.
 * @param       data            Buffer to copy data to.
 * @param       length          Number of bytes to read.
 * @param       offset          Byte offset from which to begin reading.
 * @return      Number of bytes read (-1 on error).
 **/
ssize_t fs_read(FileSystem *fs, size_t inode_number, char *data, size_t length, size_t offset) {
    return -1;
}

/**
 * Write to the specified Inode from the data buffer exactly length bytes
 * beginning from the specified offset by doing the following:
 *
 *  1. Load Inode information.
 *
 *  2. Continuously copy data from buffer to blocks.
 *
 *  Note: Data is read from direct blocks first, and then from indirect blocks.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to write data to.
 * @param       data            Buffer with data to copy
 * @param       length          Number of bytes to write.
 * @param       offset          Byte offset from which to begin writing.
 * @return      Number of bytes read (-1 on error).
 **/
ssize_t fs_write(FileSystem *fs, size_t inode_number, char *data, size_t length, size_t offset) {
    return -1;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
