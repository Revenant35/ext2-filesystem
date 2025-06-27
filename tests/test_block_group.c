#include "../include/block_group.h"
#include "../include/globals.h"
#include "../include/superblock.h"

#include <check.h>
#include <stdio.h>
#include <stdlib.h>

static ext2_super_block *sb;
static ext2_group_desc *gd;

// Helper to create a temporary file with a given size and content
static FILE *create_temp_file(const char *data, const size_t size) {
    FILE *fp = tmpfile();
    if (fp == NULL) {
        return NULL;
    }
    if (data != NULL) {
        fwrite(data, size, 1, fp);
    }
    rewind(fp);
    return fp;
}

void setup(void) {
    sb = malloc(sizeof(ext2_super_block));
    ck_assert_ptr_nonnull(sb);
    sb->s_magic = EXT2_SUPER_MAGIC;
    sb->s_log_block_size = 0; // 1024 bytes
    sb->s_inodes_count = 16384;
    sb->s_blocks_count = 16384;
    sb->s_inodes_per_group = 8192;
    sb->s_blocks_per_group = 8192;

    gd = malloc(sizeof(ext2_group_desc));
    ck_assert_ptr_nonnull(gd);
    gd->bg_block_bitmap = 10;
    gd->bg_inode_bitmap = 11;
    gd->bg_inode_table = 12;
    gd->bg_free_blocks_count = 100;
    gd->bg_free_inodes_count = 200;
    gd->bg_used_dirs_count = 2;
}

void teardown(void) {
    free(sb);
    free(gd);
    sb = NULL;
    gd = NULL;
}

START_TEST(count_block_groups_should_return_correct_count_when_counts_match)
{
    // Arrange is done in setup

    // Act
    const uint32_t count = count_block_groups(sb);

    // Assert
    ck_assert_uint_eq(count, 2);
}
END_TEST

START_TEST(count_block_groups_should_return_block_based_count_when_counts_mismatch)
{
    // Arrange
    sb->s_inodes_count = 24576; // This would imply 3 groups

    // Act
    const uint32_t count = count_block_groups(sb);

    // Assert (should prefer the block-based calculation)
    ck_assert_uint_eq(count, 2);
}
END_TEST

START_TEST(read_group_descriptor_should_return_descriptor_when_file_is_valid)
{
    // Arrange
    const off_t table_offset = (off_t) get_block_size(sb) * 2;
    const size_t buffer_size = table_offset + sizeof(ext2_group_desc);
    char *buffer = calloc(1, buffer_size);
    memcpy(buffer + table_offset, gd, sizeof(ext2_group_desc));
    FILE *fs_image = create_temp_file(buffer, buffer_size);
    ck_assert_ptr_nonnull(fs_image);

    // Act
    ext2_group_desc *read_gd = read_group_descriptor(fs_image, sb, 0);

    // Assert
    ck_assert_ptr_nonnull(read_gd);
    ck_assert_uint_eq(read_gd->bg_block_bitmap, gd->bg_block_bitmap);
    ck_assert_uint_eq(read_gd->bg_inode_table, gd->bg_inode_table);

    // Cleanup
    free(buffer);
    free(read_gd);
    fclose(fs_image);
}
END_TEST

START_TEST(read_group_descriptor_should_return_null_when_file_is_null)
{
    // Act
    ext2_group_desc *read_gd = read_group_descriptor(NULL, sb, 0);

    // Assert
    ck_assert_ptr_null(read_gd);
}
END_TEST

START_TEST(write_group_descriptor_should_return_success_when_data_is_valid)
{
    // Arrange
    FILE *fs_image = create_temp_file(NULL, 0);
    ck_assert_ptr_nonnull(fs_image);

    // Act
    const int result = write_group_descriptor(fs_image, sb, 0, gd);

    // Assert
    ck_assert_int_eq(result, SUCCESS);

    // Verify by reading back
    const off_t offset = (off_t) get_block_size(sb) * 2;
    fseeko(fs_image, offset, SEEK_SET);
    ext2_group_desc verify_gd;
    fread(&verify_gd, sizeof(ext2_group_desc), 1, fs_image);
    ck_assert_uint_eq(verify_gd.bg_block_bitmap, gd->bg_block_bitmap);

    // Cleanup
    fclose(fs_image);
}
END_TEST

START_TEST(read_group_descriptor_table_should_return_table_when_file_is_valid)
{
    // Arrange
    const uint32_t num_groups = count_block_groups(sb);
    const off_t table_offset = (off_t) get_block_size(sb) * 2;
    const size_t table_size = num_groups * sizeof(ext2_group_desc);
    const size_t buffer_size = table_offset + table_size;
    char *buffer = calloc(1, buffer_size);

    // Populate the buffer with two group descriptors
    gd->bg_inode_table = 12; // Group 0
    memcpy(buffer + table_offset, gd, sizeof(ext2_group_desc));
    gd->bg_inode_table = 22; // Group 1
    memcpy(buffer + table_offset + sizeof(ext2_group_desc), gd, sizeof(ext2_group_desc));

    FILE *fs_image = create_temp_file(buffer, buffer_size);
    ck_assert_ptr_nonnull(fs_image);

    // Act
    ext2_group_desc_table *table = read_group_descriptor_table(fs_image, sb);

    // Assert
    ck_assert_ptr_nonnull(table);
    ck_assert_uint_eq(table->groups_count, num_groups);
    ck_assert_ptr_nonnull(table->groups);
    ck_assert_uint_eq(table->groups[0].bg_inode_table, 12);
    ck_assert_uint_eq(table->groups[1].bg_inode_table, 22);

    // Cleanup
    free(buffer);
    free(table->groups);
    free(table);
    fclose(fs_image);
}
END_TEST

Suite *block_group_suite(void)
{
    Suite *s = suite_create("BlockGroup");
    TCase *tc_core = tcase_create("Core");

    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, count_block_groups_should_return_correct_count_when_counts_match);
    tcase_add_test(tc_core, count_block_groups_should_return_block_based_count_when_counts_mismatch);
    tcase_add_test(tc_core, read_group_descriptor_should_return_descriptor_when_file_is_valid);
    tcase_add_test(tc_core, read_group_descriptor_should_return_null_when_file_is_null);
    tcase_add_test(tc_core, write_group_descriptor_should_return_success_when_data_is_valid);
    tcase_add_test(tc_core, read_group_descriptor_table_should_return_table_when_file_is_valid);

    suite_add_tcase(s, tc_core);
    return s;
}

int main(void)
{
    Suite *s = block_group_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    const int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
