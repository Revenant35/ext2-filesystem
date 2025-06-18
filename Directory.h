#ifndef C_EXT2_FILESYSTEM_DIRECTORY_H
#define C_EXT2_FILESYSTEM_DIRECTORY_H

#include <stdint.h>
#include <stdio.h>

struct ext2_super_block;
struct ext2_group_desc;
struct ext2_inode;

#define EXT2_NAME_LEN 255

struct ext2_dir_entry_2 {
    uint32_t inode;     // Inode number (0 if entry is unused)
    uint16_t rec_len;   // Directory entry length (total size of this entry in bytes)
    uint8_t  name_len;  // Name length (actual length of the name string in characters)
    uint8_t  file_type; // File type indicator
    char     name[EXT2_NAME_LEN]; // File name characters (NOT necessarily null-terminated within this array)
                                  // The actual name occupies the first 'name_len' bytes.
};

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
 * @param fp Pointer to an open FILE stream for the filesystem image.
 * @param sb Pointer to the filesystem's superblock.
 * @param gdt Pointer to the array of block group descriptors (the GDT).
 * @param dir_inode_num The inode number of the directory to list.
 * @return 0 on success, or a negative error code on failure.
 */
int list_directory_entries(FILE *fp, const struct ext2_super_block *sb, const struct ext2_group_desc *gdt, uint32_t dir_inode_num);


#endif //C_EXT2_FILESYSTEM_DIRECTORY_H
