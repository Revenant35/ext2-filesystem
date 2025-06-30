/**
 * @file Superblock.h
 * @brief Defines the structure of the ext2 superblock and related constants.
 *
 * The superblock is a critical data structure in an ext2 filesystem. It contains
 * metadata about the filesystem as a whole, such as the total number of inodes and
 * blocks, block size, filesystem state, and feature flags.
 */
#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <stdint.h>
#include <stdio.h>

#include "types.h"

/**
 * @brief Reads the superblock from an open file stream into memory.
 * @param file Pointer to an open FILE stream (e.g., from an ext2 image file).
 * @return Pointer to an ext2_super_block structure containing the superblock data,
 */
ext2_super_block *read_superblock(
    FILE *file
);

/**
 * @brief Writes the superblock from memory to an open file stream.
 * @param file Pointer to an open FILE stream (e.g., for an ext2 image file).
 * @param superblock Pointer to an ext2_super_block structure containing the data to write.
 * @return 0 on success, non-zero on failure (e.g., I/O error, invalid superblock data).
 */
int write_superblock(
    FILE *file,
    const ext2_super_block *superblock
);

/**
 * @brief Calculates the block size in bytes from the superblock's log field.
 * @param superblock Pointer to a populated ext2_super_block structure.
 * @return The block size in bytes (e.g., 1024, 2048, 4096).
 */
uint32_t get_block_size(const ext2_super_block *superblock);


/**
 * @brief Calculates the fragment size in bytes from the superblock's log field.
 * @param superblock Pointer to a populated ext2_super_block structure.
 * @return The fragment size in bytes.
 */
uint32_t get_fragment_size(const ext2_super_block *superblock);

/**
 * @brief Calculates the total number of block groups in the filesystem.
 * @param superblock Pointer to a populated ext2_super_block structure.
 * @return The total number of block groups.
 */
uint32_t get_block_group_count(const ext2_super_block *superblock);

#endif //SUPERBLOCK_H
