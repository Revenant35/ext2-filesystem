#include "allocation.h"
#include "globals.h"
#include "superblock.h"
#include "block_group.h"
#include "bitmap.h"
#include "inode.h"

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mock data
static ext2_super_block *sb;
static ext2_group_desc_table *bgdt;
static FILE *fs_image;
static uint8_t *inode_bitmap;
static uint8_t *block_bitmap;

void setup(void) {
    const uint32_t block_size = 1024;

    // 1. Setup Superblock
    sb = calloc(1, sizeof(ext2_super_block));
    ck_assert_ptr_nonnull(sb);
    sb->s_magic = EXT2_SUPER_MAGIC;
    sb->s_inodes_count = 32;
    sb->s_blocks_count = 32;
    sb->s_free_inodes_count = 32;
    sb->s_free_blocks_count = 32;
    sb->s_inodes_per_group = 16;
    sb->s_blocks_per_group = 16;
    sb->s_inode_size = sizeof(ext2_inode);
    sb->s_log_block_size = 0; // -> 1024 bytes
    sb->s_first_data_block = 1;

    // 2. Setup Block Group Descriptor Table
    bgdt = malloc(sizeof(ext2_group_desc_table));
    ck_assert_ptr_nonnull(bgdt);
    bgdt->groups_count = 2;
    bgdt->groups = calloc(bgdt->groups_count, sizeof(ext2_group_desc));
    ck_assert_ptr_nonnull(bgdt->groups);

    bgdt->groups[0].bg_inode_bitmap = 3;
    bgdt->groups[0].bg_block_bitmap = 4;
    bgdt->groups[0].bg_free_inodes_count = 16;
    bgdt->groups[0].bg_free_blocks_count = 16;

    bgdt->groups[1].bg_inode_bitmap = 5;
    bgdt->groups[1].bg_block_bitmap = 6;
    bgdt->groups[1].bg_free_inodes_count = 16;
    bgdt->groups[1].bg_free_blocks_count = 16;

    // 3. Setup Bitmaps
    inode_bitmap = calloc(block_size, 1);
    block_bitmap = calloc(block_size, 1);
    ck_assert_ptr_nonnull(inode_bitmap);
    ck_assert_ptr_nonnull(block_bitmap);

    // 4. Create mock filesystem image
    fs_image = tmpfile();
    ck_assert_ptr_nonnull(fs_image);

    // Write all mock data to the temp file
    write_superblock(fs_image, sb);
    for (uint32_t i = 0; i < bgdt->groups_count; ++i) {
        write_group_descriptor(fs_image, sb, i, &bgdt->groups[i]);
        write_bitmap(fs_image, sb, bgdt->groups[i].bg_inode_bitmap, inode_bitmap);
        write_bitmap(fs_image, sb, bgdt->groups[i].bg_block_bitmap, block_bitmap);
    }
    rewind(fs_image);
}

void teardown(void) {
    free(sb);
    free(bgdt->groups);
    free(bgdt);
    free(inode_bitmap);
    free(block_bitmap);
    fclose(fs_image);
}

START_TEST(allocate_inode_should_succeed_when_inodes_are_available)
{
    // Arrange is in setup
    uint32_t new_inode_num;

    // Act
    const int result = allocate_inode(fs_image, sb, bgdt, &new_inode_num);

    // Assert
    ck_assert_int_eq(result, SUCCESS);
    ck_assert_uint_eq(new_inode_num, 1); // First free inode

    // Verify counts were decremented
    ck_assert_uint_eq(sb->s_free_inodes_count, 31);
    ck_assert_uint_eq(bgdt->groups[0].bg_free_inodes_count, 15);

    // Verify bitmap was updated on disk
    uint8_t updated_bitmap[1024];
    read_bitmap(fs_image, sb, bgdt->groups[0].bg_inode_bitmap, updated_bitmap);
    uint32_t first_free;
    find_first_free_bit(updated_bitmap, sb->s_inodes_per_group, &first_free);
    ck_assert_uint_eq(first_free, 1);
}
END_TEST

START_TEST(allocate_block_should_succeed_when_blocks_are_available)
{
    // Arrange is in setup
    uint32_t new_block_num;

    // Act
    const int result = allocate_block(fs_image, sb, bgdt, &new_block_num);

    // Assert
    ck_assert_int_eq(result, SUCCESS);
    ck_assert_uint_eq(new_block_num, 1); // First data block + first free bit

    // Verify counts were decremented
    ck_assert_uint_eq(sb->s_free_blocks_count, 31);
    ck_assert_uint_eq(bgdt->groups[0].bg_free_blocks_count, 15);
}
END_TEST

START_TEST(allocate_inode_should_fail_when_no_inodes_are_available)
{
    // Arrange
    sb->s_free_inodes_count = 0;
    bgdt->groups[0].bg_free_inodes_count = 0;
    bgdt->groups[1].bg_free_inodes_count = 0;
    uint32_t new_inode_num;

    // Act
    const int result = allocate_inode(fs_image, sb, bgdt, &new_inode_num);

    // Assert
    ck_assert_int_eq(result, ERROR);
}
END_TEST

Suite *allocation_suite(void) {
    Suite *s = suite_create("Allocation");
    TCase *tc_core = tcase_create("Core");

    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, allocate_inode_should_succeed_when_inodes_are_available);
    tcase_add_test(tc_core, allocate_block_should_succeed_when_blocks_are_available);
    tcase_add_test(tc_core, allocate_inode_should_fail_when_no_inodes_are_available);

    suite_add_tcase(s, tc_core);
    return s;
}

int main(void) {
    Suite *s = allocation_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    const int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
