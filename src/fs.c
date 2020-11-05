/* fs.c: SimpleFS file system */

#include "sfs/fs.h"
#include "sfs/logging.h"
#include "sfs/utils.h"

#include <stdio.h>
#include <string.h>

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
    //printf("\n\nFS_DEBUG START\n\n");
    /* Read SuperBlock */
    if (disk_read(disk, 0, block.data) == DISK_FAILURE) {
        printf("\n\nFor some reason this fires\n\n");
        return;
    }

    printf("SuperBlock:\n");
    if (block.super.magic_number == MAGIC_NUMBER)
        printf("    magic number is valid\n");
    else
        printf("    magic number is not valid\n");
    printf("    %u blocks\n"         , block.super.blocks);
    printf("    %u inode blocks\n"   , block.super.inode_blocks);
    printf("    %u inodes\n"         , block.super.inodes);
    
    for (uint32_t k = 1; k <= block.super.inode_blocks; k++){
        if (disk_read(disk, k, block.data) == BLOCK_SIZE){

            /* Read Inodes */
            for (uint32_t i = 1; i <= INODES_PER_BLOCK; i++){

                //printf("\n\n ENTERED THE FOR LOOP \n\n");
                /*if (disk_read(disk, i, block.data) == DISK_FAILURE){
                    return;
                }*/
                //printf("\n\n Valid: %i\n\n",block.inodes[i].valid);
                if (block.inodes[i].valid==1){
                    printf("Inode %u:\n", i);
                    printf("    size: %u bytes\n", block.inodes[i].size);
                    char buffer[BUFSIZ] = {NULL};
                    for (uint32_t j = 0; j < POINTERS_PER_INODE; j++){
                        if (block.inodes[i].direct[j] > 0)
                            sprintf(buffer, "%s %u", buffer, block.inodes[i].direct[j]);
                    }
                    printf("    direct blocks:%s\n", buffer);
                    if (block.inodes[i].indirect){
                        printf("    indirect block: %u\n", block.inodes[i].indirect);
                    // Use indirect block number to disk read to get the indirect block, which is an array of pointers so you can iterate through it
                        char indirectbuf[BUFSIZ] = {NULL};
                        if(disk_read(disk, block.inodes[i].indirect, block.data) == BLOCK_SIZE){
                            for (uint32_t j = 0; j < INODES_PER_BLOCK; j++){
                                if (block.pointers[j]>0){
                                    sprintf(indirectbuf, "%s %u", indirectbuf, block.pointers[j]);
                                }
                            }
                        }
                        printf("    indirect data blocks:%s\n", indirectbuf);
                    }
                }
            }
        }
    }

    // Print total disk block reads and writes
    // disk->blocks, disk->reads, and disk->writes are important here
    // disk->fd is also thing but probs not as important 
    //printf("%zu disk block reads\n", disk->reads);
    //printf("%zu disk block writes\n", disk->writes);
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
    return false;
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
    return false;
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
