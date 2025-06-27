add_library(ext2_filesystem STATIC
        superblock.c
        block_group.c
        inode.c
        directory.c
        bitmap.c
        allocation.c
)

target_include_directories(ext2_filesystem PUBLIC "${PROJECT_SOURCE_DIR}/include")

add_executable(c_ext2_filesystem main.c)
target_link_libraries(c_ext2_filesystem PRIVATE ext2_filesystem)