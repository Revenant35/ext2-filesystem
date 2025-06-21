/**
 * @file Bitmap.c
 * @brief Implements functions for reading, writing, and manipulating ext2 bitmaps.
 */

#include "Bitmap.h"
#include "Superblock.h"
#include "BlockGroup.h"
#include "globals.h"

#include <stdio.h>

int read_bitmap(
    FILE *file,
    const struct ext2_super_block *superblock,
    const uint32_t bitmap_block_id,
    uint8_t *bitmap_buffer
) {
    const uint32_t block_size = get_block_size(superblock);
    const off_t offset = (off_t) bitmap_block_id * block_size;

    if (fseeko(file, offset, SEEK_SET) != 0) {
        perror("read_bitmap: fseeko");
        return IO_ERROR;
    }

    if (fread(bitmap_buffer, block_size, 1, file) != 1) {
        perror("read_bitmap: fread");
        return IO_ERROR;
    }

    return SUCCESS;
}

int write_bitmap(
    FILE *file,
    const struct ext2_super_block *superblock,
    const uint32_t bitmap_block_id,
    const uint8_t *bitmap_buffer
) {
    const uint32_t block_size = get_block_size(superblock);
    const off_t offset = (off_t) bitmap_block_id * block_size;

    if (fseeko(file, offset, SEEK_SET) != 0) {
        perror("write_bitmap: fseeko");
        return IO_ERROR;
    }

    if (fwrite(bitmap_buffer, block_size, 1, file) != 1) {
        perror("write_bitmap: fwrite");
        return IO_ERROR;
    }

    return SUCCESS;
}

int find_first_free_bit(
    const uint8_t *bitmap_buffer,
    const uint32_t size_in_bits,
    uint32_t *first_free_bit_index
) {
    const uint32_t size_in_bytes = (size_in_bits + 7) / 8;
    for (uint32_t byte_idx = 0; byte_idx < size_in_bytes; ++byte_idx) {
        if (bitmap_buffer[byte_idx] == 0xFF) continue;

        // If byte is not all 1s, there's a 0 bit
        for (int bit_idx = 0; bit_idx < 8; ++bit_idx) {
            const uint32_t current_bit = byte_idx * 8 + bit_idx;
            if (current_bit >= size_in_bits || bitmap_buffer[byte_idx] >> bit_idx & 1) continue;
            *first_free_bit_index = current_bit;
            return SUCCESS;
        }
    }
    return ERROR;
}

void set_bit(
    uint8_t *bitmap_buffer,
    const uint32_t bit_index
) {
    const uint32_t byte_idx = bit_index / 8;
    const uint8_t bit_idx_in_byte = bit_index % 8;
    bitmap_buffer[byte_idx] |= 1 << bit_idx_in_byte;
}

void clear_bit(
    uint8_t *bitmap_buffer,
    const uint32_t bit_index
) {
    const uint32_t byte_idx = bit_index / 8;
    const uint8_t bit_idx_in_byte = bit_index % 8;
    bitmap_buffer[byte_idx] &= ~(1 << bit_idx_in_byte);
}
