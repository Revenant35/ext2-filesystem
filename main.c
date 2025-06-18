#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "Superblock.h"
#include "BlockGroup.h"
#include "Directory.h" // For list_directory_entries
#include "Inode.h"     // For EXT2_ROOT_INO constant

// Function to list the root directory entries
void list_root_directory(const char *filesystem_image_path) {
    printf("\n--- Listing Root Directory Entries from %s ---\n", filesystem_image_path);

    FILE *fp = fopen(filesystem_image_path, "rb");
    if (fp == NULL) {
        perror("Error opening filesystem image for root directory listing");
        return;
    }

    struct ext2_super_block sb;
    if (read_superblock(fp, &sb) != 0) {
        fprintf(stderr, "Failed to read superblock for root directory listing.\n");
        fclose(fp);
        return;
    }

    uint32_t num_groups_read = 0;
    struct ext2_group_desc *gdt = read_all_group_descriptors(fp, &sb, &num_groups_read);
    if (gdt == NULL) {
        fprintf(stderr, "Failed to read block group descriptors for root directory listing.\n");
        fclose(fp);
        return;
    }

    // List entries for the root directory (inode 2)
    // EXT2_ROOT_INO is defined as 2 in ext2_fs.h, commonly used in ext2 implementations.
    // If not available in your Inode.h, you might need to define it or use 2 directly.
    // Assuming Inode.h might have it or it's a common understanding.
    int list_status = list_directory_entries(fp, &sb, gdt, EXT2_ROOT_INO);
    if (list_status != 0) {
        fprintf(stderr, "Failed to list root directory entries. Error code: %d\n", list_status);
    }

    free(gdt);
    if (fclose(fp) != 0) {
        perror("Error closing filesystem image after listing root directory");
    }
    printf("--- Root directory listing complete. ---\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <ext2_image_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];

    printf("Attempting to open: %s\n", filename);
    FILE *fp_orig_img = fopen(filename, "rb"); // Renamed to avoid confusion later
    if (fp_orig_img == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    printf("Attempting to read superblock from: %s\n", filename);
    struct ext2_super_block sb;
    int read_status = read_superblock(fp_orig_img, &sb);

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

    // --- Read and print Block Group Descriptors ---
    printf("\n--- Reading Block Group Descriptors ---\n");
    uint32_t num_groups_read = 0;
    struct ext2_group_desc *gdt = read_all_group_descriptors(fp_orig_img, &sb, &num_groups_read);

    // Close the original image file pointer AFTER we are done with all reads from it
    if (fclose(fp_orig_img) != 0) {
        perror("Error closing original image file");
        // Decide if this should be a fatal error for the rest of the program flow
        if (gdt != NULL) free(gdt); // Clean up if GDT was read before close error
        return EXIT_FAILURE; 
    }

    if (gdt == NULL) {
        fprintf(stderr, "Failed to read block group descriptors.\n");
        return EXIT_FAILURE;
    }

    printf("Successfully read %u block group descriptors.\n", num_groups_read);
    uint32_t groups_to_print = num_groups_read < 3 ? num_groups_read : 3; // Print up to 3 groups
    for (uint32_t i = 0; i < groups_to_print; ++i) {
        printf("  Group %u:\n", i);
        printf("    Block Bitmap Block: %u\n", gdt[i].bg_block_bitmap);
        printf("    Inode Bitmap Block: %u\n", gdt[i].bg_inode_bitmap);
        printf("    Inode Table Block:  %u\n", gdt[i].bg_inode_table);
        printf("    Free Blocks:      %u\n", gdt[i].bg_free_blocks_count);
        printf("    Free Inodes:      %u\n", gdt[i].bg_free_inodes_count);
        printf("    Used Dirs:        %u\n", gdt[i].bg_used_dirs_count);
        printf("    Flags:            0x%X\n", gdt[i].bg_flags);
    }

    free(gdt); // Free the allocated GDT memory
    printf("--- Block group descriptor processing complete. ---\n");

    // Call the function to list the root directory from the original image
    // This is done after closing fp_orig_img and freeing gdt from the main scope's operations.
    // The list_root_directory function will reopen the file and re-read sb and gdt.
    // This keeps it self-contained. Alternatively, we could pass sb and gdt if fp_orig_img was still open.
    list_root_directory(filename);

    return EXIT_SUCCESS;
}
