/* disk.c: SimpleFS disk emulator */

#include "sfs/disk.h"
#include "sfs/logging.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

/* Internal Prototyes */

bool    disk_sanity_check(Disk *disk, size_t blocknum, const char *data);

/* External Functions */

/**
 *
 * Opens disk at specified path with the specified number of blocks by doing
 * the following:
 *
 *  1. Allocate Disk structure and sets appropriate attributes.
 *
 *  2. Open file descriptor to specified path.
 *
 *  3. Truncate file to desired file size (blocks * BLOCK_SIZE).
 *
 * @param       path        Path to disk image to create.
 * @param       blocks      Number of blocks to allocate for disk image.
 *
 * @return      Pointer to newly allocated and configured Disk structure (NULL
 *              on failure).
 **/
Disk *	disk_open(const char *path, size_t blocks) {
    //disk_open complete and correct (I think)
    Disk * new_disk = malloc(sizeof(Disk));
    int fd = open(path, O_RDWR | O_CREAT, 0644);
    //printf("\n\nFile descriptor: %i\n\n", fd);
    if(fd<0){
        //printf("THIS IS THE BAD FILE POLICE REROOREROO");
        if (!truncate(path, blocks*BLOCK_SIZE)){
            free(new_disk);
            return NULL;
        }
        fd = open(path, O_RDONLY);
        if(fd<0){
            free(new_disk);
            return NULL;
        }
    }
    if(blocks>BLOCK_SIZE){
        free(new_disk);
        return NULL;
    }
    new_disk->fd = fd;
    new_disk->reads = 0;
    new_disk->writes = 0;
    new_disk->blocks = blocks;
    return new_disk;
}

/**
 * Close disk structure by doing the following:
 *
 *  1. Close disk file descriptor.
 *
 *  2. Report number of disk reads and writes.
 *
 *  3. Release disk structure memory.
 *
 * @param       disk        Pointer to Disk structure.
 */
void	disk_close(Disk *disk) {
    //complete and correct (I think)
    close(disk->fd);
    printf("%zu disk block reads\n" , disk->reads);
    printf("%zu disk block writes\n", disk->writes);
    free(disk);
}

/**
 * Read data from disk at specified block into data buffer by doing the
 * following:
 *
 *  1. Perform sanity check.
 *
 *  2. Seek to specified block.
 *
 *  3. Read from block to data buffer (must be BLOCK_SIZE).
 *
 * @param       disk        Pointer to Disk structure.
 * @param       block       Block number to perform operation on.
 * @param       data        Data buffer.
 *
 * @return      Number of bytes read.
 *              (BLOCK_SIZE on success, DISK_FAILURE on failure).
 **/
ssize_t disk_read(Disk *disk, size_t block, char *data) {
    // incorrect
    if (disk_sanity_check(disk, block, data)){
        if(lseek(disk->fd,block*BLOCK_SIZE,SEEK_SET)<0){
            //printf("\n\nLSEEK FAIL\n\n");
            return DISK_FAILURE; //correct
        }
        if (read(disk->fd, data, BLOCK_SIZE) == BLOCK_SIZE){
            disk->reads++;
            return BLOCK_SIZE;
        }
        //printf("\n\nLSEEK/READ FAIL\n\n");
        return DISK_FAILURE;
    }
    //printf("\n\nSANITY CHECK FAILED\n\n");
    return DISK_FAILURE;
}

/**
 * Write data to disk at specified block from data buffer by doing the
 * following:
 *
 *  1. Perform sanity check.
 *
 *  2. Seek to specified block.
 *
 *  3. Write data buffer (must be BLOCK_SIZE) to disk block.
 *
 * @param       disk        Pointer to Disk structure.
 * @param       block       Block number to perform operation on.
 * @param       data        Data buffer.
 *
 * @return      Number of bytes written.
 *              (BLOCK_SIZE on success, DISK_FAILURE on failure).
 **/
ssize_t disk_write(Disk *disk, size_t block, char *data) {
    //incorrect
    if (disk_sanity_check(disk, block, data)){
        if(lseek(disk->fd,block*BLOCK_SIZE,SEEK_SET)<0){
            //printf("\n\nLSEEK FAIL\n\n");
            return DISK_FAILURE; //correct
        }
        //definitely wrong. help
        if (write(disk->fd, data, BLOCK_SIZE) == BLOCK_SIZE){
            disk->writes++;
            return BLOCK_SIZE;
        }
        //printf("\n\nLSEEK/READ FAIL\n\n");
        return DISK_FAILURE;
    }
    //printf("\n\nSANITY CHECK FAILED\n\n");

    return DISK_FAILURE;

}

/* Internal Functions */

/**
 * Perform sanity check before read or write operation by doing the following:
 *
 *  1. Check for valid disk.
 *
 *  2. Check for valid block.
 *
 *  3. Check for valid data.
 *
 * @param       disk        Pointer to Disk structure.
 * @param       block       Block number to perform operation on.
 * @param       data        Data buffer.
 *
 * @return      Whether or not it is safe to perform a read/write operation
 *              (true for safe, false for unsafe).
 **/
bool    disk_sanity_check(Disk *disk, size_t block, const char *data) {
    //sanity check works...for now
    if (!disk){ //correct
        //printf("\n\nBAD DISK\n\n");
        return false;
    }
    if (!data){ //correct
        //printf("\n\nBAD DATA\n\n");
        return false;
    }
    if (block>=disk->blocks){ //correct
        //printf("\n\nBAD BLOCK\n\n");
        return false;
    }
    return true;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
