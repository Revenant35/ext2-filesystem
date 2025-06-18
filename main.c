#include <stdio.h>
#include <stdlib.h> // For EXIT_FAILURE, EXIT_SUCCESS
#include "Superblock.h" // For ext2_super_block structure


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

    printf("Attempting to open: %s\n", filename);
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    printf("Attempting to read superblock from: %s\n", filename);
    struct ext2_super_block sb;
    int read_status = read_superblock(fp, &sb);

    if (fclose(fp) != 0) {
        perror("Error closing file");
        return EXIT_FAILURE;
    }

    if (read_status != 0) {
        fprintf(stderr, "Failed to successfully process superblock from %s. Error code: %d\n", filename, read_status);
        return EXIT_FAILURE;
    }

    printf("Superblock read successfully!\n");
    printf("  Magic number: 0x%X\n", sb.s_magic);
    printf("  Inodes count: %u\n", sb.s_inodes_count);
    printf("  Blocks count: %u\n", sb.s_blocks_count);
    printf("  Free blocks count: %u\n", sb.s_free_blocks_count);
    printf("  Free inodes count: %u\n", sb.s_free_inodes_count);
    printf("  Block size (log2): %u (Actual: %u bytes)\n", sb.s_log_block_size, 1024 << sb.s_log_block_size);
    printf("  Blocks per group: %u\n", sb.s_blocks_per_group);
    printf("  Inodes per group: %u\n", sb.s_inodes_per_group);

    // Print File System State
    printf("  File System State: ");
    switch (sb.s_state) {
        case EXT2_VALID_FS: printf("Cleanly unmounted\n"); break;
        case EXT2_ERROR_FS: printf("Errors detected\n"); break;
        default: printf("Unknown (%u)\n", sb.s_state); break;
    }

    // Print Error Handling Method
    printf("  Error Handling: ");
    switch (sb.s_errors) {
        case EXT2_ERRORS_CONTINUE: printf("Continue\n"); break;
        case EXT2_ERRORS_RO: printf("Remount read-only\n"); break;
        case EXT2_ERRORS_PANIC: printf("Panic\n"); break;
        default: printf("Unknown (%u)\n", sb.s_errors); break;
    }

    // Print Creator OS
    printf("  Creator OS: ");
    switch (sb.s_creator_os) {
        case EXT2_OS_LINUX: printf("Linux\n"); break;
        case EXT2_OS_HURD: printf("GNU HURD\n"); break;
        case EXT2_OS_MASIX: printf("MASIX\n"); break;
        case EXT2_OS_FREEBSD: printf("FreeBSD\n"); break;
        case EXT2_OS_LITES: printf("Lites\n"); break;
        default: printf("Unknown (%u)\n", sb.s_creator_os); break;
    }

    printf("  Revision level: %u\n", sb.s_rev_level);
    if (sb.s_rev_level >= EXT2_DYNAMIC_REV) {
        printf("  Inode size: %u bytes\n", sb.s_inode_size);
        printf("  Volume name: %.16s\n", sb.s_volume_name);
    }

    return EXIT_SUCCESS;
}