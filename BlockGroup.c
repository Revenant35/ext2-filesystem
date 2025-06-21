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
    const struct ext2_super_block *superblock
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
    const struct ext2_super_block *superblock
) {
    return (superblock->s_blocks_count + superblock->s_blocks_per_group - 1) / superblock->s_blocks_per_group;
}

uint32_t get_block_size(
    const struct ext2_super_block *superblock
) {
    if (superblock == NULL) {
        return 0;
    }
    return 1024 << superblock->s_log_block_size;
}

off_t get_table_offset(
    const struct ext2_super_block *superblock
) {
    if (superblock == NULL) {
        return 0;
    }
    const uint32_t block_size = get_block_size(superblock);
    return (off_t) block_size * (block_size == 1024 ? 2 : 1);
}

off_t get_descriptor_offset(
    const struct ext2_super_block *superblock,
    const uint64_t group_index
) {
    if (superblock == NULL) {
        return 0;
    }
    return (off_t) group_index * sizeof(struct ext2_group_desc) + get_table_offset(superblock);
}

uint32_t get_num_block_groups(
    const struct ext2_super_block *superblock
) {
    if (superblock == NULL) {
        return 0;
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
    const struct ext2_super_block *superblock,
    const uint32_t group_index,
    struct ext2_group_desc *group_desc_out
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

    if (fread(group_desc_out, sizeof(struct ext2_group_desc), 1, file) != 1) {
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

    return 0;
}

int write_single_group_descriptor(
    FILE *file,
    const struct ext2_super_block *superblock,
    const uint32_t group_index,
    const struct ext2_group_desc *group_desc_in
) {
    if (file == NULL || superblock == NULL || group_desc_in == NULL) {
        return INVALID_PARAMETER;
    }

    const off_t offset = get_descriptor_offset(superblock, group_index);

    if (fseeko(file, offset, SEEK_SET) != 0) {
        perror("Error (write_single_group_descriptor): Seeking to group descriptor offset");
        return -2;
    }

    if (fwrite(group_desc_in, sizeof(struct ext2_group_desc), 1, file) != 1) {
        if (ferror(file)) {
            perror("Error (write_single_group_descriptor): Writing group descriptor");
        } else {
            fprintf(stderr, "Error (write_single_group_descriptor): fwrite did not write the expected number of items.\n");
        }
        return -3;
    }

    return 0;
}

struct ext2_group_desc *read_all_group_descriptors(
    FILE *file,
    const struct ext2_super_block *superblock,
    uint32_t *num_groups_read_out
) {
    if (file == NULL || superblock == NULL || num_groups_read_out == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to read_all_group_descriptors.\n");
        if (num_groups_read_out) *num_groups_read_out = 0;
        return nullptr;
    }

    const uint32_t num_groups = get_num_block_groups(superblock);
    if (num_groups == 0) {
        fprintf(stderr, "Error: Filesystem has 0 block groups according to superblock.\n");
        *num_groups_read_out = 0;
        return nullptr;
    }

    *num_groups_read_out = 0;

    struct ext2_group_desc *gdt_table = malloc(num_groups * sizeof(struct ext2_group_desc));
    if (gdt_table == NULL) {
        perror("Error allocating memory for BLOCK_GROUP_DESCRIPTOR_TABLE table");
        return nullptr;
    }

    memset(gdt_table, 0, num_groups * sizeof(struct ext2_group_desc));

    const off_t bgdt_start_offset = get_table_offset(superblock);

    if (fseeko(file, bgdt_start_offset, SEEK_SET) != 0) {
        perror("Error seeking to start of BLOCK_GROUP_DESCRIPTOR_TABLE");
        free(gdt_table);
        return nullptr;
    }

    const size_t items_read = fread(gdt_table, sizeof(struct ext2_group_desc), num_groups, file);
    if (items_read != num_groups) {
        if (feof(file)) {
            fprintf(stderr, "Error reading BLOCK_GROUP_DESCRIPTOR_TABLE: unexpected end of file. Expected %u groups, read %zu.\n", num_groups,
                    items_read);
            free(gdt_table);
            return nullptr;
        }

        if (ferror(file)) {
            perror("Error reading BLOCK_GROUP_DESCRIPTOR_TABLE");
            free(gdt_table);
            return nullptr;
        }

        free(gdt_table);
        return nullptr;
    }

    *num_groups_read_out = num_groups;
    return gdt_table;
}
