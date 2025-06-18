/**
 * @file Inode.c
 * @brief Implements functions for reading and writing ext2 inodes.
 *
 * These functions handle the I/O operations to load an inode from a
 * filesystem image into memory and to write a modified inode back to the image.
 */

#include "Inode.h"
#include "Superblock.h"
#include "BlockGroup.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Calculates the disk offset of a given inode.
 *
 * @param sb Pointer to the filesystem's superblock.
 * @param gdt Pointer to the array of block group descriptors (the GDT).
 * @param inode_num The number of the inode to locate (1-based).
 * @param inode_size The size of a single inode structure on disk (from sb->s_inode_size).
 * @param offset_out Pointer to an off_t variable where the calculated disk offset will be stored.
 * @return 0 on success, or a negative error code on failure.
 */
static int calculate_inode_disk_offset(
    const struct ext2_super_block *sb,
    const struct ext2_group_desc *gdt,
    const uint32_t inode_num,
    const uint16_t inode_size,
    off_t *offset_out
) {
    if (inode_num == 0) {
        fprintf(stderr, "Error (calc_offset): Inode number 0 is invalid.\n");
        return -1; // Invalid inode number
    }
    if (inode_num > sb->s_inodes_count) {
        fprintf(stderr, "Error (calc_offset): Inode number %u exceeds total inodes %u.\n", inode_num, sb->s_inodes_count);
        return -2; // Inode number out of bounds
    }
    if (inode_size == 0) {
        fprintf(stderr, "Error (calc_offset): Superblock indicates inode_size is 0.\n");
        return -3; // Invalid inode size from superblock
    }

    // Inode numbers are 1-based. Adjust to 0-based for calculations.
    const uint32_t inode_index = inode_num - 1;

    const uint32_t block_group_num = inode_index / sb->s_inodes_per_group;
    const uint32_t inode_index_in_group = inode_index % sb->s_inodes_per_group;

    // Check if block_group_num is valid (though s_inodes_count check should cover this)
    const uint32_t num_block_groups = (sb->s_blocks_count + sb->s_blocks_per_group - 1) / sb->s_blocks_per_group;
    if (block_group_num >= num_block_groups) { // This check might be redundant if GDT is correctly sized
        fprintf(stderr, "Error (calc_offset): Calculated block group %u is out of bounds.\n", block_group_num);
        return -4;
    }

    const struct ext2_group_desc *target_group = &gdt[block_group_num];
    const uint32_t inode_table_start_block_id = target_group->bg_inode_table;

    const uint32_t block_size = get_block_size(sb);
    const off_t inode_table_start_byte_offset = (off_t)inode_table_start_block_id * block_size;
    const off_t inode_offset_in_table_bytes = (off_t)inode_index_in_group * inode_size;

    *offset_out = inode_table_start_byte_offset + inode_offset_in_table_bytes;
    return 0; // Success
}

/**
 * @brief Reads an inode from the filesystem into memory.
 *
 * @param fp Pointer to an open FILE stream for the filesystem image.
 * @param sb Pointer to the filesystem's superblock.
 * @param gdt Pointer to the array of block group descriptors (the GDT).
 * @param inode_num The number of the inode to read (1-based).
 * @param inode_out Pointer to an `ext2_inode` structure to populate with the read data.
 * @return 0 on success, or a negative error code on failure.
 */
