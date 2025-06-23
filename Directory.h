#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "Superblock.h"
#include "Inode.h"
#include "BlockGroup.h"

#include <stdint.h>
#include <stdio.h>

#define EXT2_NAME_LEN 255

typedef struct {
    uint32_t inode;     // Inode number (0 if entry is unused)
    uint16_t rec_len;   // Directory entry length (total size of this entry in bytes)
    uint8_t  name_len;  // Name length (actual length of the name string in characters)
    uint8_t  file_type; // File type indicator
    char     name[EXT2_NAME_LEN]; // File name characters (NOT necessarily null-terminated within this array)
                                  // The actual name occupies the first 'name_len' bytes.
} ext2_directory_entry;

#define EXT2_FT_UNKNOWN  0 // Unknown Type
#define EXT2_FT_REG_FILE 1 // Regular File
#define EXT2_FT_DIR      2 // Directory
#define EXT2_FT_CHRDEV   3 // Character Device
#define EXT2_FT_BLKDEV   4 // Block Device
#define EXT2_FT_FIFO     5 // FIFO (named pipe)
#define EXT2_FT_SOCK     6 // Socket
#define EXT2_FT_SYMLINK  7 // Symbolic Link

// Minimum size of a directory entry's fixed part (inode + rec_len + name_len + file_type)
#define EXT2_DIR_ENTRY_FIXED_SIZE (sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint8_t))

// Macro to calculate the padded length of a directory entry based on name length.
// Directory entries must be 4-byte aligned.
// (8 bytes for fixed fields + name_len, then rounded up to a multiple of 4)
#define EXT2_DIR_REC_LEN(name_len) (((name_len) + EXT2_DIR_ENTRY_FIXED_SIZE + 3) & ~3)

/**
 * @brief Reads and lists the entries of a directory.
 *
 * This function reads the data blocks of the specified directory inode and prints
 * the details of each directory entry found within those blocks.
 * Currently, it only processes direct blocks (i_block[0] to i_block[11]).
 *
 * @param file Pointer to an open FILE stream for the filesystem image.
 * @param superblock Pointer to the filesystem's superblock.
 * @param block_group_descriptor_table Pointer to the array of block group descriptors (the BLOCK_GROUP_DESCRIPTOR_TABLE).
 * @param dir_inode_num The inode number of the directory to list.
 * @return 0 on success, or a negative error code on failure.
 */
int list_directory_entries(FILE *file, const ext2_super_block *superblock, const ext2_group_desc *block_group_descriptor_table, uint32_t dir_inode_num);

/**
 * @brief Creates a new directory.
 *
 * This function orchestrates the entire process of creating a new directory:
 * 1. Allocates a new inode.
 * 2. Allocates a new data block.
 * 3. Initializes the new inode as a directory.
 * 4. Initializes the new data block with '.' and '..' entries.
 * 5. Adds an entry for the new directory into its parent directory.
 * 6. Updates all on-disk structures (bitmaps, superblock, group descriptors, inodes).
 *
 * @param file                Pointer to the filesystem image file.
 * @param superblock                Pointer to the superblock (will be updated).
 * @param block_group_descriptor_table               Pointer to the BLOCK_GROUP_DESCRIPTOR_TABLE (will be updated).
 * @param parent_inode_num  Inode number of the parent directory.
 * @param new_dir_name      The name for the new directory.
 * @param new_inode_num_out Pointer to store the newly created inode number.
 * @return 0 on success, or a negative error code on failure.
 */
int create_directory(
    FILE *file,
    ext2_super_block *superblock,
    ext2_group_desc_table *block_group_descriptor_table,
    uint32_t parent_inode_num,
    const char *new_dir_name,
    uint32_t *new_inode_num_out
);

/**
 * @brief (Helper) Adds a new entry to a directory's data block(s).
 *
 * This function finds space within a directory's existing data blocks or allocates a
 * new block if necessary, then writes the new directory entry. It updates the parent
 * inode in memory (e.g., size, block count) but does NOT write it to disk.
 * The caller is responsible for writing the modified parent inode.
 *
 * @param file                Pointer to the filesystem image file.
 * @param superblock                Pointer to the superblock.
 * @param block_group_descriptor_table               Pointer to the BLOCK_GROUP_DESCRIPTOR_TABLE.
 * @param parent_inode      Pointer to the parent directory's inode (will be updated in memory).
 * @param new_entry_inode_num Inode number for the new entry.
 * @param new_entry_name    Name for the new entry.
 * @param new_entry_type    File type for the new entry (EXT2_FT_*).
 * @return 0 on success, or a negative error code on failure.
 */
int add_directory_entry(
    FILE *file,
    ext2_super_block *superblock,
    const ext2_group_desc_table *block_group_descriptor_table,
    ext2_inode *parent_inode,
    uint32_t new_entry_inode_num,
    const char *new_entry_name,
    uint8_t new_entry_type
);

/**
 * @brief Resolves a path to an inode number.
 *
 * Traverses the directory tree starting from the root to find the inode
 * corresponding to the given path.
 *
 * @param file Pointer to the filesystem image file.
 * @param superblock Pointer to the superblock.
 * @param block_group_descriptor_table Pointer to the block group descriptor table.
 * @param path The absolute path to resolve.
 * @return The inode number on success, or 0 if the path is not found or an error occurs.
 */
uint32_t get_inode_for_path(
    FILE *file,
    const ext2_super_block *superblock,
    const ext2_group_desc *block_group_descriptor_table,
    const char *path
);

#endif //DIRECTORY_H
