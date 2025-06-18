#ifndef BLOCK_GROUP_H
#define BLOCK_GROUP_H

#include <stdint.h>
#include <stdio.h>

#include "Superblock.h"

struct ext2_group_desc {
    uint32_t bg_block_bitmap;         // Block address of block usage bitmap
    uint32_t bg_inode_bitmap;         // Block address of inode usage bitmap
    uint32_t bg_inode_table;          // Starting block address of inode table
    uint16_t bg_free_blocks_count;    // Number of free blocks in this group
    uint16_t bg_free_inodes_count;    // Number of free inodes in this group
    uint16_t bg_used_dirs_count;      // Number of directories in this group
    uint16_t bg_flags;                // Block group flags (EXT2_BG_*)
    uint32_t bg_reserved1;            // Was bg_exclude_bitmap_lo in ext4 struct (4 bytes)
    uint16_t bg_reserved2;            // Was bg_block_bitmap_csum_lo in ext4 struct (2 bytes)
    uint16_t bg_reserved3;            // Was bg_inode_bitmap_csum_lo in ext4 struct (2 bytes)
    uint16_t bg_itable_unused;        // Unused inodes count (for fast fsck)
    uint16_t bg_checksum;             // Group descriptor checksum (if RO_COMPAT_GDT_CSUM)
};

#define EXT2_BG_INODE_UNINIT    0x0001  // Inode table and bitmap are not initialized
#define EXT2_BG_BLOCK_UNINIT    0x0002  // Block bitmap is not initialized
#define EXT2_BG_INODE_ZEROED    0x0004  // Inode table is zeroed

int read_single_group_descriptor(FILE *fp, const struct ext2_super_block *sb, uint32_t group_index, struct ext2_group_desc *group_desc_out);
struct ext2_group_desc* read_all_group_descriptors(FILE *fp, const struct ext2_super_block *sb, uint32_t *num_groups_read_out);

#endif // BLOCK_GROUP_H
