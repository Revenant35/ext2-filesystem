#include "../include/superblock.h"
#include "../include/globals.h"

#include <check.h>
#include <stdio.h>
#include <stdlib.h>

static ext2_super_block *sb;

// Helper to create a temporary file with a given size and content
static FILE *create_temp_file(const char *data, size_t size) {
    FILE *fp = tmpfile();
    if (fp == NULL) {
        return NULL;
    }
    if (data != NULL && size > 0) {
        fwrite(data, 1, size, fp);
    }
    rewind(fp);
    return fp;
}

void setup(void) {
    sb = malloc(sizeof(ext2_super_block));
    ck_assert_ptr_nonnull(sb);
    // Initialize with some default valid values
    sb->s_magic = EXT2_SUPER_MAGIC;
    sb->s_log_block_size = 0; // 1024 bytes
    sb->s_log_frag_size = 0;  // 1024 bytes
    sb->s_blocks_count = 1000;
    sb->s_blocks_per_group = 8192;
}

void teardown(void) {
    free(sb);
    sb = NULL;
}

START_TEST(test_get_block_size_valid)
{
    sb->s_log_block_size = 0; // 1024 << 0 = 1024
    ck_assert_uint_eq(get_block_size(sb), 1024);

    sb->s_log_block_size = 1; // 1024 << 1 = 2048
    ck_assert_uint_eq(get_block_size(sb), 2048);

    sb->s_log_block_size = 2; // 1024 << 2 = 4096
    ck_assert_uint_eq(get_block_size(sb), 4096);
}
END_TEST

START_TEST(test_get_block_size_null)
{
    ck_assert_uint_eq(get_block_size(NULL), 0);
}
END_TEST

START_TEST(test_get_fragment_size_valid)
{
    sb->s_log_frag_size = 0; // 1024 << 0 = 1024
    ck_assert_uint_eq(get_fragment_size(sb), 1024);

    sb->s_log_frag_size = 1; // 1024 << 1 = 2048
    ck_assert_uint_eq(get_fragment_size(sb), 2048);

    sb->s_log_frag_size = 2; // 1024 << 2 = 4096
    ck_assert_uint_eq(get_fragment_size(sb), 4096);
}
END_TEST

START_TEST(test_get_fragment_size_null)
{
    ck_assert_uint_eq(get_fragment_size(NULL), 0);
}
END_TEST

START_TEST(test_get_block_group_count_valid)
{
    sb->s_blocks_count = 8192;
    sb->s_blocks_per_group = 8192;
    ck_assert_uint_eq(get_block_group_count(sb), 1);

    sb->s_blocks_count = 8193;
    sb->s_blocks_per_group = 8192;
    ck_assert_uint_eq(get_block_group_count(sb), 2);

    sb->s_blocks_count = 16384;
    sb->s_blocks_per_group = 8192;
    ck_assert_uint_eq(get_block_group_count(sb), 2);

    sb->s_blocks_count = 1;
    sb->s_blocks_per_group = 8192;
    ck_assert_uint_eq(get_block_group_count(sb), 1);
}
END_TEST

START_TEST(test_get_block_group_count_zero_blocks_per_group)
{
    sb->s_blocks_per_group = 0;
    ck_assert_uint_eq(get_block_group_count(sb), 0);
}
END_TEST

START_TEST(test_get_block_group_count_null)
{
    ck_assert_uint_eq(get_block_group_count(NULL), 0);
}
END_TEST

START_TEST(test_read_superblock_valid)
{
    char buffer[EXT2_SUPERBLOCK_OFFSET + sizeof(ext2_super_block)] = {0};
    sb->s_inodes_count = 12345; // Set a unique value to check
    memcpy(buffer + EXT2_SUPERBLOCK_OFFSET, sb, sizeof(ext2_super_block));

    FILE *fs_image = create_temp_file(buffer, sizeof(buffer));
    ck_assert_ptr_nonnull(fs_image);

    ext2_super_block *read_sb = read_superblock(fs_image);
    ck_assert_ptr_nonnull(read_sb);
    ck_assert_uint_eq(read_sb->s_magic, EXT2_SUPER_MAGIC);
    ck_assert_uint_eq(read_sb->s_inodes_count, 12345);

    free(read_sb);
    fclose(fs_image);
}
END_TEST

