/**
 * @file Directory.c
 * @brief Implements functions for reading and listing ext2 directory entries.
 */

#include "Directory.h"
#include "Inode.h"
#include "BlockGroup.h"
#include "Allocation.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

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
int list_directory_entries(FILE *fp, const struct ext2_super_block *sb, const struct ext2_group_desc *gdt,
                           uint32_t dir_inode_num) {
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
    if (!((dir_inode.i_mode & 0xF000) == 0x4000)) {
        // S_IFMT is 0xF000, S_IFDIR is 0x4000
        fprintf(stderr, "Error (list_directory): Inode %u is not a directory (mode: %04X).\n", dir_inode_num,
                dir_inode.i_mode);
        return -3;
    }

    const uint32_t block_size = get_block_size(sb);
    const auto block_buffer = (char *) malloc(block_size);
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

        const uint32_t data_block_id = dir_inode.i_block[i];
        const off_t block_offset = (off_t) data_block_id * block_size;

        if (fseeko(fp, block_offset, SEEK_SET) != 0) {
            perror("Error (list_directory): Seeking to data block");
            free(block_buffer);
            return -5;
        }

        if (fread(block_buffer, block_size, 1, fp) != 1) {
            if (feof(fp)) {
                fprintf(stderr, "Error (list_directory): Reading data block %u: unexpected end of file.\n",
                        data_block_id);
            } else if (ferror(fp)) {
                perror("Error (list_directory): Reading data block");
            }
            free(block_buffer);
            return -6;
        }

        uint32_t current_offset_in_block = 0;
        while (current_offset_in_block < block_size) {
            const auto entry = (struct ext2_dir_entry_2 *) (block_buffer + current_offset_in_block);

            if (entry->inode == 0 && entry->rec_len == 0) {
                // Should not happen if rec_len is always valid
                // This might indicate end of entries if not properly padded, or corruption.
                // Standard says rec_len should span to end of block or next entry.
                break;
            }

            if (entry->rec_len == 0) {
                // Invalid entry, stop processing this block
                fprintf(
                    stderr,
                    "Warning (list_directory): Encountered directory entry with rec_len=0 in block %u. Stopping parse of this block.\n",
                    data_block_id);
                break;
            }

            if (entry->inode != 0) {
                // Only print valid entries
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

int add_directory_entry(FILE *fp, struct ext2_super_block *sb, struct ext2_group_desc *gdt, uint32_t num_block_groups,
                        struct ext2_inode *parent_inode, uint32_t parent_inode_num, const uint32_t new_entry_inode_num,
                        const char *new_entry_name, const uint8_t new_entry_type) {
    const uint32_t block_size = get_block_size(sb);
    const uint8_t name_len = strlen(new_entry_name);
    const uint16_t new_entry_len = EXT2_DIR_REC_LEN(name_len);

    const auto block_buffer = (char *) malloc(block_size);
    if (!block_buffer) {
        return -1; // Malloc failed
    }

    // Iterate through direct blocks only for now
    for (int i = 0; i < 12; ++i) {
        if (parent_inode->i_block[i] == 0) {
            continue; // Skip unused blocks
        }

        const off_t block_offset = parent_inode->i_block[i] * block_size;
        fseeko(fp, block_offset, SEEK_SET);
        fread(block_buffer, block_size, 1, fp);

        char *current_pos = block_buffer;
        auto entry = (struct ext2_dir_entry_2 *) current_pos;

        while (current_pos - block_buffer < block_size) {
            // Actual space used by the current entry
            const uint16_t current_entry_actual_len = EXT2_DIR_REC_LEN(entry->name_len);

            // Check if this is the last entry in the block and if there's enough space
            if (current_pos + current_entry_actual_len - block_buffer < block_size &&
                entry->rec_len >= current_entry_actual_len + new_entry_len) {
                // Resize current entry
                const uint16_t old_rec_len = entry->rec_len;
                entry->rec_len = current_entry_actual_len;

                // Create the new entry in the remaining space
                current_pos += entry->rec_len;
                const auto new_entry = (struct ext2_dir_entry_2 *) current_pos;
                new_entry->inode = new_entry_inode_num;
                new_entry->rec_len = old_rec_len - entry->rec_len;
                new_entry->name_len = name_len;
                new_entry->file_type = new_entry_type;
                strncpy(new_entry->name, new_entry_name, name_len);

                // Write the modified block back to disk
                fseeko(fp, block_offset, SEEK_SET);
                fwrite(block_buffer, block_size, 1, fp);

                free(block_buffer);
                return 0; // Success
            }

            // Move to the next entry
            current_pos += entry->rec_len;
            entry = (struct ext2_dir_entry_2 *) current_pos;
        }
    }

    // If we are here, no space was found in existing blocks. Allocate a new one.
    uint32_t new_block_num;
    if (allocate_block(fp, sb, gdt, num_block_groups, &new_block_num) != 0) {
        free(block_buffer);
        return -2; // Failed to allocate new block
    }

    // Find a free i_block entry (direct only for now)
    int free_block_idx = -1;
    for (int i = 0; i < 12; ++i) {
        if (parent_inode->i_block[i] == 0) {
            free_block_idx = i;
            break;
        }
    }

    if (free_block_idx == -1) {
        // TODO: Handle indirect blocks
        free(block_buffer);
        // We should probably free the block we just allocated here, but that's for later.
        return -3; // No free direct block pointers
    }

    // Add new block to parent inode
    parent_inode->i_block[free_block_idx] = new_block_num;
    parent_inode->i_size += block_size;
    parent_inode->i_blocks += block_size / 512;

    // Initialize the new block with the new entry
    const auto new_entry = (struct ext2_dir_entry_2 *) block_buffer;
    new_entry->inode = new_entry_inode_num;
    new_entry->rec_len = block_size;
    new_entry->name_len = name_len;
    new_entry->file_type = new_entry_type;
    strncpy(new_entry->name, new_entry_name, name_len);

    // Write the new block to disk
    const off_t block_offset = new_block_num * block_size;
    fseeko(fp, block_offset, SEEK_SET);
    fwrite(block_buffer, block_size, 1, fp);

    free(block_buffer);
    return 0;
}

int create_directory(FILE *fp, struct ext2_super_block *sb, struct ext2_group_desc *gdt, const uint32_t num_block_groups,
                     const uint32_t parent_inode_num, const char *new_dir_name, uint32_t *new_inode_num_out) {
    if (strlen(new_dir_name) > EXT2_NAME_LEN) {
        return -1; // Name too long
    }

    uint32_t new_inode_num;
    if (allocate_inode(fp, sb, gdt, num_block_groups, &new_inode_num) != 0) {
        return -2; // Failed to allocate inode
    }

    uint32_t new_block_num;
    if (allocate_block(fp, sb, gdt, num_block_groups, &new_block_num) != 0) {
        // TODO: Deallocate inode
        return -3; // Failed to allocate block
    }

    const uint32_t block_size = get_block_size(sb);

    // Initialize the new directory's inode
    struct ext2_inode new_inode = {0};
    new_inode.i_mode = EXT2_S_IFDIR | 0755;
    new_inode.i_links_count = 2; // For '.' and the entry in the parent
    new_inode.i_size = block_size;
    new_inode.i_blocks = block_size / 512;
    new_inode.i_atime = new_inode.i_ctime = new_inode.i_mtime = time(nullptr);
    new_inode.i_block[0] = new_block_num;

    // Initialize the new data block with '.' and '..'
    const auto block_buffer = (char *) calloc(1, block_size);
    // '.' entry
    const auto self_entry = (struct ext2_dir_entry_2 *) block_buffer;
    self_entry->inode = new_inode_num;
    self_entry->name_len = 1;
    strcpy(self_entry->name, ".");
    self_entry->file_type = EXT2_FT_DIR;
    self_entry->rec_len = EXT2_DIR_REC_LEN(self_entry->name_len);
    // '..' entry
    const auto parent_entry = (struct ext2_dir_entry_2 *) (block_buffer + self_entry->rec_len);
    parent_entry->inode = parent_inode_num;
    parent_entry->name_len = 2;
    strcpy(parent_entry->name, "..");
    parent_entry->file_type = EXT2_FT_DIR;
    parent_entry->rec_len = block_size - self_entry->rec_len;

    // Write the new block to disk
    fseeko(fp, (off_t) new_block_num * block_size, SEEK_SET);
    fwrite(block_buffer, block_size, 1, fp);
    free(block_buffer);

    // Add entry to parent directory
    struct ext2_inode parent_inode;
    read_inode(fp, sb, gdt, parent_inode_num, &parent_inode);
    add_directory_entry(fp, sb, gdt, num_block_groups, &parent_inode, parent_inode_num, new_inode_num, new_dir_name,
                        EXT2_FT_DIR);
    parent_inode.i_links_count++;
    parent_inode.i_mtime = parent_inode.i_ctime = time(nullptr);
    write_inode(fp, sb, gdt, parent_inode_num, &parent_inode);

    // Write the new inode to disk
    write_inode(fp, sb, gdt, new_inode_num, &new_inode);

    if (new_inode_num_out) {
        *new_inode_num_out = new_inode_num;
    }

    return 0;
}
