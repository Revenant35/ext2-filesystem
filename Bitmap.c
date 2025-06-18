/**
 * @file Bitmap.c
 * @brief Implements functions for reading, writing, and manipulating ext2 bitmaps.
 */

#include "Bitmap.h"
#include "Superblock.h"
#include "BlockGroup.h"
#include "globals.h"

#include <stdio.h>

int read_bitmap(FILE *fp, const struct ext2_super_block *sb, uint32_t bitmap_block_id, char *bitmap_buffer) {
    const uint32_t block_size = get_block_size(sb);
    const off_t offset = (off_t)bitmap_block_id * block_size;

    if (fseeko(fp, offset, SEEK_SET) != 0) {
        perror("read_bitmap: fseeko");
        return -2;
    }
    if (fread(bitmap_buffer, block_size, 1, fp) != 1) {
        perror("read_bitmap: fread");
        return -2;
    }
    return 0;
}

int write_bitmap(FILE *fp, const struct ext2_super_block *sb, uint32_t bitmap_block_id, const char *bitmap_buffer) {
    const uint32_t block_size = get_block_size(sb);
    const off_t offset = (off_t)bitmap_block_id * block_size;

    if (fseeko(fp, offset, SEEK_SET) != 0) {
        perror("write_bitmap: fseeko");
        return -1;
    }
    if (fwrite(bitmap_buffer, block_size, 1, fp) != 1) {
        perror("write_bitmap: fwrite");
        return -2;
    }
    return 0;
}

uint32_t find_first_free_bit(const char *bitmap_buffer, uint32_t size_in_bits) {
    const uint32_t size_in_bytes = (size_in_bits + 7) / 8;
    for (uint32_t byte_idx = 0; byte_idx < size_in_bytes; ++byte_idx) {
        if ((unsigned char)bitmap_buffer[byte_idx] != 0xFF) { // If byte is not all 1s, there's a 0 bit
            for (int bit_idx = 0; bit_idx < 8; ++bit_idx) {
                const uint32_t current_bit = byte_idx * 8 + bit_idx;
                if (current_bit < size_in_bits) {
                    if (!((bitmap_buffer[byte_idx] >> bit_idx) & 1)) {
                        return current_bit; // Found a free bit
                    }
                }
            }
        }
    }
    return INVALID_PARAMETER; // No free bit found
}

void set_bit(char *bitmap_buffer, const uint32_t bit_index) {
    const uint32_t byte_idx = bit_index / 8;
    const uint8_t bit_idx_in_byte = bit_index % 8;
    bitmap_buffer[byte_idx] |= 1 << bit_idx_in_byte;
}

void clear_bit(char *bitmap_buffer, const uint32_t bit_index) {
    const uint32_t byte_idx = bit_index / 8;
    const uint8_t bit_idx_in_byte = bit_index % 8;
    bitmap_buffer[byte_idx] &= ~(1 << bit_idx_in_byte);
}
