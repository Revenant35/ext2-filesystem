find_package(Check REQUIRED)

add_executable(run_superblock_tests test_superblock.c)

target_link_libraries(run_superblock_tests PRIVATE ext2_filesystem Check::check)

add_test(NAME SinglyLinkedListTest COMMAND run_superblock_tests)
