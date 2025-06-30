#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdio.h>

#include "types.h"

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
