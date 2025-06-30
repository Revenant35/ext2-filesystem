#include "filesystem.h"
#include "superblock.h"
#include "block_group.h"
#include "globals.h"

#include <stdlib.h>
#include <string.h>

ext2_filesystem *filesystem_init(FILE *device) {
    if (device == NULL) {
        log_error("device cannot be NULL.\n");
        return NULL;
    }

    ext2_filesystem *fs = malloc(sizeof(ext2_filesystem));
    if (fs == NULL) {
        log_error("Failed to allocate memory for filesystem context.\n");
        return NULL;
    }

    memset(fs, 0, sizeof(ext2_filesystem));

    ext2_super_block *superblock = read_superblock(device);
    if (superblock == NULL) {
        log_error("Failed to read superblock.\n");
        free(fs);
        return NULL;
    }

    ext2_group_desc_table *bgdt = read_group_descriptor_table(device, superblock);
    if (fs->bgdt == NULL) {
        log_error("Failed to read block group descriptor table.\n");
        free(superblock);
        free(fs);
        return NULL;
    }

    fs->device = device;
    fs->superblock = superblock;
    fs->bgdt = bgdt;

    return fs;
}

void filesystem_free(ext2_filesystem *fs) {
    if (fs == NULL) {
        return;
    }

    if (fs->superblock) {
        free(fs->superblock);
    }
    if (fs->bgdt) {
        if (fs->bgdt->groups) {
            free(fs->bgdt->groups);
        }
        free(fs->bgdt);
    }
    if (fs->device) {
        fclose(fs->device);
    }
    free(fs);
}
