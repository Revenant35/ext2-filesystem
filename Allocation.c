/**
 * @file Allocation.c
 * @brief Implements resource allocation functions for the ext2 filesystem.
 */

#include "Allocation.h"
#include "Bitmap.h"
#include "Superblock.h"
#include "BlockGroup.h"

#include <stdio.h>
#include <stdlib.h>

int allocate_inode(FILE *fp, struct ext2_super_block *sb, struct ext2_group_desc *gdt, uint32_t num_block_groups, uint32_t *new_inode_num_out) {
    if (fp == NULL || sb == NULL || gdt == NULL || new_inode_num_out == NULL) {
        return -1; // Invalid arguments
    }

    const uint32_t block_size = get_block_size(sb);
    char *bitmap_buffer = malloc(block_size);
    if (bitmap_buffer == NULL) {
        return -2; // Memory allocation failure
    }

    for (uint32_t group_idx = 0; group_idx < num_block_groups; ++group_idx) {
        if (gdt[group_idx].bg_free_inodes_count > 0) {
            const uint32_t inode_bitmap_block_id = gdt[group_idx].bg_inode_bitmap;

            if (read_bitmap(fp, sb, inode_bitmap_block_id, bitmap_buffer) != 0) {
                fprintf(stderr, "Failed to read inode bitmap for group %u\n", group_idx);
                free(bitmap_buffer);
                return -3;
            }

            const int free_bit_idx = find_first_free_bit(bitmap_buffer, sb->s_inodes_per_group);
            if (free_bit_idx != -1) {
                // Mark bit as used
                set_bit(bitmap_buffer, free_bit_idx);

                // Write bitmap back to disk
                if (write_bitmap(fp, sb, inode_bitmap_block_id, bitmap_buffer) != 0) {
                    fprintf(stderr, "Failed to write updated inode bitmap for group %u\n", group_idx);
                    free(bitmap_buffer);
                    return -4;
                }

                // Update counts
                gdt[group_idx].bg_free_inodes_count--;
                sb->s_free_inodes_count--;

                // Write updated group descriptor and superblock back to disk
                if (write_single_group_descriptor(fp, sb, group_idx, &gdt[group_idx]) != 0) {
                    fprintf(stderr, "Failed to write updated group descriptor for group %u\n", group_idx);
                    // Attempt to revert changes? For now, just report error.
                    free(bitmap_buffer);
                    return -5;
                }
                if (write_superblock(fp, sb) != 0) {
                    fprintf(stderr, "Failed to write updated superblock\n");
                    free(bitmap_buffer);
                    return -6;
                }

                *new_inode_num_out = group_idx * sb->s_inodes_per_group + free_bit_idx + 1;
                free(bitmap_buffer);
                return 0; // Success
            }
        }
    }

    free(bitmap_buffer);
    fprintf(stderr, "No free inodes found in any block group.\n");
    return -7; // No free inodes found
}

int allocate_block(FILE *fp, struct ext2_super_block *sb, struct ext2_group_desc *gdt, uint32_t num_block_groups, uint32_t *new_block_num_out) {
    if (fp == NULL || sb == NULL || gdt == NULL || new_block_num_out == NULL) {
        return -1; // Invalid arguments
    }

    const uint32_t block_size = get_block_size(sb);
    char *bitmap_buffer = malloc(block_size);
    if (bitmap_buffer == NULL) {
        return -2; // Memory allocation failure
    }

    for (uint32_t group_idx = 0; group_idx < num_block_groups; ++group_idx) {
        if (gdt[group_idx].bg_free_blocks_count > 0) {
            const uint32_t block_bitmap_block_id = gdt[group_idx].bg_block_bitmap;

            if (read_bitmap(fp, sb, block_bitmap_block_id, bitmap_buffer) != 0) {
                fprintf(stderr, "Failed to read block bitmap for group %u\n", group_idx);
                free(bitmap_buffer);
                return -3;
            }

            const int free_bit_idx = find_first_free_bit(bitmap_buffer, sb->s_blocks_per_group);
            if (free_bit_idx != -1) {
                // Mark bit as used
                set_bit(bitmap_buffer, free_bit_idx);

                // Write bitmap back to disk
                if (write_bitmap(fp, sb, block_bitmap_block_id, bitmap_buffer) != 0) {
                    fprintf(stderr, "Failed to write updated block bitmap for group %u\n", group_idx);
                    free(bitmap_buffer);
                    return -4;
                }

                // Update counts
                gdt[group_idx].bg_free_blocks_count--;
                sb->s_free_blocks_count--;

                // Write updated group descriptor and superblock back to disk
                if (write_single_group_descriptor(fp, sb, group_idx, &gdt[group_idx]) != 0) {
                    fprintf(stderr, "Failed to write updated group descriptor for group %u\n", group_idx);
                    free(bitmap_buffer);
                    return -5;
                }
                if (write_superblock(fp, sb) != 0) {
                    fprintf(stderr, "Failed to write updated superblock\n");
                    free(bitmap_buffer);
                    return -6;
                }

                // The first data block is at sb->s_first_data_block (usually 0 or 1)
                *new_block_num_out = group_idx * sb->s_blocks_per_group + sb->s_first_data_block + free_bit_idx;
                free(bitmap_buffer);
                return 0; // Success
            }
        }
    }

    free(bitmap_buffer);
    fprintf(stderr, "No free blocks found in any block group.\n");
    return -7; // No free blocks found
}
