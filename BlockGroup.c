/**
 * @file BlockGroup.c
 * @brief Implements functions for reading ext2 block group descriptors.
 *
 * This file contains the logic to locate and read both individual block group
 * descriptors and the entire Block Group Descriptor Table (BGDT) from an
 * ext2 filesystem image.
 */
#include "BlockGroup.h"
#include "globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Calculates the number of block groups based on the total number of inodes
 *        and inodes per group, as specified in the superblock.
 * @param superblock Pointer to the superblock structure.
 * @return The calculated number of block groups.
 */
static uint32_t count_block_groups_by_inodes(
    const ext2_super_block *superblock
) {
    return (superblock->s_inodes_count + superblock->s_inodes_per_group - 1) / superblock->s_inodes_per_group;
}

/**
 * @brief Calculates the number of block groups based on the total number of blocks
 *        and blocks per group, as specified in the superblock.
 * @param superblock Pointer to the superblock structure.
 * @return The calculated number of block groups.
 */
static uint32_t count_block_groups_by_blocks(
    const ext2_super_block *superblock
) {
    return (superblock->s_blocks_count + superblock->s_blocks_per_group - 1) / superblock->s_blocks_per_group;
}

off_t get_table_offset(
    const ext2_super_block *superblock
) {
    if (superblock == NULL) {
        return SUCCESS;
    }
    const uint32_t block_size = get_block_size(superblock);
    return (off_t) block_size * (block_size == 1024 ? 2 : 1);
}

off_t get_descriptor_offset(
    const ext2_super_block *superblock,
    const uint64_t group_index
) {
    if (superblock == NULL) {
        return SUCCESS;
    }
    return (off_t) group_index * sizeof(ext2_group_desc) + get_table_offset(superblock);
}

uint32_t get_num_block_groups(
    const ext2_super_block *superblock
) {
    if (superblock == NULL) {
        return SUCCESS;
    }
    const uint32_t num_block_groups_by_blocks = count_block_groups_by_blocks(superblock);
    const uint32_t num_block_groups_by_inodes = count_block_groups_by_inodes(superblock);

    if (num_block_groups_by_blocks != num_block_groups_by_inodes) {
        fprintf(stderr, "Warning: Number of block groups differs based on block count (%u) vs inode count (%u).\n",
                num_block_groups_by_blocks, num_block_groups_by_inodes);
        // For safety, or as per spec, one might be preferred or an error raised.
        // Let's use the one derived from block counts, but this is a point of attention.
    }

    return num_block_groups_by_blocks;
}

int read_single_group_descriptor(
    FILE *file,
    const ext2_super_block *superblock,
    const uint32_t group_index,
    ext2_group_desc *group_desc_out
) {
    if (file == NULL || superblock == NULL || group_desc_out == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to read_single_group_descriptor.\n");
        return INVALID_PARAMETER;
    }

    const off_t descriptor_offset = get_descriptor_offset(superblock, group_index);

    if (fseeko(file, descriptor_offset, SEEK_SET) != 0) {
        perror("Error seeking to group descriptor");
        return -2;
    }

    if (fread(group_desc_out, sizeof(ext2_group_desc), 1, file) != 1) {
        if (feof(file)) {
            fprintf(stderr, "Error reading group descriptor: unexpected end of file for group %u.\n", group_index);
            return -3;
        }

        if (ferror(file)) {
            perror("Error reading group descriptor");
            return -3;
        }

        return -3;
    }

    return SUCCESS;
}

int write_single_group_descriptor(
    FILE *file,
    const ext2_super_block *superblock,
    const uint32_t group_index,
    const ext2_group_desc *group_desc_in
) {
    if (file == NULL || superblock == NULL || group_desc_in == NULL) {
        return INVALID_PARAMETER;
    }

    const off_t offset = get_descriptor_offset(superblock, group_index);

    if (fseeko(file, offset, SEEK_SET) != 0) {
        perror("Error (write_single_group_descriptor): Seeking to group descriptor offset");
        return -2;
    }

    if (fwrite(group_desc_in, sizeof(ext2_group_desc), 1, file) != 1) {
        if (ferror(file)) {
            perror("Error (write_single_group_descriptor): Writing group descriptor");
        } else {
            fprintf(stderr, "Error (write_single_group_descriptor): fwrite did not write the expected number of items.\n");
        }
        return -3;
    }

    return SUCCESS;
}

ext2_group_desc_table *read_all_group_descriptors(
    FILE *file,
    const ext2_super_block *superblock
) {
    if (file == NULL || superblock == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to read_all_group_descriptors.\n");
        return nullptr;
    }

    const uint32_t num_groups = get_num_block_groups(superblock);
    if (num_groups == 0) {
        fprintf(stderr, "Error: Filesystem has 0 block groups according to superblock.\n");
        return nullptr;
    }

    ext2_group_desc *block_group_descriptors = malloc(num_groups * sizeof(ext2_group_desc));
    if (block_group_descriptors == NULL) {
        perror("Error allocating memory for BLOCK_GROUP_DESCRIPTOR_TABLE table");
        return nullptr;
    }

    memset(block_group_descriptors, 0, num_groups * sizeof(ext2_group_desc));

    const off_t bgdt_start_offset = get_table_offset(superblock);

    if (fseeko(file, bgdt_start_offset, SEEK_SET) != 0) {
        perror("Error seeking to start of BLOCK_GROUP_DESCRIPTOR_TABLE");
        free(block_group_descriptors);
        return nullptr;
    }

    const size_t items_read = fread(block_group_descriptors, sizeof(ext2_group_desc), num_groups, file);
    if (items_read != num_groups) {
        if (feof(file)) {
            fprintf(stderr, "Error reading BLOCK_GROUP_DESCRIPTOR_TABLE: unexpected end of file. Expected %u groups, read %zu.\n", num_groups,
                    items_read);
            free(block_group_descriptors);
            return nullptr;
        }

        if (ferror(file)) {
            perror("Error reading BLOCK_GROUP_DESCRIPTOR_TABLE");
            free(block_group_descriptors);
            return nullptr;
        }

        free(block_group_descriptors);
        return nullptr;
    }

    ext2_group_desc_table *table = malloc(sizeof(ext2_group_desc_table));
    if (table == NULL) {
        perror("Error allocating memory for BLOCK_GROUP_DESCRIPTOR_TABLE table");
        free(block_group_descriptors);
        return nullptr;
    }

    table->groups = block_group_descriptors;
    table->groups_count = num_groups;

    return table;
}
