/**
 * @file Directory.c
 * @brief Implements functions for reading and listing ext2 directory entries.
 */

#include "Directory.h"
#include "Inode.h"
#include "Superblock.h"
#include "BlockGroup.h"

#include <stdio.h>
#include <stdlib.h> // For malloc, free
#include <string.h> // For strncpy, memset

// Define for the number of direct blocks in an inode, typically 12
#define EXT2_NDIR_BLOCKS 12

/**
 * @brief Reads and lists the entries of a directory.
 *
 * This function reads the data blocks of the specified directory inode and prints
 * the details of each directory entry found within those blocks.
 * Currently, it only processes direct blocks (i_block[0] to i_block[11]).
 *
 * @param fp Pointer to an open FILE stream for the filesystem image.
 * @param sb Pointer to the filesystem's superblock.
 * @param gdt Pointer to the array of block group descriptors (the GDT).
 * @param dir_inode_num The inode number of the directory to list.
 * @return 0 on success, or a negative error code on failure:
 *         -1: NULL pointer argument.
 *         -2: Failed to read directory inode.
 *         -3: Specified inode is not a directory.
 *         -4: Memory allocation failure for block buffer.
 *         -5: Failed to seek to data block.
 *         -6: Failed to read data block.
 */
int list_directory_entries(FILE *fp, const struct ext2_super_block *sb, const struct ext2_group_desc *gdt, uint32_t dir_inode_num) {
    if (fp == NULL || sb == NULL || gdt == NULL) {
        fprintf(stderr, "Error (list_directory): NULL pointer argument provided.\n");
        return -1;
    }

    struct ext2_inode dir_inode;
    if (read_inode(fp, sb, gdt, dir_inode_num, &dir_inode) != 0) {
        fprintf(stderr, "Error (list_directory): Failed to read inode %u.\n", dir_inode_num);
        return -2;
    }

    // Check if it's a directory (S_IFDIR is 0x4000)
    if (!((dir_inode.i_mode & 0xF000) == 0x4000)) { // S_IFMT is 0xF000, S_IFDIR is 0x4000
        fprintf(stderr, "Error (list_directory): Inode %u is not a directory (mode: %04X).\n", dir_inode_num, dir_inode.i_mode);
        return -3;
    }

    uint32_t block_size = get_block_size(sb);
    char *block_buffer = (char *)malloc(block_size);
    if (block_buffer == NULL) {
        fprintf(stderr, "Error (list_directory): Failed to allocate memory for block buffer.\n");
        return -4;
    }

    printf("Directory listing for inode %u:\n", dir_inode_num);
    printf("Inode | Rec Len | Name Len | Type | Name\n");
    printf("----------------------------------------------------\n");

    // Iterate over direct blocks only for now
    for (int i = 0; i < EXT2_NDIR_BLOCKS; ++i) {
        if (dir_inode.i_block[i] == 0) {
            // Block pointer is 0, means no more data blocks for this directory in direct pointers
            // or this specific direct block is not used.
            continue;
        }

        uint32_t data_block_id = dir_inode.i_block[i];
        off_t block_offset = (off_t)data_block_id * block_size;

        if (fseeko(fp, block_offset, SEEK_SET) != 0) {
            perror("Error (list_directory): Seeking to data block");
            free(block_buffer);
            return -5;
        }

        if (fread(block_buffer, block_size, 1, fp) != 1) {
            if (feof(fp)) {
                fprintf(stderr, "Error (list_directory): Reading data block %u: unexpected end of file.\n", data_block_id);
            } else if (ferror(fp)) {
                perror("Error (list_directory): Reading data block");
            }
            free(block_buffer);
            return -6;
        }

        uint32_t current_offset_in_block = 0;
        while (current_offset_in_block < block_size) {
            struct ext2_dir_entry_2 *entry = (struct ext2_dir_entry_2 *)(block_buffer + current_offset_in_block);

            if (entry->inode == 0 && entry->rec_len == 0) { // Should not happen if rec_len is always valid
                 // This might indicate end of entries if not properly padded, or corruption.
                 // Standard says rec_len should span to end of block or next entry.
                 break;
            }
            
            if (entry->rec_len == 0) { // Invalid entry, stop processing this block
                fprintf(stderr, "Warning (list_directory): Encountered directory entry with rec_len=0 in block %u. Stopping parse of this block.\n", data_block_id);
                break;
            }

            if (entry->inode != 0) { // Only print valid entries
                char name_buffer[EXT2_NAME_LEN + 1];
                strncpy(name_buffer, entry->name, entry->name_len);
                name_buffer[entry->name_len] = '\0'; // Ensure null termination

                printf("%-5u | %-7u | %-8u | %-4u | %s\n",
                       entry->inode,
                       entry->rec_len,
                       entry->name_len,
                       entry->file_type,
                       name_buffer);
            }

            current_offset_in_block += entry->rec_len;
        }
    }

    free(block_buffer);
    return 0; // Success
}
