#ifndef C_EXT2_FILESYSTEM_ALLOCATION_H
#define C_EXT2_FILESYSTEM_ALLOCATION_H

#include <stdint.h>
#include <stdio.h>

// Forward declarations
struct ext2_super_block;
struct ext2_group_desc;

/**
 * @brief Allocates a new inode in the filesystem.
 *
 * This function finds the first available inode, marks it as used in the inode bitmap,
 * updates the relevant filesystem metadata (superblock and block group descriptor),
 * and writes these changes back to the disk.
 *
 * @param fp Pointer to the filesystem image file, opened for reading and writing.
 * @param sb Pointer to the superblock structure (will be updated if an inode is allocated).
 * @param gdt Pointer to the array of block group descriptors (will be updated).
 * @param num_block_groups The total number of block groups.
 * @param new_inode_num_out Pointer to a uint32_t where the number of the newly allocated inode will be stored.
 * @return 0 on success, or a negative error code on failure.
 */
int allocate_inode(FILE *fp, struct ext2_super_block *sb, struct ext2_group_desc *gdt, uint32_t num_block_groups, uint32_t *new_inode_num_out);

/**
 * @brief Allocates a new data block in the filesystem.
 *
 * This function finds the first available data block, marks it as used in the block bitmap,
 * updates the relevant filesystem metadata (superblock and block group descriptor),
 * and writes these changes back to the disk.
 *
 * @param fp Pointer to the filesystem image file, opened for reading and writing.
 * @param sb Pointer to the superblock structure (will be updated).
 * @param gdt Pointer to the array of block group descriptors (will be updated).
 * @param num_block_groups The total number of block groups.
 * @param new_block_num_out Pointer to a uint32_t where the number of the newly allocated block will be stored.
 * @return 0 on success, or a negative error code on failure.
 */
int allocate_block(FILE *fp, struct ext2_super_block *sb, struct ext2_group_desc *gdt, uint32_t num_block_groups, uint32_t *new_block_num_out);

#endif //C_EXT2_FILESYSTEM_ALLOCATION_H
