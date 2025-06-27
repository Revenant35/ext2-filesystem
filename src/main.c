#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/superblock.h"
#include "../include/block_group.h"
#include "../include/directory.h"
#include "../include/globals.h"

#define MAX_CMD_LEN 1024
#define MAX_ARGS 10

// Command handler for 'ls'
void handle_ls(FILE *file, const ext2_super_block *superblock, const ext2_group_desc *bgdt, int argc, char *argv[]) {
    const char *path = (argc > 1) ? argv[1] : "/";
    printf("Listing directory for path: %s\n", path);
    const uint32_t inode_num = get_inode_for_path(file, superblock, bgdt, path);

    if (inode_num == 0) {
        log_error("Could not find path: %s\n", path);
        return;
    }

    list_directory_entries(file, superblock, bgdt, inode_num);
}

// Function to parse the command line input
void parse_command(char *cmd_line, int *argc, char *argv[]) {
    *argc = 0;
    char *token = strtok(cmd_line, " \t\n");
    while (token != NULL && *argc < MAX_ARGS) {
        argv[(*argc)++] = token;
        token = strtok(NULL, " \t\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        log_error("Usage: %s <ext2_image_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];

    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        log_error("Error opening filesystem image: %s\n", filename);
        return EXIT_FAILURE;
    }

    ext2_super_block *superblock = read_superblock(file);
    if (superblock == NULL) {
        log_error("Failed to read superblock from %s.\n", filename);
        fclose(file);
        return EXIT_FAILURE;
    }

    ext2_group_desc_table *gdt = read_group_descriptor_table(file, superblock);
    if (gdt == NULL) {
        log_error("Failed to read block group descriptors from %s.\n", filename);
        free(superblock);
        fclose(file);
        return EXIT_FAILURE;
    }

    char cmd_line[MAX_CMD_LEN];
    char *cmd_argv[MAX_ARGS];
    int cmd_argc;

    printf("Welcome to the ext2 filesystem shell.\n");
    printf("Available commands: ls [path], exit, quit\n");

    while (1) {
        printf("ext2> ");
        if (fgets(cmd_line, sizeof(cmd_line), stdin) == NULL) {
            break; // End of input
        }

        parse_command(cmd_line, &cmd_argc, cmd_argv);

        if (cmd_argc == 0) {
            continue; // Empty command
        }

        const char *command = cmd_argv[0];

        if (strcmp(command, "ls") == 0) {
            handle_ls(file, superblock, gdt->groups, cmd_argc, cmd_argv);
        } else if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0) {
            printf("Exiting shell.\n");
            break;
        } else {
            log_error("Unknown command: %s\n", command);
        }
    }

    // Cleanup
    free(gdt);
    free(superblock);
    fclose(file);

    return EXIT_SUCCESS;
}

