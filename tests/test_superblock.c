#include "../include/superblock.h"
#include "../include/globals.h"

#include <check.h>
#include <stdio.h>
#include <stdlib.h>

static ext2_super_block *sb;

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
    sb->s_log_frag_size = 0;  // 1024 bytes
    sb->s_blocks_count = 1000;
    sb->s_blocks_per_group = 8192;
}

void teardown(void) {
    free(sb);
    sb = NULL;
}

START_TEST(get_block_size_should_return_correct_size_when_log_block_size_is_valid)
{
    // Arrange
    sb->s_log_block_size = 1; // 1024 << 1 = 2048

    // Act
    const uint32_t block_size = get_block_size(sb);

    // Assert
    ck_assert_uint_eq(block_size, 2048);
}
END_TEST

START_TEST(get_block_size_should_return_zero_when_superblock_is_null)
{
    // Act
    const uint32_t block_size = get_block_size(NULL);

    // Assert
    ck_assert_uint_eq(block_size, 0);
}
END_TEST

START_TEST(get_fragment_size_should_return_correct_size_when_log_frag_size_is_valid)
{
    // Arrange
    sb->s_log_frag_size = 2; // 1024 << 2 = 4096

    // Act
    const uint32_t frag_size = get_fragment_size(sb);

    // Assert
    ck_assert_uint_eq(frag_size, 4096);
}
END_TEST

START_TEST(get_fragment_size_should_return_zero_when_superblock_is_null)
{
    // Arrange
    // No arrangement needed

    // Act
    const uint32_t frag_size = get_fragment_size(NULL);

    // Assert
    ck_assert_uint_eq(frag_size, 0);
}
END_TEST

START_TEST(get_block_group_count_should_return_correct_count_when_division_is_exact)
{
    // Arrange
    sb->s_blocks_count = 16384;
    sb->s_blocks_per_group = 8192;

    // Act
    const uint32_t count = get_block_group_count(sb);

    // Assert
    ck_assert_uint_eq(count, 2);
}
END_TEST

START_TEST(get_block_group_count_should_return_correct_count_when_division_has_remainder)
{
    // Arrange
    sb->s_blocks_count = 16385;
    sb->s_blocks_per_group = 8192;

    // Act
    const uint32_t count = get_block_group_count(sb);

    // Assert
    ck_assert_uint_eq(count, 3);
}
END_TEST

START_TEST(get_block_group_count_should_return_zero_when_blocks_per_group_is_zero)
{
    // Arrange
    sb->s_blocks_per_group = 0;

    // Act
    const uint32_t count = get_block_group_count(sb);

    // Assert
    ck_assert_uint_eq(count, 0);
}
END_TEST

START_TEST(get_block_group_count_should_return_zero_when_superblock_is_null)
{
    // Arrange
    // No arrangement needed

    // Act
    const uint32_t count = get_block_group_count(NULL);

    // Assert
    ck_assert_uint_eq(count, 0);
}
END_TEST

START_TEST(read_superblock_should_return_superblock_when_file_is_valid)
{
    // Arrange
    char buffer[EXT2_SUPERBLOCK_OFFSET + sizeof(ext2_super_block)] = {0};
    sb->s_inodes_count = 54321;
    memcpy(buffer + EXT2_SUPERBLOCK_OFFSET, sb, sizeof(ext2_super_block));
    FILE *fs_image = create_temp_file(buffer, sizeof(buffer));
    ck_assert_ptr_nonnull(fs_image);

    // Act
    ext2_super_block *read_sb = read_superblock(fs_image);

    // Assert
    ck_assert_ptr_nonnull(read_sb);
    ck_assert_uint_eq(read_sb->s_magic, EXT2_SUPER_MAGIC);
    ck_assert_uint_eq(read_sb->s_inodes_count, 54321);

    // Cleanup
    free(read_sb);
    fclose(fs_image);
}
END_TEST

START_TEST(read_superblock_should_return_null_when_file_is_null)
{
    // Arrange
    // No arrangement needed

    // Act
    const ext2_super_block *read_sb = read_superblock(NULL);

    // Assert
    ck_assert_ptr_null(read_sb);
}
END_TEST