int read_inode(FILE *fp, const struct ext2_super_block *sb, const struct ext2_group_desc *gdt, uint32_t inode_num, struct ext2_inode *inode_out) {
    if (fp == NULL || sb == NULL || gdt == NULL || inode_out == NULL) {
        fprintf(stderr, "Error (read_inode): NULL pointer argument provided.\n");
        return -1;
    }

    // inode_num validity (0 or > s_inodes_count) is checked by calculate_inode_disk_offset

    // The size of the inode structure on disk is given by sb->s_inode_size.
    // The ext2_inode struct in Inode.h is the base 128-byte structure.
    // If sb->s_inode_size is larger, this function will only read sizeof(struct ext2_inode) bytes.
    // This is a simplification; a full implementation for variable inode sizes might require
    // reading sb->s_inode_size bytes into a buffer and then accessing the known part.
    // For ext2 rev 1+, s_inode_size from superblock is authoritative.
    const uint16_t on_disk_inode_size = sb->s_inode_size;
    if (on_disk_inode_size < sizeof(struct ext2_inode) && on_disk_inode_size != 0) {
        // This case is problematic, as we might read beyond the actual inode if on_disk_inode_size is too small
        // but not zero. However, standard ext2 usually has 128 or more.
        fprintf(stderr, "Warning (read_inode): sb->s_inode_size (%u) is less than sizeof(struct ext2_inode) (%zu).\n", 
                on_disk_inode_size, sizeof(struct ext2_inode));
        // Proceeding to read sizeof(struct ext2_inode) might be incorrect here.
        // For safety, one might choose to read only on_disk_inode_size, but then inode_out would be partially filled.
        // This situation indicates a potential mismatch or a very unusual ext2 configuration.
    }

    off_t inode_disk_offset;
    const int calc_status = calculate_inode_disk_offset(sb, gdt, inode_num, on_disk_inode_size, &inode_disk_offset);
    if (calc_status != 0) {
        fprintf(stderr, "Error (read_inode): Failed to calculate location for inode %u (status: %d).\n", inode_num, calc_status);
        return -5; // Error in calculation, specific error code from calc_status might be more informative
    }

    if (fseeko(fp, inode_disk_offset, SEEK_SET) != 0) {
        perror("Error (read_inode): Seeking to inode location");
        return -6;
    }

    // We read sizeof(struct ext2_inode) bytes, assuming this is the desired portion
    // or that on_disk_inode_size >= sizeof(struct ext2_inode).
    if (fread(inode_out, sizeof(struct ext2_inode), 1, fp) != 1) {
        if (feof(fp)) {
            fprintf(stderr, "Error (read_inode): Reading inode %u: unexpected end of file.\n", inode_num);
        } else if (ferror(fp)) {
            perror("Error (read_inode): Reading inode");
        }
        return -7;
    }

    return 0;
}

/**
 * @brief Writes an inode from memory to the filesystem.
 *
 * @param fp Pointer to an open FILE stream for the filesystem image.
 * @param sb Pointer to the filesystem's superblock.
 * @param gdt Pointer to the array of block group descriptors (the GDT).
 * @param inode_num The number of the inode to write (1-based).
 * @param inode_in Pointer to an `ext2_inode` structure containing the data to write.
 * @return 0 on success, or a negative error code on failure.
 */
int write_inode(FILE *fp, const struct ext2_super_block *sb, const struct ext2_group_desc *gdt, uint32_t inode_num, const struct ext2_inode *inode_in) {
    if (fp == NULL || sb == NULL || gdt == NULL || inode_in == NULL) {
        fprintf(stderr, "Error (write_inode): NULL pointer argument provided.\n");
        return -1;
    }

    // Similar to read_inode, using sb->s_inode_size for location calculation
    // and sizeof(struct ext2_inode) for the actual write operation.
    const uint16_t on_disk_inode_size = sb->s_inode_size;
    // Add similar warning for on_disk_inode_size < sizeof(struct ext2_inode) if desired.

    off_t inode_disk_offset;
    const int calc_status = calculate_inode_disk_offset(sb, gdt, inode_num, on_disk_inode_size, &inode_disk_offset);
    if (calc_status != 0) {
        fprintf(stderr, "Error (write_inode): Failed to calculate location for inode %u (status: %d).\n", inode_num, calc_status);
        return -5;
    }

    if (fseeko(fp, inode_disk_offset, SEEK_SET) != 0) {
        perror("Error (write_inode): Seeking to inode location");
        return -6;
    }

    if (fwrite(inode_in, sizeof(struct ext2_inode), 1, fp) != 1) {
        if (ferror(fp)) {
            perror("Error (write_inode): Writing inode");
        } else {
            fprintf(stderr, "Error (write_inode): Writing inode %u: fwrite did not write the expected number of items.\n", inode_num);
        }
        return -7;
    }
    
    // fflush(fp); // Optional: ensure data is written to disk immediately.

    return 0; // Success
}
