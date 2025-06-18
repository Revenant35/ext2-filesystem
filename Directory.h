#ifndef C_EXT2_FILESYSTEM_DIRECTORY_H
#define C_EXT2_FILESYSTEM_DIRECTORY_H

#include <stdint.h>

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


#endif //C_EXT2_FILESYSTEM_DIRECTORY_H
