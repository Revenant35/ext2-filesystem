#include <stdio.h>
#include <stdlib.h> // For EXIT_FAILURE, EXIT_SUCCESS
#include "Superblock.h" // For ext2_super_block structure

// Function to read the superblock from a file into memory
// filename: The path to the ext2 image file
// sb: Pointer to an ext2_super_block structure to populate
// Returns 0 on success, non-zero on failure
int read_superblock(const char *filename, struct ext2_super_block *sb) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    if (fseek(fp, EXT2_SUPERBLOCK_OFFSET, SEEK_SET) != 0) {
        perror("Error seeking to superblock");
        fclose(fp);
        return 2;
    }

    if (fread(sb, sizeof(struct ext2_super_block), 1, fp) != 1) {
        if (feof(fp)) {
            fprintf(stderr, "Error reading superblock: unexpected end of file.\n");
        } else if (ferror(fp)) {
            perror("Error reading superblock");
        }
        fclose(fp);
        return 3;
    }

    if (fclose(fp) != 0) {
        perror("Error closing file");
        return 4;
    }

    if (sb->s_magic != EXT2_SUPER_MAGIC) {
        fprintf(stderr, "Error: Not an ext2 filesystem (magic number mismatch: expected 0x%X, got 0x%X)\n", EXT2_SUPER_MAGIC, sb->s_magic);
        return 5;
    }

    return 0;
}

// TODO: Write Superblock to [stream/disk]



// TODO: Read filesystem into memory
// TODO: Write filesystem to disk
// TODO: Read Block Group Descriptors from [stream/disk]
// TODO: Write Block Group Descriptors to [stream/disk]

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <ext2_image_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    struct ext2_super_block sb;

    printf("Attempting to read superblock from: %s\n", filename);

    if (read_superblock(filename, &sb) == 0) {
        printf("Superblock read successfully!\n");
        printf("  Magic number: 0x%X\n", sb.s_magic);
        printf("  Inodes count: %u\n", sb.s_inodes_count);
        printf("  Blocks count: %u\n", sb.s_blocks_count);
        printf("  Free blocks count: %u\n", sb.s_free_blocks_count);
        printf("  Free inodes count: %u\n", sb.s_free_inodes_count);
        printf("  Block size (log2): %u (Actual: %u bytes)\n", sb.s_log_block_size, 1024 << sb.s_log_block_size);
        printf("  Blocks per group: %u\n", sb.s_blocks_per_group);
        printf("  Inodes per group: %u\n", sb.s_inodes_per_group);
        printf("  Creator OS: %u\n", sb.s_creator_os);
        printf("  Revision level: %u\n", sb.s_rev_level);
        if (sb.s_rev_level >= EXT2_DYNAMIC_REV) {
            printf("  Inode size: %u bytes\n", sb.s_inode_size);
            printf("  Volume name: %.16s\n", sb.s_volume_name);
        }
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "Failed to read superblock from %s.\n", filename);
        return EXIT_FAILURE;
    }
}