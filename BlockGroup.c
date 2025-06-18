#include "BlockGroup.h"
#include <stdio.h>
#include <stdlib.h> // For malloc, free
#include <string.h> // For memset

// Helper function to calculate block size
static uint32_t get_block_size(const struct ext2_super_block *sb) {
    return 1024 << sb->s_log_block_size;
}

// Helper function to calculate the starting byte offset of the Block Group Descriptor Table (BGDT)
static uint64_t get_table_offset(const struct ext2_super_block *sb) {
    const uint32_t block_size = get_block_size(sb);
    return (uint64_t) block_size * (block_size == 1024 ? 2 : 1);
}

static uint64_t get_descriptor_offset(const struct ext2_super_block *sb, const uint64_t group_index) {
    return get_table_offset(sb) + group_index * sizeof(struct ext2_group_desc);
}

static uint32_t count_block_groups_by_inodes(const struct ext2_super_block *sb) {
    return (sb->s_inodes_count + sb->s_inodes_per_group - 1) / sb->s_inodes_per_group;
}

static uint32_t count_block_groups_by_blocks(const struct ext2_super_block *sb) {
    return (sb->s_blocks_count + sb->s_blocks_per_group - 1) / sb->s_blocks_per_group;
}

// Helper function to calculate the number of block groups in the filesystem
static int get_num_block_groups(const struct ext2_super_block *sb) {
    const uint32_t num_block_groups_by_blocks = count_block_groups_by_blocks(sb);
    const uint32_t num_block_groups_by_inodes = count_block_groups_by_inodes(sb);

    if (num_block_groups_by_blocks != num_block_groups_by_inodes) {
        fprintf(stderr, "Warning: Number of block groups differs based on block count (%u) vs inode count (%u).\n",
                num_block_groups_by_blocks, num_block_groups_by_inodes);
        // For safety, or as per spec, one might be preferred or an error raised.
        // Let's use the one derived from block counts, but this is a point of attention.
    }

    return num_block_groups_by_blocks;
}

// Function to read a single block group descriptor
int read_single_group_descriptor(FILE *fp, const struct ext2_super_block *sb, uint32_t group_index,
                                 struct ext2_group_desc *group_desc_out) {
    if (fp == NULL || sb == NULL || group_desc_out == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to read_single_group_descriptor.\n");
        return -1;
    }

    const uint64_t descriptor_offset = get_descriptor_offset(sb, group_index);

    if (fseeko(fp, descriptor_offset, SEEK_SET) != 0) {
        perror("Error seeking to group descriptor");
        return -2;
    }

    if (fread(group_desc_out, sizeof(struct ext2_group_desc), 1, fp) != 1) {
        if (feof(fp)) {
            fprintf(stderr, "Error reading group descriptor: unexpected end of file for group %u.\n", group_index);
            return -3;
        }

        if (ferror(fp)) {
            perror("Error reading group descriptor");
            return -3;
        }

        return -3;
    }

    return 0;
}

// Function to read all block group descriptors into an array
struct ext2_group_desc *read_all_group_descriptors(FILE *fp, const struct ext2_super_block *sb,
                                                   uint32_t *num_groups_read_out) {
    if (fp == NULL || sb == NULL || num_groups_read_out == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to read_all_group_descriptors.\n");
        if (num_groups_read_out) *num_groups_read_out = 0;
        return NULL;
    }

    const uint32_t num_groups = get_num_block_groups(sb);
    if (num_groups == 0) {
        fprintf(stderr, "Error: Filesystem has 0 block groups according to superblock.\n");
        *num_groups_read_out = 0;
        return NULL;
    }

    *num_groups_read_out = 0;

    struct ext2_group_desc *gdt_table = malloc(num_groups * sizeof(struct ext2_group_desc));
    if (gdt_table == NULL) {
        perror("Error allocating memory for GDT table");
        return NULL;
    }

    memset(gdt_table, 0, num_groups * sizeof(struct ext2_group_desc));

    const uint64_t bgdt_start_offset = get_table_offset(sb);

    if (fseeko(fp, bgdt_start_offset, SEEK_SET) != 0) {
        perror("Error seeking to start of GDT");
        free(gdt_table);
        return NULL;
    }

    const size_t items_read = fread(gdt_table, sizeof(struct ext2_group_desc), num_groups, fp);
    if (items_read != num_groups) {
        if (feof(fp)) {
            fprintf(stderr, "Error reading GDT: unexpected end of file. Expected %u groups, read %zu.\n", num_groups,
                    items_read);
            free(gdt_table);
            return NULL;
        }

        if (ferror(fp)) {
            perror("Error reading GDT");
            free(gdt_table);
            return NULL;
        }

        free(gdt_table);
        return NULL;
    }

    *num_groups_read_out = num_groups;
    return gdt_table;
}
