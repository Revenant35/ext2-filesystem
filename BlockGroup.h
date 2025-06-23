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

#include "Superblock.h"

#include <stdint.h>
#include <stdio.h>

/**
 * @brief The ext2 block group descriptor structure (typically 32 bytes).
 *
 * This structure holds metadata for a single block group within the filesystem.
 * An array of these descriptors, known as the Block Group Descriptor Table (BGDT),
 * follows the superblock.
 */
typedef struct {
    uint32_t bg_block_bitmap;         //!< Block ID of the block usage bitmap for this group.
    uint32_t bg_inode_bitmap;         //!< Block ID of the inode usage bitmap for this group.
    uint32_t bg_inode_table;          //!< Block ID of the starting block of the inode table for this group.
    uint16_t bg_free_blocks_count;    //!< Number of free blocks in this group.
    uint16_t bg_free_inodes_count;    //!< Number of free inodes in this group.
    uint16_t bg_used_dirs_count;      //!< Number of directories in this group.
    uint16_t bg_flags;                //!< Block group flags (see EXT2_BG_* defines).
    uint32_t bg_reserved1;            //!< Reserved for future use. (Was bg_exclude_bitmap_lo in ext4 for snapshot feature)
    uint16_t bg_reserved2;            //!< Reserved for future use. (Was bg_block_bitmap_csum_lo in ext4 for checksumming)
    uint16_t bg_reserved3;            //!< Reserved for future use. (Was bg_inode_bitmap_csum_lo in ext4 for checksumming)
    uint16_t bg_itable_unused;        //!< Number of unused inodes in this group (if INODE_ZEROED flag is set).
    uint16_t bg_checksum;             //!< Group descriptor checksum (if EXT2_FEATURE_RO_COMPAT_GDT_CSUM is set in superblock).
} ext2_group_desc;

typedef struct {
    ext2_group_desc *groups;
    uint32_t groups_count;
} ext2_group_desc_table;

#define EXT2_BG_INODE_UNINIT    0x0001  // Inode table and bitmap are not initialized
#define EXT2_BG_BLOCK_UNINIT    0x0002  // Block bitmap is not initialized
#define EXT2_BG_INODE_ZEROED    0x0004  // Inode table is zeroed


/**
 * @brief Calculates the starting byte offset of the Block Group Descriptor Table (BGDT).
 *
 * The BGDT starts on the block immediately following the superblock.
 * If the block size is 1024 bytes, the superblock occupies block 1, so BGDT starts at block 2.
 * If the block size is > 1024 bytes, the superblock is in block 0, so BGDT starts at block 1.
 *
 * @param superblock Pointer to the superblock structure.
 * @return The starting byte offset of the BGDT from the beginning of the filesystem.
 */
off_t get_table_offset(
    const ext2_super_block *superblock
);

/**
 * @brief Calculates the byte offset of a specific block group descriptor within the BGDT.
 * @param superblock Pointer to the superblock structure.
 * @param group_index The 0-based index of the target block group.
 * @return The byte offset of the specified group descriptor from the beginning of the filesystem.
 */
off_t get_descriptor_offset(
    const ext2_super_block *superblock,
    uint64_t group_index
);

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
uint32_t get_num_block_groups(
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
