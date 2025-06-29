#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "superblock.h"
#include "block_group.h"
#include <stdio.h>

/**
 * @brief Represents the entire state of a mounted ext2 filesystem.
 *
 * This structure encapsulates the file pointer to the device, the superblock,
 * and the block group descriptor table, providing a single context object
 * for all filesystem operations.
 */
typedef struct {
    FILE *device;
    ext2_super_block *superblock;
    ext2_group_desc_table *bgdt;
} ext2_filesystem;

/**
 * @brief Initializes the filesystem context.
 *
 * Opens the specified device, reads the superblock and the block group
 * descriptor table, and returns a new filesystem context object.
 *
 * @param device_path The path to the filesystem image or device.
 * @return A pointer to a new ext2_filesystem object on success, or NULL on failure.
 */
ext2_filesystem *filesystem_init(FILE *device_path);

/**
 * @brief Frees all resources associated with a filesystem context.
 *
 * @param fs A pointer to the ext2_filesystem object to be freed.
 */
void filesystem_free(ext2_filesystem *fs);

#endif // FILESYSTEM_H
