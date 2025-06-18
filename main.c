#include <stdio.h>
#include <stdlib.h> // For EXIT_FAILURE, EXIT_SUCCESS
#include <stdint.h>   // For uint32_t, uint16_t, etc.
#include <string.h>   // For memcmp
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

    // --- Test write_superblock functionality ---
    const char *output_filename = "test_output.img";
    struct ext2_super_block sb_read_back;
    int read_back_status;

    printf("\n--- Testing write_superblock to %s ---\n", output_filename);

    // Open the output file for writing and reading (wb+)
    printf("Attempting to open output file: %s (mode wb+)\n", output_filename);
    FILE *fp_out = fopen(output_filename, "wb+");
    if (fp_out == NULL) {
        perror("Error opening output file for write test");
        return EXIT_FAILURE; // Critical error, cannot proceed with write test
    }

    // Write the original superblock to the new file
    printf("Attempting to write superblock to %s\n", output_filename);
    int write_status = write_superblock(fp_out, &sb);
    if (write_status != 0) {
        fprintf(stderr, "Failed to write superblock to %s. Error code: %d\n", output_filename, write_status);
        // Still need to close fp_out before exiting
    } else {
        printf("Superblock written successfully to %s.\n", output_filename);

        // Now, read it back from the same file
        printf("Attempting to read back superblock from %s\n", output_filename);
        // read_superblock will fseek from the beginning, so no explicit rewind needed after write
        read_back_status = read_superblock(fp_out, &sb_read_back);
        if (read_back_status != 0) {
            fprintf(stderr, "Failed to read back superblock from %s. Error code: %d\n", output_filename, read_back_status);
        } else {
            printf("Superblock read back successfully from %s.\n", output_filename);

            // Compare the original superblock with the one read back
            if (memcmp(&sb, &sb_read_back, sizeof(struct ext2_super_block)) == 0) {
                printf("SUCCESS: Original superblock and read-back superblock are identical.\n");
            } else {
                fprintf(stderr, "FAILURE: Original superblock and read-back superblock differ!\n");
                // Optionally, print details of differences here for debugging
            }
        }
    }

    // Close the output file
    if (fclose(fp_out) != 0) {
        perror("Error closing output file");
        // If other operations were successful, this still makes the overall test a failure.
        if (write_status == 0 && read_back_status == 0) {
            return EXIT_FAILURE;
        }
    }

    // Determine final exit status based on test outcomes
    if (write_status != 0 || read_back_status != 0 || memcmp(&sb, &sb_read_back, sizeof(struct ext2_super_block)) != 0) {
        fprintf(stderr, "--- write_superblock test FAILED. ---\n");
        return EXIT_FAILURE;
    }

    printf("--- write_superblock test completed successfully. ---\n");
    return EXIT_SUCCESS;
}
