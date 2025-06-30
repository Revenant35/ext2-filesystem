#ifndef INODE_H
#define INODE_H

#include <stdint.h>
#include <stdio.h>

#include "types.h"

/**
 * @brief Reads an inode from the filesystem into memory.
 *
 * @param file Pointer to an open FILE stream for the filesystem image.
 * @param superblock Pointer to the filesystem's superblock.
 * @param block_group_descriptor_table Pointer to the array of block group descriptors (the BLOCK_GROUP_DESCRIPTOR_TABLE).
 * @param inode_num The number of the inode to read (1-based).
 * @param inode_out Pointer to an `ext2_inode` structure to populate with the read data.
 * @return 0 on success, or a negative error code on failure.
 */
int read_inode(
    FILE *file,
    const ext2_super_block *superblock,
    const ext2_group_desc *block_group_descriptor_table,
    uint32_t inode_num,
    ext2_inode *inode_out
);

/**
 * @brief Writes an inode from memory to the filesystem.
 *
 * @param file Pointer to an open FILE stream for the filesystem image.
 * @param superblock Pointer to the filesystem's superblock.
 * @param block_group_descriptor_table Pointer to the array of block group descriptors (the BLOCK_GROUP_DESCRIPTOR_TABLE).
 * @param inode_num The number of the inode to write (1-based).
 * @param inode_in Pointer to an `ext2_inode` structure containing the data to write.
 * @return 0 on success, or a negative error code on failure.
 */
int write_inode(
    FILE *file,
    const ext2_super_block *superblock,
    const ext2_group_desc *block_group_descriptor_table,
    uint32_t inode_num,
    const ext2_inode *inode_in
);


#endif //INODE_H
