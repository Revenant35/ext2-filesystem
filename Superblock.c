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

/**
 * @brief Reads the superblock from an open file stream into memory.
 *
 * This function seeks to the predefined superblock offset (1024 bytes) in the
 * given file stream, reads the superblock data into the provided structure,
 * and verifies the ext2 magic number.
 *
 * @param fp Pointer to an open FILE stream representing the ext2 filesystem image.
 *           The file pointer must be valid and opened in a mode that allows reading.
 * @param sb Pointer to an `ext2_super_block` structure where the read superblock
 *           data will be stored. This structure must be allocated by the caller.
 * @return 0 on successful read and validation.
 * @return 1 if `fp` is NULL.
 * @return 2 if seeking to the superblock offset fails.
 * @return 3 if reading the superblock data fails (e.g., fread error or EOF).
 * @return 4 if the ext2 magic number is incorrect, indicating it's not a valid
 *           ext2 filesystem or the superblock is corrupted.
 */
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

/**
 * @brief Writes the superblock from memory to an open file stream.
 *
 * This function seeks to the predefined superblock offset (1024 bytes) in the
 * given file stream and writes the data from the provided `ext2_super_block`
 * structure to the filesystem image. It also checks if the superblock to be
 * written has a valid ext2 magic number.
 *
 * @param fp Pointer to an open FILE stream representing the ext2 filesystem image.
 *           The file pointer must be valid and opened in a mode that allows writing.
 * @param sb Pointer to a constant `ext2_super_block` structure containing the
 *           superblock data to be written. This structure should be populated and
 *           valid before calling this function.
 * @return 0 on successful write.
 * @return 1 if `fp` is NULL.
 * @return 2 if `sb` is NULL.
 * @return 3 if the superblock to be written has an invalid ext2 magic number.
 * @return 4 if seeking to the superblock offset for writing fails.
 * @return 5 if writing the superblock data fails (e.g., fwrite error).
 */
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
