/**
 * @file inode.c
 * @brief Implements functions for reading and writing ext2 inodes.
 *
 * These functions handle the I/O operations to load an inode from a
 * filesystem image into memory and to write a modified inode back to the image.
 */

#include "inode.h"
#include "superblock.h"
#include "block_group.h"
#include "globals.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Calculates the disk offset of a given inode.
 *
 * @param superblock Pointer to the filesystem's superblock.
 * @param block_group_descriptor_table Pointer to the array of block group descriptors (the BLOCK_GROUP_DESCRIPTOR_TABLE).
 * @param inode_num The number of the inode to locate (1-based).
 * @param offset_out Pointer to an off_t variable where the calculated disk offset will be stored.
 * @return 0 on success, or a negative error code on failure.
 */
static int calculate_inode_disk_offset(
    const ext2_super_block *superblock,
    const ext2_group_desc *block_group_descriptor_table,
    const uint32_t inode_num,
    off_t *offset_out
) {
    if (inode_num == 0 || inode_num > superblock->s_inodes_count) {
        log_error("Error (calc_offset): Inode number %u must be within range [0, %u].\n", inode_num, superblock->s_inodes_count);
        return INVALID_PARAMETER;
    }

    // Inode numbers are 1-based. Adjust to 0-based for calculations.
    const uint32_t inode_index = inode_num - 1;

    const uint32_t block_group_num = inode_index / superblock->s_inodes_per_group;
    const uint32_t inode_index_in_group = inode_index % superblock->s_inodes_per_group;

    // Check if block_group_num is valid (though s_inodes_count check should cover this)
    const uint32_t num_block_groups = (superblock->s_blocks_count + superblock->s_blocks_per_group - 1) / superblock->s_blocks_per_group;
    if (block_group_num >= num_block_groups) { // This check might be redundant if BLOCK_GROUP_DESCRIPTOR_TABLE is correctly sized
        log_error("Error (calc_offset): Calculated block group %u is out of bounds.\n", block_group_num);
        return ERROR;
    }

    const ext2_group_desc *target_group = &block_group_descriptor_table[block_group_num];
    const uint32_t inode_table_start_block_id = target_group->bg_inode_table;

    const uint32_t block_size = get_block_size(superblock);
    const off_t inode_table_start_byte_offset = (off_t)inode_table_start_block_id * block_size;
    const off_t inode_offset_in_table_bytes = (off_t)inode_index_in_group * superblock->s_inode_size;

    *offset_out = inode_table_start_byte_offset + inode_offset_in_table_bytes;
    return SUCCESS;
}

/**
 * @brief Reads an inode from the filesystem into memory.
 *
 * @param file Pointer to an open FILE stream for the filesystem image.
 * @param superblock Pointer to the filesystem's superblock.
 * @param block_group_descriptor_table Pointer to the array of block group descriptors (the BLOCK_GROUP_DESCRIPTOR_TABLE).
 * @param inode_num The number of the inode to read (1-based).
 * @param inode_out Pointer to an `ext2_inode` structure to populate with the read data.
 * @return 0 on success, or a negative error code on failure.
 */
int read_inode(
    FILE *file,
    const ext2_super_block *superblock,
    const ext2_group_desc *block_group_descriptor_table,
    const uint32_t inode_num,
    ext2_inode *inode_out
) {
    if (file == NULL || superblock == NULL || block_group_descriptor_table == NULL || inode_out == NULL) {
        log_error("Error (read_inode): NULL pointer argument provided.\n");
        return INVALID_PARAMETER;
    }

    off_t inode_disk_offset;
    const int calc_status = calculate_inode_disk_offset(superblock, block_group_descriptor_table, inode_num, &inode_disk_offset);
    if (calc_status != 0) {
        log_error("Error (read_inode): Failed to calculate location for inode %u (status: %d).\n", inode_num, calc_status);
        return ERROR;
    }

    if (fseeko(file, inode_disk_offset, SEEK_SET) != 0) {
        log_error("Error (read_inode): Seeking to inode location");
        return ERROR;
    }

    if (fread(inode_out, sizeof(ext2_inode), 1, file) != 1) {
        if (feof(file)) {
            log_error("Error (read_inode): Reading inode %u: unexpected end of file.\n", inode_num);
        } else if (ferror(file)) {
            log_error("Error (read_inode): Reading inode");
        }
        return ERROR;
    }

    return SUCCESS;
}

/**
 * @brief Writes an inode from memory to the filesystem.
 *
 * @param file Pointer to an open FILE stream for the filesystem image.
 * @param superblock Pointer to the filesystem's superblock.
 * @param block_group_descriptor_table Pointer to the array of block group descriptors (the BLOCK_GROUP_DESCRIPTOR_TABLE).
 * @param inode_num The number of the inode to write (1-based).
 * @param inode_in Pointer to an `ext2_inode` structure containing the data to write.
 * @return 0 on success, or a negative error code on failure.
 */
int write_inode(
    FILE *file,
    const ext2_super_block *superblock,
    const ext2_group_desc *block_group_descriptor_table,
    const uint32_t inode_num,
    const ext2_inode *inode_in
) {
    if (file == NULL || superblock == NULL || block_group_descriptor_table == NULL || inode_in == NULL) {
        log_error("Error (write_inode): NULL pointer argument provided.\n");
        return INVALID_PARAMETER;
    }

    off_t inode_disk_offset;
    const int calc_status = calculate_inode_disk_offset(superblock, block_group_descriptor_table, inode_num, &inode_disk_offset);
    if (calc_status != SUCCESS) {
        log_error("Error (write_inode): Failed to calculate location for inode %u (status: %d).\n", inode_num, calc_status);
        return ERROR;
    }

    if (fseeko(file, inode_disk_offset, SEEK_SET) != 0) {
        log_error("Error (write_inode): Seeking to inode location");
        return ERROR;
    }

    if (fwrite(inode_in, sizeof(ext2_inode), 1, file) != 1) {
        if (ferror(file)) {
            log_error("Error (write_inode): Writing inode");
        } else {
            log_error("Error (write_inode): Writing inode %u: fwrite did not write the expected number of items.\n", inode_num);
        }
        return ERROR;
    }

    return SUCCESS;
}
