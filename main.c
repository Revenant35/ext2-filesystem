#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "Superblock.h"
#include "BlockGroup.h"
#include "Directory.h" // For list_directory_entries
#include "Inode.h"     // For EXT2_ROOT_INO constant

// Function to list the root directory entries
void test_create_directory(const char *filesystem_image_path) {
    printf("\n--- Testing Directory Creation ---\n");
    FILE *file = fopen(filesystem_image_path, "r+");
    if (!file) {
        perror("Failed to open filesystem image for writing");
        return;
    }

    ext2_super_block superblock;
    if (read_superblock(file, &superblock) != 0) {
        fclose(file);
        return;
    }

    uint32_t num_groups;
    ext2_group_desc *block_group_descriptor_table = read_all_group_descriptors(file, &superblock, &num_groups);
    if (!block_group_descriptor_table) {
        fclose(file);
        return;
    }

    printf("Attempting to create '/new_dir'...\n");
    uint32_t new_inode;
    if (create_directory(file, &superblock, block_group_descriptor_table, num_groups, EXT2_ROOT_INO, "new_dir", &new_inode) == 0) {
        printf("Successfully created '/new_dir' with inode %u\n", new_inode);
    } else {
        fprintf(stderr, "Failed to create directory.\n");
    }

    printf("\n--- Listing Root Directory After Creation ---\n");
    list_directory_entries(file, &superblock, block_group_descriptor_table, EXT2_ROOT_INO);

    free(block_group_descriptor_table);
    fclose(file);
    printf("--- Test Complete ---\n");
}

