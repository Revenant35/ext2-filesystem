/**
 * @file Superblock.c
 * @brief Implements functions for reading and writing the ext2 superblock.
 *
 * These functions handle the I/O operations to load the superblock from a
 * filesystem image into memory and to write a modified superblock back to the image.
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
        fprintf(stderr, "Error: read_superblock received a NULL file pointer.\n");
        return INVALID_PARAMETER;
    }

    if (fseek(file, EXT2_SUPERBLOCK_OFFSET, SEEK_SET) != 0) {
        perror("Error seeking to superblock");
        return 2;
    }

    if (fread(superblock, sizeof(ext2_super_block), 1, file) != 1) {
        if (feof(file)) {
            fprintf(stderr, "Error reading superblock: unexpected end of file.\n");
        } else if (ferror(file)) {
            perror("Error reading superblock");
        }
        return 3;
    }

    if (superblock->s_magic != EXT2_SUPER_MAGIC) {
        fprintf(stderr, "Error: Not an ext2 filesystem (magic number mismatch: expected 0x%X, got 0x%X)\n",
                EXT2_SUPER_MAGIC, superblock->s_magic);
        return 4;
    }

    return SUCCESS;
}

int write_superblock(
    FILE *file,
    const ext2_super_block *superblock
) {
    if (file == NULL) {
        fprintf(stderr, "Error: write_superblock received a NULL file pointer.\n");
        return INVALID_PARAMETER;
    }

    if (superblock == NULL) {
        fprintf(stderr, "Error: write_superblock received a NULL superblock pointer.\n");
        return 2;
    }

    if (superblock->s_magic != EXT2_SUPER_MAGIC) {
        fprintf(
            stderr,
            "Error: write_superblock attempted to write an invalid superblock (magic number mismatch: expected 0x%X, got 0x%X).\n",
            EXT2_SUPER_MAGIC, superblock->s_magic);
        return 3;
    }

    if (fseek(file, EXT2_SUPERBLOCK_OFFSET, SEEK_SET) != 0) {
        perror("Error seeking to superblock for writing");
        return 4;
    }

    if (fwrite(superblock, sizeof(ext2_super_block), 1, file) != 1) {
        if (ferror(file)) {
            perror("Error writing superblock");
        } else {
            fprintf(stderr, "Error writing superblock: fwrite did not write the expected number of items.\n");
        }
        return 5;
    }

    return SUCCESS;
}
