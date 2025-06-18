#ifndef C_EXT2_FILESYSTEM_INODE_H
#define C_EXT2_FILESYSTEM_INODE_H

#include <stdint.h>

#define EXT2_N_BLOCKS 15

struct ext2_inode {
    uint16_t i_mode;        // File mode (type and permissions)
    uint16_t i_uid;         // Low 16 bits of Owner Uid
    uint32_t i_size;        // Size in bytes
    uint32_t i_atime;       // Access time (POSIX time)
    uint32_t i_ctime;       // Creation time (POSIX time)
    uint32_t i_mtime;       // Modification time (POSIX time)
    uint32_t i_dtime;       // Deletion time (POSIX time)
    uint16_t i_gid;         // Low 16 bits of Group Id
    uint16_t i_links_count; // Links count
    uint32_t i_blocks;      // Blocks count (in 512-byte units)
    uint32_t i_flags;       // File flags
    uint32_t i_osd1;        // OS dependent 1

    uint32_t i_block[EXT2_N_BLOCKS]; // Pointers to blocks (12 direct, 1 indirect, 1 dbl-indirect, 1 trpl-indirect)

    uint32_t i_generation;  // File version (for NFS)
    uint32_t i_file_acl;    // File ACL
    uint32_t i_dir_acl;     // Directory ACL (or i_size_high for regular files > 2GB if EXT2_FEATURE_RO_COMPAT_LARGE_FILE)
    uint32_t i_faddr;       // Fragment address (deprecated)

    // OS dependent 2 (12 bytes)
    union {
        struct {
            uint8_t  l_i_frag;  // Fragment number (Linux)
            uint8_t  l_i_fsize; // Fragment size (Linux)
            uint16_t i_pad1;
            uint16_t l_i_uid_high; // High 16 bits of Owner Uid (Linux)
            uint16_t l_i_gid_high; // High 16 bits of Group Id (Linux)
            uint32_t l_i_reserved2; // Unused (Linux)
        } linux2;
        struct {
            uint8_t  h_i_frag;  // Fragment number (Hurd)
            uint8_t  h_i_fsize; // Fragment size (Hurd)
            uint16_t h_i_mode_high;
            uint16_t h_i_uid_high;
            uint16_t h_i_gid_high;
            uint32_t h_i_author;
        } hurd;
        struct {
            uint8_t  m_i_frag;  // Fragment number (Masix)
            uint8_t  m_i_fsize; // Fragment size (Masix)
            uint16_t m_pad1;
            uint32_t m_i_reserved2[2]; // Unused (Masix)
        } masix;
    } i_osd2;
};

#define EXT2_S_IFMT   0xF000 // Format mask
#define EXT2_S_IFSOCK 0xC000 // Socket
#define EXT2_S_IFLNK  0xA000 // Symbolic link
#define EXT2_S_IFREG  0x8000 // Regular file
#define EXT2_S_IFBLK  0x6000 // Block device
#define EXT2_S_IFDIR  0x4000 // Directory
#define EXT2_S_IFCHR  0x2000 // Character device
#define EXT2_S_IFIFO  0x1000 // FIFO

#define EXT2_S_ISUID  0x0800 // Set UID
#define EXT2_S_ISGID  0x0400 // Set GID
#define EXT2_S_ISVTX  0x0200 // Sticky bit

#define EXT2_S_IRUSR  0x0100 // User read
#define EXT2_S_IWUSR  0x0080 // User write
#define EXT2_S_IXUSR  0x0040 // User execute

#define EXT2_S_IRGRP  0x0020 // Group read
#define EXT2_S_IWGRP  0x0010 // Group write
#define EXT2_S_IXGRP  0x0008 // Group execute

#define EXT2_S_IROTH  0x0004 // Others read
#define EXT2_S_IWOTH  0x0002 // Others write
#define EXT2_S_IXOTH  0x0001 // Others execute

#define EXT2_SECRM_FL        0x00000001 // Secure deletion
#define EXT2_UNRM_FL         0x00000002 // Undelete
#define EXT2_COMPR_FL        0x00000004 // Compress file
#define EXT2_SYNC_FL         0x00000008 // Synchronous updates (data and metadata written immediately)
#define EXT2_IMMUTABLE_FL    0x00000010 // Immutable file
#define EXT2_APPEND_FL       0x00000020 // Append only
#define EXT2_NODUMP_FL       0x00000040 // Do not dump file
#define EXT2_NOATIME_FL      0x00000080 // Do not update access time
#define EXT2_DIRTY_FL        0x00000100 // Dirty (modified, not used in kernel)
#define EXT2_COMPRBLK_FL     0x00000200 // Compressed blocks
#define EXT2_NOCOMPR_FL      0x00000400 // Access raw compressed data
#define EXT2_ECOMPR_FL       0x00000800 // Compression error detected
#define EXT2_BTREE_FL        0x00001000 // B-tree format directory (deprecated by HTREE)
#define EXT2_INDEX_FL        0x00001000 // Hash indexed directory (HTREE index)
#define EXT2_IMAGIC_FL       0x00002000 // AFS directory
#define EXT3_JOURNAL_DATA_FL 0x00004000 // Journal file data (Ext3)
#define EXT2_NOTAIL_FL       0x00008000 // No tail-merging of files (Ext3)
#define EXT2_DIRSYNC_FL      0x00010000 // Directory contents must be written synchronously
#define EXT2_TOPDIR_FL       0x00020000 // Top of directory hierarchy
#define EXT4_HUGE_FILE_FL    0x00040000 // Huge file (Ext4)
#define EXT4_EXTENTS_FL      0x00080000 // Extents (Ext4)
#define EXT4_EA_INODE_FL     0x00200000 // Inode used for large EA (Ext4)
#define EXT4_EOFBLOCKS_FL    0x00400000 // Blocks allocated beyond EOF (Ext4)
#define EXT2_RESERVED_FL     0x80000000 // Reserved for ext2 library

#endif //C_EXT2_FILESYSTEM_INODE_H
