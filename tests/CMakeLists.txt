find_package(Check REQUIRED)

add_executable(run_superblock_tests test_superblock.c)
add_executable(run_block_group_tests test_block_group.c)

target_link_libraries(run_superblock_tests PRIVATE ext2_filesystem Check::check)
target_link_libraries(run_block_group_tests PRIVATE ext2_filesystem Check::check)

add_test(NAME SuperblockTest COMMAND run_superblock_tests)
add_test(NAME BlockGroupTest COMMAND run_block_group_tests)

add_executable(run_inode_tests test_inode.c)

target_link_libraries(run_inode_tests PRIVATE ext2_filesystem Check::check)

add_test(NAME InodeTest COMMAND run_inode_tests)

add_executable(run_directory_tests test_directory.c)

target_link_libraries(run_directory_tests PRIVATE ext2_filesystem Check::check)

add_test(NAME DirectoryTest COMMAND run_directory_tests)

add_executable(run_bitmap_tests test_bitmap.c)

target_link_libraries(run_bitmap_tests PRIVATE ext2_filesystem Check::check)

add_test(NAME BitmapTest COMMAND run_bitmap_tests)

add_executable(run_allocation_tests test_allocation.c)

target_link_libraries(run_allocation_tests PRIVATE ext2_filesystem Check::check)

add_test(NAME AllocationTest COMMAND run_allocation_tests)
