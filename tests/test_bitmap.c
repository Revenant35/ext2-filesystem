#include "../include/bitmap.h"
#include "../include/globals.h"
#include "../include/superblock.h"

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BITMAP_SIZE 1024 // In bytes, matching a block size

static uint8_t *bitmap_buffer;
static ext2_super_block *sb;

void setup(void) {
    bitmap_buffer = calloc(BITMAP_SIZE, sizeof(uint8_t));
    ck_assert_ptr_nonnull(bitmap_buffer);

    sb = malloc(sizeof(ext2_super_block));
    ck_assert_ptr_nonnull(sb);
    sb->s_log_block_size = 0; // 1024 bytes
}

void teardown(void) {
    free(bitmap_buffer);
    free(sb);
}

START_TEST(set_bit_should_set_the_correct_bit)
{
    // Act
    set_bit(bitmap_buffer, 10);

    // Assert
    ck_assert_uint_eq(bitmap_buffer[1], 1 << 2);
}
END_TEST

START_TEST(clear_bit_should_clear_the_correct_bit)
{
    // Arrange
    memset(bitmap_buffer, 0xFF, BITMAP_SIZE);

    // Act
    clear_bit(bitmap_buffer, 10);

    // Assert
    ck_assert_uint_eq(bitmap_buffer[1], (uint8_t)~(1 << 2));
}
END_TEST

START_TEST(find_first_free_bit_should_find_the_correct_bit)
{
    // Arrange
    set_bit(bitmap_buffer, 0);
    set_bit(bitmap_buffer, 1);

    // Act
    uint32_t free_bit_index;
    int result = find_first_free_bit(bitmap_buffer, BITMAP_SIZE * 8, &free_bit_index);

    // Assert
    ck_assert_int_eq(result, SUCCESS);
    ck_assert_uint_eq(free_bit_index, 2);
}
END_TEST

START_TEST(find_first_free_bit_should_return_error_when_bitmap_is_full)
{
    // Arrange
    memset(bitmap_buffer, 0xFF, BITMAP_SIZE);

    // Act
    uint32_t free_bit_index;
    int result = find_first_free_bit(bitmap_buffer, BITMAP_SIZE * 8, &free_bit_index);

    // Assert
    ck_assert_int_eq(result, ERROR);
}
END_TEST

START_TEST(write_and_read_bitmap_should_preserve_data)
{
    // Arrange
    FILE *fs_image = tmpfile();
    ck_assert_ptr_nonnull(fs_image);
    set_bit(bitmap_buffer, 5);
    const uint32_t bitmap_block_id = 3;

    // Act
    int write_result = write_bitmap(fs_image, sb, bitmap_block_id, bitmap_buffer);
    ck_assert_int_eq(write_result, SUCCESS);

    uint8_t *read_buffer = calloc(BITMAP_SIZE, sizeof(uint8_t));
    int read_result = read_bitmap(fs_image, sb, bitmap_block_id, read_buffer);

    // Assert
    ck_assert_int_eq(read_result, SUCCESS);
    ck_assert_mem_eq(bitmap_buffer, read_buffer, BITMAP_SIZE);

    // Cleanup
    free(read_buffer);
    fclose(fs_image);
}
END_TEST

Suite *bitmap_suite(void) {
    Suite *s = suite_create("Bitmap");
    TCase *tc_core = tcase_create("Core");

    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, set_bit_should_set_the_correct_bit);
    tcase_add_test(tc_core, clear_bit_should_clear_the_correct_bit);
    tcase_add_test(tc_core, find_first_free_bit_should_find_the_correct_bit);
    tcase_add_test(tc_core, find_first_free_bit_should_return_error_when_bitmap_is_full);
    tcase_add_test(tc_core, write_and_read_bitmap_should_preserve_data);

    suite_add_tcase(s, tc_core);
    return s;
}

int main(void) {
    Suite *s = bitmap_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    const int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
