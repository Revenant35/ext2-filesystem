#include <stdio.h>
#include <stdlib.h>
#include "Superblock.h"

// Function to read the superblock from an open file stream into memory
// fp: Pointer to an open FILE stream (e.g., from an ext2 image file)
// sb: Pointer to an ext2_super_block structure to populate
// Returns 0 on success, non-zero on failure
int read_superblock(FILE *fp, struct ext2_super_block *sb) {
    if (fp == NULL) {
        fprintf(stderr, "Error: read_superblock received a NULL file pointer.\n");
        return 1;
    }

    if (fseek(fp, EXT2_SUPERBLOCK_OFFSET, SEEK_SET) != 0) {
        perror("Error seeking to superblock");
        return 2;
    }

    if (fread(sb, sizeof(struct ext2_super_block), 1, fp) != 1) {
        if (feof(fp)) {
            fprintf(stderr, "Error reading superblock: unexpected end of file.\n");
        } else if (ferror(fp)) {
            perror("Error reading superblock");
        }
        return 3;
    }

    if (sb->s_magic != EXT2_SUPER_MAGIC) {
        fprintf(stderr, "Error: Not an ext2 filesystem (magic number mismatch: expected 0x%X, got 0x%X)\n",
                EXT2_SUPER_MAGIC, sb->s_magic);
        return 4;
    }

    return 0;
}

// Function to write the superblock from memory to an open file stream
// fp: Pointer to an open FILE stream (e.g., for an ext2 image file)
// sb: Pointer to an ext2_super_block structure containing the data to write
// Returns 0 on success, non-zero on failure
int write_superblock(FILE *fp, const struct ext2_super_block *sb) {
    if (fp == NULL) {
        fprintf(stderr, "Error: write_superblock received a NULL file pointer.\n");
        return 1;
    }

    if (sb == NULL) {
        fprintf(stderr, "Error: write_superblock received a NULL superblock pointer.\n");
        return 2;
    }

    if (sb->s_magic != EXT2_SUPER_MAGIC) {
        fprintf(
            stderr,
            "Error: write_superblock attempted to write an invalid superblock (magic number mismatch: expected 0x%X, got 0x%X).\n",
            EXT2_SUPER_MAGIC, sb->s_magic);
        return 3;
    }

    if (fseek(fp, EXT2_SUPERBLOCK_OFFSET, SEEK_SET) != 0) {
        perror("Error seeking to superblock for writing");
        return 4;
    }

    if (fwrite(sb, sizeof(struct ext2_super_block), 1, fp) != 1) {
        if (ferror(fp)) {
            perror("Error writing superblock");
        } else {
            fprintf(stderr, "Error writing superblock: fwrite did not write the expected number of items.\n");
        }
        return 5;
    }

    return 0;
}
