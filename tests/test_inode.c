#include "inode.h"
#include "globals.h"
#include "superblock.h"
#include "block_group.h"

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mock data
static ext2_super_block *sb;
static ext2_group_desc *bgdt; // Block Group Descriptor Table
static ext2_inode *test_inode;

// Helper to create a temporary file and write data to it
static FILE *create_temp_fs_image(const char *data, size_t size) {
    FILE *fp = tmpfile();
    if (fp == NULL) {
        ck_abort_msg("Failed to create temporary file");
    }
    if (data != NULL && size > 0) {
        if (fwrite(data, 1, size, fp) != size) {
            ck_abort_msg("Failed to write to temporary file");
        }
    }
    rewind(fp);
    return fp;
}

void setup(void) {
    // Allocate and initialize a mock superblock
    sb = malloc(sizeof(ext2_super_block));
    ck_assert_ptr_nonnull(sb);
    sb->s_inodes_count = 32;
    sb->s_blocks_count = 32;
    sb->s_inodes_per_group = 16;
    sb->s_blocks_per_group = 16;
    sb->s_inode_size = sizeof(ext2_inode);
    sb->s_log_block_size = 0; // 1024 bytes block size

    // Allocate and initialize a mock block group descriptor table (2 groups)
    bgdt = malloc(2 * sizeof(ext2_group_desc));
    ck_assert_ptr_nonnull(bgdt);
    bgdt[0].bg_inode_table = 10; // Inode table for group 0 starts at block 10
    bgdt[1].bg_inode_table = 20; // Inode table for group 1 starts at block 20

    // Allocate and initialize a sample inode
    test_inode = malloc(sizeof(ext2_inode));
    ck_assert_ptr_nonnull(test_inode);
    test_inode->i_mode = 0x81A4; // S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
    test_inode->i_size = 4096;
    test_inode->i_links_count = 1;
    test_inode->i_blocks = 8;
}

void teardown(void) {
    free(sb);
    free(bgdt);
    free(test_inode);
}

START_TEST(read_inode_should_return_success_when_inode_is_valid)
{
    // Arrange: inode 17 is the first inode in the second block group (index 1)
    const uint32_t inode_num = 17;
    const off_t inode_table_offset = (off_t)bgdt[1].bg_inode_table * get_block_size(sb);
    const size_t file_size = inode_table_offset + sizeof(ext2_inode);
    char *buffer = calloc(1, file_size);
    memcpy(buffer + inode_table_offset, test_inode, sizeof(ext2_inode));
    FILE *fs_image = create_temp_fs_image(buffer, file_size);
    ext2_inode result_inode;

    // Act
    const int result = read_inode(fs_image, sb, bgdt, inode_num, &result_inode);

    // Assert
    ck_assert_int_eq(result, SUCCESS);
    ck_assert_uint_eq(result_inode.i_mode, test_inode->i_mode);
    ck_assert_uint_eq(result_inode.i_size, test_inode->i_size);
    ck_assert_uint_eq(result_inode.i_links_count, test_inode->i_links_count);

    // Cleanup
    free(buffer);
    fclose(fs_image);
}
END_TEST

START_TEST(read_inode_should_return_error_for_invalid_inode_number)
{
    // Arrange
    FILE *fs_image = create_temp_fs_image(NULL, 0);
    ext2_inode result_inode;

    // Act & Assert
    ck_assert_int_ne(read_inode(fs_image, sb, bgdt, 0, &result_inode), SUCCESS); // Inode 0 is invalid
    ck_assert_int_ne(read_inode(fs_image, sb, bgdt, 33, &result_inode), SUCCESS); // Inode 33 > s_inodes_count

    // Cleanup
    fclose(fs_image);
}
END_TEST

START_TEST(read_inode_should_return_error_for_null_parameters)
{
    // Arrange
    FILE *fs_image = create_temp_fs_image(NULL, 0);
    ext2_inode result_inode;

    // Act & Assert
    ck_assert_int_eq(read_inode(NULL, sb, bgdt, 1, &result_inode), INVALID_PARAMETER);
    ck_assert_int_eq(read_inode(fs_image, NULL, bgdt, 1, &result_inode), INVALID_PARAMETER);
    ck_assert_int_eq(read_inode(fs_image, sb, NULL, 1, &result_inode), INVALID_PARAMETER);
    ck_assert_int_eq(read_inode(fs_image, sb, bgdt, 1, NULL), INVALID_PARAMETER);

    // Cleanup
    fclose(fs_image);
}
END_TEST

START_TEST(write_inode_should_return_success_and_write_data_correctly)
{
    // Arrange
    const uint32_t inode_num = 17;
    const off_t inode_table_offset = (off_t)bgdt[1].bg_inode_table * get_block_size(sb);
    const size_t file_size = inode_table_offset + sizeof(ext2_inode);
    FILE *fs_image = create_temp_fs_image(NULL, file_size);

    // Act
    const int result = write_inode(fs_image, sb, bgdt, inode_num, test_inode);

    // Assert
    ck_assert_int_eq(result, SUCCESS);

    // Verify by reading back the data
    ext2_inode verify_inode;
    fseeko(fs_image, inode_table_offset, SEEK_SET);
    fread(&verify_inode, sizeof(ext2_inode), 1, fs_image);
    ck_assert_uint_eq(verify_inode.i_mode, test_inode->i_mode);
    ck_assert_uint_eq(verify_inode.i_size, test_inode->i_size);

    // Cleanup
    fclose(fs_image);
}
END_TEST

Suite *inode_suite(void) {
    Suite *s = suite_create("Inode");
    TCase *tc_core = tcase_create("Core");

    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, read_inode_should_return_success_when_inode_is_valid);
    tcase_add_test(tc_core, read_inode_should_return_error_for_invalid_inode_number);
    tcase_add_test(tc_core, read_inode_should_return_error_for_null_parameters);
    tcase_add_test(tc_core, write_inode_should_return_success_and_write_data_correctly);

    suite_add_tcase(s, tc_core);
    return s;
}

int main(void) {
    Suite *s = inode_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    const int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
