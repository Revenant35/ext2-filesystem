#include "../include/directory.h"
#include "../include/globals.h"
#include "../include/inode.h"
#include "../include/superblock.h"
#include "../include/block_group.h"

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mock data
static ext2_super_block *sb;
static ext2_group_desc *bgdt;
static ext2_inode *dir_inode;
static FILE *fs_image;
static char *block_buffer;

void setup(void) {
    sb = malloc(sizeof(ext2_super_block));
    ck_assert_ptr_nonnull(sb);
    sb->s_inodes_count = 32;
    sb->s_blocks_count = 32;
    sb->s_inodes_per_group = 16;
    sb->s_blocks_per_group = 16;
    sb->s_inode_size = sizeof(ext2_inode);
    sb->s_log_block_size = 0; // 1024 bytes

    bgdt = malloc(2 * sizeof(ext2_group_desc));
    ck_assert_ptr_nonnull(bgdt);
    bgdt[0].bg_inode_table = 10;
    bgdt[1].bg_inode_table = 20;

    dir_inode = malloc(sizeof(ext2_inode));
    ck_assert_ptr_nonnull(dir_inode);
    dir_inode->i_mode = EXT2_S_IFDIR;
    dir_inode->i_size = get_block_size(sb);
    dir_inode->i_links_count = 2;
    dir_inode->i_blocks = 2;
    dir_inode->i_block[0] = 15; // Data block for this directory

    block_buffer = calloc(1, get_block_size(sb));
    ck_assert_ptr_nonnull(block_buffer);

    // Create a mock directory entry for '.'
    ext2_directory_entry *entry1 = (ext2_directory_entry *)block_buffer;
    entry1->inode = 1;
    entry1->rec_len = 12;
    entry1->name_len = 1;
    entry1->file_type = EXT2_FT_DIR;
    strcpy(entry1->name, ".");

    // Create a mock directory entry for '..'
    ext2_directory_entry *entry2 = (ext2_directory_entry *)(block_buffer + entry1->rec_len);
    entry2->inode = 2;
    entry2->rec_len = get_block_size(sb) - entry1->rec_len;
    entry2->name_len = 2;
    entry2->file_type = EXT2_FT_DIR;
    strcpy(entry2->name, "..");

    fs_image = tmpfile();
    ck_assert_ptr_nonnull(fs_image);

    // Write the inode to the inode table (inode 1 is the first in group 0)
    const off_t inode_table_offset = (off_t)bgdt[0].bg_inode_table * get_block_size(sb);
    fseeko(fs_image, inode_table_offset, SEEK_SET);
    fwrite(dir_inode, sizeof(ext2_inode), 1, fs_image);

    // Write the directory data block
    fseeko(fs_image, (off_t)dir_inode->i_block[0] * get_block_size(sb), SEEK_SET);
    fwrite(block_buffer, get_block_size(sb), 1, fs_image);
    rewind(fs_image);
}

void teardown(void) {
    free(sb);
    free(bgdt);
    free(dir_inode);
    free(block_buffer);
    fclose(fs_image);
}

START_TEST(find_entry_in_directory_should_return_inode_for_existing_entry)
{
    // Arrange is done in setup

    // Act
    const uint32_t inode_num = find_entry_in_directory(fs_image, sb, bgdt, 1, ".");

    // Assert
    ck_assert_uint_eq(inode_num, 1);
}
END_TEST

Suite *directory_suite(void) {
    Suite *s = suite_create("Directory");
    TCase *tc_core = tcase_create("Core");

    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, find_entry_in_directory_should_return_inode_for_existing_entry);

    suite_add_tcase(s, tc_core);
    return s;
}

int main(void) {
    Suite *s = directory_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    const int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