START_TEST(read_superblock_should_return_null_when_magic_is_invalid)
{
    // Arrange
    sb->s_magic = 0; // Invalid magic number
    char buffer[EXT2_SUPERBLOCK_OFFSET + sizeof(ext2_super_block)] = {0};
    memcpy(buffer + EXT2_SUPERBLOCK_OFFSET, sb, sizeof(ext2_super_block));
    FILE *fs_image = create_temp_file(buffer, sizeof(buffer));
    ck_assert_ptr_nonnull(fs_image);

    // Act
    const ext2_super_block *read_sb = read_superblock(fs_image);

    // Assert
    ck_assert_ptr_null(read_sb);

    // Cleanup
    fclose(fs_image);
}
END_TEST

START_TEST(read_superblock_should_return_null_when_file_is_too_short)
{
    // Arrange
    FILE *fs_image = create_temp_file("short", 5);
    ck_assert_ptr_nonnull(fs_image);

    // Act
    const ext2_super_block *read_sb = read_superblock(fs_image);

    // Assert
    ck_assert_ptr_null(read_sb);

    // Cleanup
    fclose(fs_image);
}
END_TEST

START_TEST(write_superblock_should_return_success_when_superblock_is_valid)
{
    // Arrange
    FILE *fs_image = create_temp_file(NULL, 0);
    ck_assert_ptr_nonnull(fs_image);

    // Act
    const int result = write_superblock(fs_image, sb);

    // Assert
    ck_assert_int_eq(result, SUCCESS);
    fseek(fs_image, EXT2_SUPERBLOCK_OFFSET, SEEK_SET);
    ext2_super_block *verify_sb = malloc(sizeof(ext2_super_block));
    fread(verify_sb, sizeof(ext2_super_block), 1, fs_image);
    ck_assert_uint_eq(verify_sb->s_magic, sb->s_magic);
    ck_assert_uint_eq(verify_sb->s_blocks_count, sb->s_blocks_count);

    // Cleanup
    free(verify_sb);
    fclose(fs_image);
}
END_TEST

START_TEST(write_superblock_should_return_invalid_parameter_when_args_are_null)
{
    // Arrange
    FILE *fs_image = create_temp_file(NULL, 0);
    ck_assert_ptr_nonnull(fs_image);

    // Act & Assert
    ck_assert_int_eq(write_superblock(NULL, sb), INVALID_PARAMETER);
    ck_assert_int_eq(write_superblock(fs_image, NULL), INVALID_PARAMETER);
    ck_assert_int_eq(write_superblock(NULL, NULL), INVALID_PARAMETER);

    // Cleanup
    fclose(fs_image);
}
END_TEST

START_TEST(write_superblock_should_return_error_when_magic_is_invalid)
{
    // Arrange
    FILE *fs_image = create_temp_file(NULL, 0);
    ck_assert_ptr_nonnull(fs_image);
    sb->s_magic = 0; // Invalid magic

    // Act
    const int result = write_superblock(fs_image, sb);

    // Assert
    ck_assert_int_eq(result, ERROR);

    // Cleanup
    fclose(fs_image);
}
END_TEST

Suite *superblock_suite(void)
{
    Suite *s = suite_create("Superblock");
    TCase *tc_core = tcase_create("Core");

    tcase_add_checked_fixture(tc_core, setup, teardown);

    // Getter tests
    tcase_add_test(tc_core, get_block_size_should_return_correct_size_when_log_block_size_is_valid);
    tcase_add_test(tc_core, get_block_size_should_return_zero_when_superblock_is_null);
    tcase_add_test(tc_core, get_fragment_size_should_return_correct_size_when_log_frag_size_is_valid);
    tcase_add_test(tc_core, get_fragment_size_should_return_zero_when_superblock_is_null);
    tcase_add_test(tc_core, get_block_group_count_should_return_correct_count_when_division_is_exact);
    tcase_add_test(tc_core, get_block_group_count_should_return_correct_count_when_division_has_remainder);
    tcase_add_test(tc_core, get_block_group_count_should_return_zero_when_blocks_per_group_is_zero);
    tcase_add_test(tc_core, get_block_group_count_should_return_zero_when_superblock_is_null);

    // Read tests
    tcase_add_test(tc_core, read_superblock_should_return_superblock_when_file_is_valid);
    tcase_add_test(tc_core, read_superblock_should_return_null_when_file_is_null);
    tcase_add_test(tc_core, read_superblock_should_return_null_when_magic_is_invalid);
    tcase_add_test(tc_core, read_superblock_should_return_null_when_file_is_too_short);

    // Write tests
    tcase_add_test(tc_core, write_superblock_should_return_success_when_superblock_is_valid);
    tcase_add_test(tc_core, write_superblock_should_return_invalid_parameter_when_args_are_null);
    tcase_add_test(tc_core, write_superblock_should_return_error_when_magic_is_invalid);

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
