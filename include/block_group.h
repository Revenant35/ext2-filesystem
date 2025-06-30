/**
 * @file BlockGroup.h
 * @brief Defines the ext2 block group descriptor structure and related constants.
 *
 * An ext2 filesystem is divided into several block groups. Each block group has its
 * own bitmap for block usage, inode usage, and a portion of the inode table.
 * The block group descriptor contains pointers to these structures and other
 * metadata for the group.
 */
#ifndef BLOCK_GROUP_H
#define BLOCK_GROUP_H

#include <stdint.h>
#include <stdio.h>

#include "types.h"

/**
 * @brief Calculates the total number of block groups in the filesystem.
 *
 * The number of block groups can be derived from either the total block count
 * or the total inode count stored in the superblock. These two calculations
 * should ideally yield the same result. This function performs both and issues
 * a warning if they differ, then returns the count based on blocks.
 *
 * @param superblock Pointer to the superblock structure.
 * @return The total number of block groups.
 */
uint32_t count_block_groups(
    const ext2_super_block *superblock
);

/**
 * @brief Reads a single block group descriptor from the filesystem image.
 *
 * @param file Pointer to an open FILE stream for the filesystem image.
 * @param superblock Pointer to the filesystem's superblock (already read into memory).
 * @param group_index The index of the block group descriptor to read (0-based).
 * @return Pointer to an `ext2_group_desc` structure containing the data read from the filesystem,
 */
ext2_group_desc *read_group_descriptor(
    FILE *file,
    const ext2_super_block *superblock,
    uint32_t group_index
);

/**
 * @brief Writes a single block group descriptor from memory to the filesystem image.
 *
 * @param file Pointer to an open FILE stream for the filesystem image.
 * @param superblock Pointer to the filesystem's superblock.
 * @param group_index The 0-based index of the block group descriptor to write.
 * @param group_desc Pointer to an `ext2_group_desc` structure containing the data to write.
 * @return 0 on success, or a negative error code on failure.
 */
int write_group_descriptor(
    FILE *file,
    const ext2_super_block *superblock,
    uint32_t group_index,
    const ext2_group_desc *group_desc
);

/**
 * @brief Reads all block group descriptors from the filesystem image into an array.
 *
 * This function calculates the number of block groups, allocates memory for the
 * Block Group Descriptor Table (BGDT), and reads all descriptors.
 * The caller is responsible for freeing the allocated memory for the returned array.
 *
 * @param file Pointer to an open FILE stream for the filesystem image.
 * @param superblock Pointer to the filesystem's superblock (already read into memory).
 * @return Pointer to an array of `ext2_group_desc` structures (the BLOCK_GROUP_DESCRIPTOR_TABLE), or NULL on failure.
 *         If NULL is returned, `num_groups_read_out` will be set to 0.
 */
ext2_group_desc_table *read_group_descriptor_table(
    FILE *file,
    const ext2_super_block *superblock
);

#endif // BLOCK_GROUP_H
