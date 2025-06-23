/**
 * @file Allocation.c
 * @brief Implements resource allocation functions for the ext2 filesystem.
 */

#include "Allocation.h"
#include "Bitmap.h"
#include "Superblock.h"
#include "BlockGroup.h"
#include "globals.h"

#include <stdio.h>
#include <stdlib.h>

int allocate_inode(
    FILE *file,
    ext2_super_block *superblock,
    const ext2_group_desc_table *block_group_descriptor_table,
    uint32_t *new_inode_num_out
) {
    if (file == NULL || superblock == NULL || block_group_descriptor_table == NULL || new_inode_num_out == NULL) {
        return INVALID_PARAMETER;
    }

    const uint32_t block_size = get_block_size(superblock);
    uint8_t *bitmap_buffer = malloc(block_size);
    if (bitmap_buffer == NULL) {
        return ERROR;
    }

    for (uint32_t group_idx = 0; group_idx < block_group_descriptor_table->groups_count; ++group_idx) {
        if (block_group_descriptor_table->groups[group_idx].bg_free_inodes_count > 0) {
            const uint32_t inode_bitmap_block_id = block_group_descriptor_table->groups[group_idx].bg_inode_bitmap;

            if (read_bitmap(file, superblock, inode_bitmap_block_id, bitmap_buffer) != SUCCESS) {
                log_error("Failed to read inode bitmap for group %u\n", group_idx);
                free(bitmap_buffer);
                return -3;
            }

            uint32_t free_bit_idx = 0;
            if (find_first_free_bit(bitmap_buffer, superblock->s_inodes_per_group, &free_bit_idx) != SUCCESS) {
                log_error("Failed to find a free block.");
                return ERROR;
            }

            set_bit(bitmap_buffer, free_bit_idx);

            if (write_bitmap(file, superblock, inode_bitmap_block_id, bitmap_buffer) != SUCCESS) {
                log_error("Failed to write updated inode bitmap for group %u\n", group_idx);
                free(bitmap_buffer);
                return ERROR;
            }

            block_group_descriptor_table->groups[group_idx].bg_free_inodes_count--;
            superblock->s_free_inodes_count--;

            if (write_group_descriptor(file, superblock, group_idx, &block_group_descriptor_table->groups[group_idx]) != SUCCESS) {
                log_error("Failed to write updated group descriptor for group %u\n", group_idx);
                free(bitmap_buffer);
                return ERROR;
            }
            if (write_superblock(file, superblock) != SUCCESS) {
                log_error("Failed to write updated superblock\n");
                free(bitmap_buffer);
                return ERROR;
            }

            *new_inode_num_out = group_idx * superblock->s_inodes_per_group + free_bit_idx + 1;
            free(bitmap_buffer);
            return SUCCESS;
        }
    }

    free(bitmap_buffer);
    log_error("No free inodes found in any block group.\n");
    return ERROR;
}

int allocate_block(
    FILE *file,
    ext2_super_block *superblock,
    const ext2_group_desc_table *block_group_descriptor_table,
    uint32_t *new_block_num_out
) {
    if (file == NULL || superblock == NULL || block_group_descriptor_table == NULL || new_block_num_out == NULL) {
        return INVALID_PARAMETER;
    }

    const uint32_t block_size = get_block_size(superblock);
    uint8_t *bitmap_buffer = malloc(block_size);
    if (bitmap_buffer == NULL) {
        return ERROR;
    }

    for (uint32_t group_idx = 0; group_idx < block_group_descriptor_table->groups_count; ++group_idx) {
        if (block_group_descriptor_table->groups[group_idx].bg_free_blocks_count > 0) {
            const uint32_t block_bitmap_block_id = block_group_descriptor_table->groups[group_idx].bg_block_bitmap;

            if (read_bitmap(file, superblock, block_bitmap_block_id, bitmap_buffer) != 0) {
                log_error("Failed to read block bitmap for group %u\n", group_idx);
                free(bitmap_buffer);
                return ERROR;
            }

            uint32_t free_bit_idx = 0;
            if (find_first_free_bit(bitmap_buffer, superblock->s_inodes_per_group, &free_bit_idx) != SUCCESS) {
                log_error("Failed to find a free block.");
                return ERROR;
            }

            // Mark bit as used
            set_bit(bitmap_buffer, free_bit_idx);

            // Write bitmap back to disk
            if (write_bitmap(file, superblock, block_bitmap_block_id, bitmap_buffer) != 0) {
                log_error("Failed to write updated block bitmap for group %u\n", group_idx);
                free(bitmap_buffer);
                return ERROR;
            }

            // Update counts
            block_group_descriptor_table->groups[group_idx].bg_free_blocks_count--;
            superblock->s_free_blocks_count--;

            // Write updated group descriptor and superblock back to disk
            if (write_group_descriptor(file, superblock, group_idx, &block_group_descriptor_table->groups[group_idx]) != 0) {
                log_error("Failed to write updated group descriptor for group %u\n", group_idx);
                free(bitmap_buffer);
                return ERROR;
            }
            if (write_superblock(file, superblock) != 0) {
                log_error("Failed to write updated superblock\n");
                free(bitmap_buffer);
                return ERROR;
            }

            // The first data block is at superblock->s_first_data_block (usually 0 or 1)
            *new_block_num_out = group_idx * superblock->s_blocks_per_group + superblock->s_first_data_block + free_bit_idx;
            free(bitmap_buffer);
            return SUCCESS;
        }
    }

    free(bitmap_buffer);
    log_error("No free blocks found in any block group.\n");
    return ERROR;
}