START_TEST(test_read_superblock_null_file)
{
    ext2_super_block *read_sb = read_superblock(NULL);
    ck_assert_ptr_null(read_sb);
}
END_TEST

START_TEST(test_read_superblock_bad_magic)
{
    sb->s_magic = 0; // Invalid magic number
    char buffer[EXT2_SUPERBLOCK_OFFSET + sizeof(ext2_super_block)] = {0};
    memcpy(buffer + EXT2_SUPERBLOCK_OFFSET, sb, sizeof(ext2_super_block));

    FILE *fs_image = create_temp_file(buffer, sizeof(buffer));
    ck_assert_ptr_nonnull(fs_image);

    ext2_super_block *read_sb = read_superblock(fs_image);
    ck_assert_ptr_null(read_sb);

    fclose(fs_image);
}
END_TEST

START_TEST(test_read_superblock_short_file)
{
    // File is too short to contain a full superblock
    FILE *fs_image = create_temp_file("short", 5);
    ck_assert_ptr_nonnull(fs_image);

    ext2_super_block *read_sb = read_superblock(fs_image);
    ck_assert_ptr_null(read_sb);

    fclose(fs_image);
}
END_TEST

START_TEST(test_write_superblock_valid)
{
    FILE *fs_image = create_temp_file(NULL, 0);
    ck_assert_ptr_nonnull(fs_image);

    int result = write_superblock(fs_image, sb);
    ck_assert_int_eq(result, SUCCESS);

    // Read back and verify
    fseek(fs_image, EXT2_SUPERBLOCK_OFFSET, SEEK_SET);
    ext2_super_block *verify_sb = malloc(sizeof(ext2_super_block));
    fread(verify_sb, sizeof(ext2_super_block), 1, fs_image);

    ck_assert_uint_eq(verify_sb->s_magic, sb->s_magic);
    ck_assert_uint_eq(verify_sb->s_blocks_count, sb->s_blocks_count);

    free(verify_sb);
    fclose(fs_image);
}
END_TEST

START_TEST(test_write_superblock_null_args)
{
    FILE *fs_image = create_temp_file(NULL, 0);
    ck_assert_ptr_nonnull(fs_image);

    ck_assert_int_eq(write_superblock(NULL, sb), INVALID_PARAMETER);
    ck_assert_int_eq(write_superblock(fs_image, NULL), INVALID_PARAMETER);
    ck_assert_int_eq(write_superblock(NULL, NULL), INVALID_PARAMETER);

    fclose(fs_image);
}
END_TEST

START_TEST(test_write_superblock_bad_magic)
{
    FILE *fs_image = create_temp_file(NULL, 0);
    ck_assert_ptr_nonnull(fs_image);

    sb->s_magic = 0; // Invalid magic
    int result = write_superblock(fs_image, sb);
    ck_assert_int_eq(result, ERROR);

    fclose(fs_image);
}
END_TEST


Suite *superblock_suite(void)
{
    Suite *s = suite_create("Superblock");
    TCase *tc_core = tcase_create("Core");

    tcase_add_checked_fixture(tc_core, setup, teardown);

    // Getter tests
    tcase_add_test(tc_core, test_get_block_size_valid);
    tcase_add_test(tc_core, test_get_block_size_null);
    tcase_add_test(tc_core, test_get_fragment_size_valid);
    tcase_add_test(tc_core, test_get_fragment_size_null);
    tcase_add_test(tc_core, test_get_block_group_count_valid);
    tcase_add_test(tc_core, test_get_block_group_count_zero_blocks_per_group);
    tcase_add_test(tc_core, test_get_block_group_count_null);

    // Read tests
    tcase_add_test(tc_core, test_read_superblock_valid);
    tcase_add_test(tc_core, test_read_superblock_null_file);
    tcase_add_test(tc_core, test_read_superblock_bad_magic);
    tcase_add_test(tc_core, test_read_superblock_short_file);

    // Write tests
    tcase_add_test(tc_core, test_write_superblock_valid);
    tcase_add_test(tc_core, test_write_superblock_null_args);
    tcase_add_test(tc_core, test_write_superblock_bad_magic);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    Suite *s = superblock_suite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    const int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
