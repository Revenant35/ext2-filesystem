#ifndef C_EXT2_FILESYSTEM_BITMAP_H
#define C_EXT2_FILESYSTEM_BITMAP_H

#include "Superblock.h"

#include <stdint.h>
#include <stdio.h>

/**
 * @brief Reads a bitmap from a specified block into a buffer.
 *
 * The caller is responsible for providing a buffer of `block_size` bytes.
 *
 * @param file Pointer to the filesystem image file.
 * @param superblock Pointer to the superblock.
 * @param bitmap_block_id The block ID where the bitmap is located.
 * @param bitmap_buffer The buffer to read the bitmap into.
 * @return 0 on success, or a negative error code on failure.
 */
int read_bitmap(
    FILE *file,
    const ext2_super_block *superblock,
    uint32_t bitmap_block_id,
    uint8_t *bitmap_buffer
);

/**
 * @brief Writes a bitmap from a buffer to a specified block on disk.
 *
 * @param file Pointer to the filesystem image file.
 * @param superblock Pointer to the superblock.
 * @param bitmap_block_id The block ID where the bitmap should be written.
 * @param bitmap_buffer The buffer containing the bitmap data to write.
 * @return 0 on success, or a negative error code on failure.
 */
int write_bitmap(
    FILE *file,
    const ext2_super_block *superblock,
    uint32_t bitmap_block_id,
    const uint8_t *bitmap_buffer
);

/**
 * @brief Finds the first free (zero) bit in a bitmap.
 *
 * @param bitmap_buffer The buffer containing the bitmap.
 * @param size_in_bits The total number of bits in the bitmap (e.g., superblock->s_inodes_per_group).
 * @param first_free_bit_index The found index of the first free bit
 * @return 0 on success, or a negative error code on failure
 */
int find_first_free_bit(
    const uint8_t *bitmap_buffer,
    uint32_t size_in_bits,
    uint32_t *first_free_bit_index
);

/**
 * @brief Sets a bit in the bitmap (marks it as used, sets to 1).
 *
 * @param bitmap_buffer The buffer containing the bitmap.
 * @param bit_index The 0-based index of the bit to set.
 */
void set_bit(
    uint8_t *bitmap_buffer,
    uint32_t bit_index
);

/**
 * @brief Clears a bit in the bitmap (marks it as free, sets to 0).
 *
 * @param bitmap_buffer The buffer containing the bitmap.
 * @param bit_index The 0-based index of the bit to clear.
 */
void clear_bit(
    uint8_t *bitmap_buffer,
    uint32_t bit_index
);

#endif //C_EXT2_FILESYSTEM_BITMAP_H
