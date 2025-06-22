/**
 * @file Superblock.c
 * @brief Implements functions for reading, writing, and interpreting the ext2 superblock.
 *
 * These functions handle the I/O operations to load the superblock from a
 * filesystem image into memory, write it back, and calculate derived values.
 */
#include <stdio.h>
#include <stdlib.h>
#include "Superblock.h"

#include "globals.h"


int read_superblock(
    FILE *file,
    ext2_super_block *superblock
) {
    if (file == NULL) {
        log_error("Error: read_superblock received a NULL file pointer.\n");
        return INVALID_PARAMETER;
    }

    if (fseek(file, EXT2_SUPERBLOCK_OFFSET, SEEK_SET) != 0) {
        log_error("Error seeking to superblock");
        return 2;
    }

    if (fread(superblock, sizeof(ext2_super_block), 1, file) != 1) {
        if (feof(file)) {
            log_error("Error reading superblock: unexpected end of file.\n");
        } else if (ferror(file)) {
            log_error("Error reading superblock");
        }
        return 3;
    }

    if (superblock->s_magic != EXT2_SUPER_MAGIC) {
        log_error("Error: Not an ext2 filesystem (magic number mismatch: expected 0x%X, got 0x%X)\n",
                EXT2_SUPER_MAGIC, superblock->s_magic);
        return 4;
    }

    return SUCCESS;
}

int write_superblock(
    FILE *file,
    const ext2_super_block *superblock
) {
    if (file == NULL || superblock == NULL) {
        log_error("Error: write_superblock received a NULL pointer.\n");
        return INVALID_PARAMETER;
    }

    if (superblock->s_magic != EXT2_SUPER_MAGIC) {
        log_error(
            "Error: write_superblock attempted to write an invalid superblock (magic number mismatch: expected 0x%X, got 0x%X).\n",
            EXT2_SUPER_MAGIC, superblock->s_magic);
        return 3;
    }

    if (fseek(file, EXT2_SUPERBLOCK_OFFSET, SEEK_SET) != 0) {
        log_error("Error seeking to superblock for writing");
        return 4;
    }

    if (fwrite(superblock, sizeof(ext2_super_block), 1, file) != 1) {
        if (ferror(file)) {
            log_error("Error writing superblock");
        } else {
            log_error("Error writing superblock: fwrite did not write the expected number of items.\n");
        }
        return IO_ERROR;
    }

    return SUCCESS;
}

uint32_t get_block_size(const ext2_super_block *superblock) {
    if (superblock == NULL) {
        return 0;
    }

    return 1024 << superblock->s_log_block_size;
}

uint32_t get_fragment_size(const ext2_super_block *superblock) {
    if (superblock == NULL) {
        return 0;
    }

    return 1024 << superblock->s_log_frag_size;

}

uint32_t get_block_group_count(const ext2_super_block *superblock) {
    if (superblock == NULL) {
        return 0;
    }

    const uint32_t blocks_count = superblock->s_blocks_count;
    const uint32_t blocks_per_group = superblock->s_blocks_per_group;

    if (blocks_per_group == 0) {
        return 0;
    }

    return (blocks_count + blocks_per_group - 1) / blocks_per_group;
}
