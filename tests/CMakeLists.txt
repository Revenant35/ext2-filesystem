find_package(Check REQUIRED)

add_executable(run_superblock_tests test_superblock.c)
add_executable(run_block_group_tests test_block_group.c)

target_link_libraries(run_superblock_tests PRIVATE ext2_filesystem Check::check)
target_link_libraries(run_block_group_tests PRIVATE ext2_filesystem Check::check)

add_test(NAME SuperblockTest COMMAND run_superblock_tests)
add_test(NAME BlockGroupTest COMMAND run_block_group_tests)