void list_root_directory(const char *filesystem_image_path) {
    printf("\n--- Listing Root Directory Entries from %s ---\n", filesystem_image_path);

    FILE *file = fopen(filesystem_image_path, "rb");
    if (file == NULL) {
        perror("Error opening filesystem image for root directory listing");
        return;
    }

    ext2_super_block superblock;
    if (read_superblock(file, &superblock) != 0) {
        fprintf(stderr, "Failed to read superblock for root directory listing.\n");
        fclose(file);
        return;
    }

    uint32_t num_groups_read = 0;
    ext2_group_desc *block_group_descriptor_table = read_all_group_descriptors(file, &superblock, &num_groups_read);
    if (block_group_descriptor_table == NULL) {
        fprintf(stderr, "Failed to read block group descriptors for root directory listing.\n");
        fclose(file);
        return;
    }

    // List entries for the root directory (inode 2)
    // EXT2_ROOT_INO is defined as 2 in ext2_fs.h, commonly used in ext2 implementations.
    // If not available in your Inode.h, you might need to define it or use 2 directly.
    // Assuming Inode.h might have it or it's a common understanding.
    int list_status = list_directory_entries(file, &superblock, block_group_descriptor_table, EXT2_ROOT_INO);
    if (list_status != 0) {
        fprintf(stderr, "Failed to list root directory entries. Error code: %d\n", list_status);
    }

    free(block_group_descriptor_table);
    if (fclose(file) != 0) {
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
    ext2_super_block superblock;
    int read_status = read_superblock(fp_orig_img, &superblock);

    if (read_status != 0) {
        fprintf(stderr, "Failed to successfully process superblock from %s. Error code: %d\n", filename, read_status);
        return EXIT_FAILURE;
    }

    printf("Superblock read successfully!\n");
    printf("  Magic number: 0x%X\n", superblock.s_magic);
    printf("  Inodes count: %u\n", superblock.s_inodes_count);
    printf("  Blocks count: %u\n", superblock.s_blocks_count);
    printf("  Free blocks count: %u\n", superblock.s_free_blocks_count);
    printf("  Free inodes count: %u\n", superblock.s_free_inodes_count);
    printf("  Block size (log2): %u (Actual: %u bytes)\n", superblock.s_log_block_size, 1024 << superblock.s_log_block_size);
    printf("  Blocks per group: %u\n", superblock.s_blocks_per_group);
    printf("  Inodes per group: %u\n", superblock.s_inodes_per_group);

    // Print File System State
    printf("  File System State: ");
    switch (superblock.s_state) {
        case EXT2_VALID_FS: printf("Cleanly unmounted\n"); break;
        case EXT2_ERROR_FS: printf("Errors detected\n"); break;
        default: printf("Unknown (%u)\n", superblock.s_state); break;
    }

    // Print Error Handling Method
    printf("  Error Handling: ");
    switch (superblock.s_errors) {
        case EXT2_ERRORS_CONTINUE: printf("Continue\n"); break;
        case EXT2_ERRORS_RO: printf("Remount read-only\n"); break;
        case EXT2_ERRORS_PANIC: printf("Panic\n"); break;
        default: printf("Unknown (%u)\n", superblock.s_errors); break;
    }

    // Print Creator OS
    printf("  Creator OS: ");
    switch (superblock.s_creator_os) {
        case EXT2_OS_LINUX: printf("Linux\n"); break;
        case EXT2_OS_HURD: printf("GNU HURD\n"); break;
        case EXT2_OS_MASIX: printf("MASIX\n"); break;
        case EXT2_OS_FREEBSD: printf("FreeBSD\n"); break;
        case EXT2_OS_LITES: printf("Lites\n"); break;
        default: printf("Unknown (%u)\n", superblock.s_creator_os); break;
    }

    printf("  Revision level: %u\n", superblock.s_rev_level);
    if (superblock.s_rev_level >= EXT2_DYNAMIC_REV) {
        printf("  Inode size: %u bytes\n", superblock.s_inode_size);
        printf("  Volume name: %.16s\n", superblock.s_volume_name);
    }

    // --- Test write_superblock functionality ---
    const char *output_filename = "test_output.img";
    ext2_super_block superblock_read_back;
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
    int write_status = write_superblock(fp_out, &superblock);
    if (write_status != 0) {
        fprintf(stderr, "Failed to write superblock to %s. Error code: %d\n", output_filename, write_status);
        // Still need to close fp_out before exiting
    } else {
        printf("Superblock written successfully to %s.\n", output_filename);

        // Now, read it back from the same file
        printf("Attempting to read back superblock from %s\n", output_filename);
        // read_superblock will fseek from the beginning, so no explicit rewind needed after write
        read_back_status = read_superblock(fp_out, &superblock_read_back);
        if (read_back_status != 0) {
            fprintf(stderr, "Failed to read back superblock from %s. Error code: %d\n", output_filename, read_back_status);
        } else {
            printf("Superblock read back successfully from %s.\n", output_filename);

            // Compare the original superblock with the one read back
            if (memcmp(&superblock, &superblock_read_back, sizeof(ext2_super_block)) == 0) {
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
    if (write_status != 0 || read_back_status != 0 || memcmp(&superblock, &superblock_read_back, sizeof(ext2_super_block)) != 0) {
        fprintf(stderr, "--- write_superblock test FAILED. ---\n");
        return EXIT_FAILURE;
    }
    printf("--- write_superblock test completed successfully. ---\n");

    // --- Read and print Block Group Descriptors ---
    printf("\n--- Reading Block Group Descriptors ---\n");
    uint32_t num_groups_read = 0;
    ext2_group_desc *block_group_descriptor_table = read_all_group_descriptors(fp_orig_img, &superblock, &num_groups_read);

    // Close the original image file pointer AFTER we are done with all reads from it
    if (fclose(fp_orig_img) != 0) {
        perror("Error closing original image file");
        // Decide if this should be a fatal error for the rest of the program flow
        if (block_group_descriptor_table != NULL) free(block_group_descriptor_table); // Clean up if BLOCK_GROUP_DESCRIPTOR_TABLE was read before close error
        return EXIT_FAILURE; 
    }

    if (block_group_descriptor_table == NULL) {
        fprintf(stderr, "Failed to read block group descriptors.\n");
        return EXIT_FAILURE;
    }

    printf("Successfully read %u block group descriptors.\n", num_groups_read);
    uint32_t groups_to_print = num_groups_read < 3 ? num_groups_read : 3; // Print up to 3 groups
    for (uint32_t i = 0; i < groups_to_print; ++i) {
        printf("  Group %u:\n", i);
        printf("    Block Bitmap Block: %u\n", block_group_descriptor_table[i].bg_block_bitmap);
        printf("    Inode Bitmap Block: %u\n", block_group_descriptor_table[i].bg_inode_bitmap);
        printf("    Inode Table Block:  %u\n", block_group_descriptor_table[i].bg_inode_table);
        printf("    Free Blocks:      %u\n", block_group_descriptor_table[i].bg_free_blocks_count);
        printf("    Free Inodes:      %u\n", block_group_descriptor_table[i].bg_free_inodes_count);
        printf("    Used Dirs:        %u\n", block_group_descriptor_table[i].bg_used_dirs_count);
        printf("    Flags:            0x%X\n", block_group_descriptor_table[i].bg_flags);
    }

    free(block_group_descriptor_table); // Free the allocated BLOCK_GROUP_DESCRIPTOR_TABLE memory
    printf("--- Block group descriptor processing complete. ---\n");

    // Call the function to list the root directory from the original image
    // This is done after closing fp_orig_img and freeing block_group_descriptor_table from the main scope's operations.
    // The list_root_directory function will reopen the file and re-read superblock and block_group_descriptor_table.
    // This keeps it self-contained. Alternatively, we could pass superblock and block_group_descriptor_table if fp_orig_img was still open.
    list_root_directory(filename);

    test_create_directory(filename);

    return SUCCESS;
}
