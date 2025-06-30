#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#define EXT2_BG_INODE_UNINIT    0x0001  // Inode table and bitmap are not initialized
#define EXT2_BG_BLOCK_UNINIT    0x0002  // Block bitmap is not initialized
#define EXT2_BG_INODE_ZEROED    0x0004  // Inode table is zeroed

#define EXT2_NAME_LEN 255

#define EXT2_FT_UNKNOWN  0 // Unknown Type
#define EXT2_FT_REG_FILE 1 // Regular File
#define EXT2_FT_DIR      2 // Directory
#define EXT2_FT_CHRDEV   3 // Character Device
#define EXT2_FT_BLKDEV   4 // Block Device
#define EXT2_FT_FIFO     5 // FIFO (named pipe)
#define EXT2_FT_SOCK     6 // Socket
#define EXT2_FT_SYMLINK  7 // Symbolic Link

#define EXT2_N_BLOCKS 15 //!< Number of block pointers in an inode (12 direct, 1 indirect, 1 dbl-indirect, 1 trpl-indirect)

#define EXT2_ROOT_INO 2          //!< Inode number for the root directory

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

#define EXT2_SUPER_MAGIC 0xEF53
#define EXT2_SUPERBLOCK_OFFSET 1024 // Superblock is always at byte 1024

#define EXT2_VALID_FS 1 // Unmounted cleanly
#define EXT2_ERROR_FS 2 // Errors detected

#define EXT2_ERRORS_CONTINUE 1 // Continue
#define EXT2_ERRORS_RO     2 // Remount read-only
#define EXT2_ERRORS_PANIC  3 // Panic

#define EXT2_OS_LINUX   0
#define EXT2_OS_HURD    1
#define EXT2_OS_MASIX   2 // (Historically, MASIX was an alternative name for some GNU variants)
#define EXT2_OS_FREEBSD 3
#define EXT2_OS_LITES   4 // (Other BSD derivatives)

#define EXT2_GOOD_OLD_REV 0 // Revision 0 (original)
#define EXT2_DYNAMIC_REV  1 // Revision 1 (dynamic resizing inodes, etc.)

#define EXT2_FEATURE_COMPAT_DIR_PREALLOC  0x0001 // Directory preallocation
#define EXT2_FEATURE_COMPAT_IMAGIC_INODES 0x0002 // (Not used in standard Linux Ext2)
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL   0x0004 // Has a journal
#define EXT2_FEATURE_COMPAT_EXT_ATTR      0x0008 // Extended attributes
#define EXT2_FEATURE_COMPAT_RESIZE_INO    0x0010 // Non-standard resize inode feature
#define EXT2_FEATURE_COMPAT_DIR_INDEX     0x0020 // Directory indexing (htree)

#define EXT2_FEATURE_INCOMPAT_COMPRESSION 0x0001 // Compression
#define EXT2_FEATURE_INCOMPAT_FILETYPE    0x0002 // Filetype field in directory entries
#define EXT3_FEATURE_INCOMPAT_RECOVER     0x0004 // Needs recovery (journal)
#define EXT3_FEATURE_INCOMPAT_JOURNAL_DEV 0x0008 // Journal device
#define EXT2_FEATURE_INCOMPAT_META_BG     0x0010 // Meta block groups

#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER 0x0001 // Sparse superblocks
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE   0x0002 // Large files (64-bit file size)
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR    0x0004 // Binary tree sorted directories (deprecated by HTREE)
#define EXT2_FEATURE_RO_COMPAT_HUGE_FILE    0x0008 // Files larger than 2TB
#define EXT2_FEATURE_RO_COMPAT_GDT_CSUM     0x0010 // Group descriptor checksums
#define EXT2_FEATURE_RO_COMPAT_DIR_NLINK    0x0020 // Directory NLINK support
#define EXT2_FEATURE_RO_COMPAT_EXTRA_ISIZE  0x0040 // Extra inode size

/**
 * @brief The ext2 block group descriptor structure (typically 32 bytes).
 *
 * This structure holds metadata for a single block group within the filesystem.
 * An array of these descriptors, known as the Block Group Descriptor Table (BGDT),
 * follows the superblock.
 */
typedef struct {
    uint32_t bg_block_bitmap;         //!< Block ID of the block usage bitmap for this group.
    uint32_t bg_inode_bitmap;         //!< Block ID of the inode usage bitmap for this group.
    uint32_t bg_inode_table;          //!< Block ID of the starting block of the inode table for this group.
    uint16_t bg_free_blocks_count;    //!< Number of free blocks in this group.
    uint16_t bg_free_inodes_count;    //!< Number of free inodes in this group.
    uint16_t bg_used_dirs_count;      //!< Number of directories in this group.
    uint16_t bg_flags;                //!< Block group flags (see EXT2_BG_* defines).
    uint32_t bg_reserved1;            //!< Reserved for future use. (Was bg_exclude_bitmap_lo in ext4 for snapshot feature)
    uint16_t bg_reserved2;            //!< Reserved for future use. (Was bg_block_bitmap_csum_lo in ext4 for checksumming)
    uint16_t bg_reserved3;            //!< Reserved for future use. (Was bg_inode_bitmap_csum_lo in ext4 for checksumming)
    uint16_t bg_itable_unused;        //!< Number of unused inodes in this group (if INODE_ZEROED flag is set).
    uint16_t bg_checksum;             //!< Group descriptor checksum (if EXT2_FEATURE_RO_COMPAT_GDT_CSUM is set in superblock).
} ext2_group_desc;

typedef struct {
    ext2_group_desc *groups;
    uint32_t groups_count;
} ext2_group_desc_table;

typedef struct {
    uint32_t inode;     // Inode number (0 if entry is unused)
    uint16_t rec_len;   // Directory entry length (total size of this entry in bytes)
    uint8_t  name_len;  // Name length (actual length of the name string in characters)
    uint8_t  file_type; // File type indicator
    char     name[EXT2_NAME_LEN]; // File name characters (NOT necessarily null-terminated within this array)
                                  // The actual name occupies the first 'name_len' bytes.
} ext2_directory_entry;

/**
 * @brief The ext2 inode structure (on-disk, 128 bytes for rev 0, potentially larger for rev 1+).
 *
 * This structure contains all metadata for a file or directory, including mode,
 * ownership, size, timestamps, and pointers to data blocks.
 */
typedef struct {
    uint16_t i_mode;        //!< File mode (type: regular, directory, symlink, etc., and permissions).
    uint16_t i_uid;         //!< Low 16 bits of Owner User ID.
    uint32_t i_size;        //!< File size in bytes. For symbolic links, this is the length of the target path.
    uint32_t i_atime;       //!< Last access time (POSIX time: seconds since epoch).
    uint32_t i_ctime;       //!< Inode change time (POSIX time) - NOT file creation time in traditional sense.
    uint32_t i_mtime;       //!< Last modification time (POSIX time).
    uint32_t i_dtime;       //!< Deletion time (POSIX time). Set when the inode is marked as deleted.
    uint16_t i_gid;         //!< Low 16 bits of Group ID.
    uint16_t i_links_count; //!< Number of hard links to this inode. When 0, inode is free.
    uint32_t i_blocks;      //!< Number of 512-byte blocks allocated to this file (disk usage).
    uint32_t i_flags;       //!< File flags (see EXT2_*_FL defines).
    uint32_t i_osd1;        //!< OS Dependent Value 1.

    uint32_t i_block[EXT2_N_BLOCKS]; //!< Array of block pointers. First 12 are direct blocks.
                                     //!< i_block[12] is a singly indirect block pointer.
                                     //!< i_block[13] is a doubly indirect block pointer.
                                     //!< i_block[14] is a triply indirect block pointer.

    uint32_t i_generation;  //!< File version (used by NFS).
    uint32_t i_file_acl;    //!< File Access Control List (block number). If 0, no ACL.
    uint32_t i_dir_acl;     //!< Directory ACL (block number). For regular files, this can be i_size_high
                            //!< (high 32 bits of file size) if EXT2_FEATURE_RO_COMPAT_LARGE_FILE is set.
    uint32_t i_faddr;       //!< Fragment address (obsolete, not used in modern ext2).

    /** @brief OS dependent 2 structure (12 bytes). Content varies by OS. */
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
} ext2_inode;

/**
 * @brief The ext2 superblock structure (1024 bytes).
 *
 * This structure holds all the metadata about an ext2 filesystem.
 * It is located at a fixed offset (1024 bytes) from the beginning of the
 * block device or image file.
 */
typedef struct {
    uint32_t   s_inodes_count;         // Total number of inodes in file system
    uint32_t   s_blocks_count;         // Total number of blocks in file system
    uint32_t   s_r_blocks_count;       // Number of blocks reserved for superuser
    uint32_t   s_free_blocks_count;    // Total number of free blocks
    uint32_t   s_free_inodes_count;    // Total number of free inodes
    uint32_t   s_first_data_block;     // ID of first data block (always 0 or 1)
    uint32_t   s_log_block_size;       // Block size (log2(block_size) - 10)
    uint32_t   s_log_frag_size;        // Fragment size (log2(frag_size) - 10)
    uint32_t   s_blocks_per_group;     // Number of blocks in each block group
    uint32_t   s_frags_per_group;      // Number of fragments in each block group
    uint32_t   s_inodes_per_group;     // Number of inodes in each block group
    uint32_t   s_mtime;                // Last mount time (POSIX time)
    uint32_t   s_wtime;                // Last written time (POSIX time)
    uint16_t   s_mnt_count;            // Number of times mounted since last fsck
    uint16_t   s_max_mnt_count;        // Max number of times to mount before fsck
    uint16_t   s_magic;                // Magic number (0xEF53)
    uint16_t   s_state;                // File system state (see EXT2_VALID_FS, EXT2_ERROR_FS)
    uint16_t   s_errors;               // Behaviour when detecting errors (see EXT2_ERRORS_*)
    uint16_t   s_minor_rev_level;      // Minor revision level
    uint32_t   s_lastcheck;            // Last fsck time (POSIX time)
    uint32_t   s_checkinterval;        // Max time between fscks (POSIX time)
    uint32_t   s_creator_os;           // Creator OS (see EXT2_OS_*)
    uint32_t   s_rev_level;            // Revision level (see EXT2_GOOD_OLD_REV, EXT2_DYNAMIC_REV)
    uint16_t   s_def_resuid;           // Default UID for reserved blocks
    uint16_t   s_def_resgid;           // Default GID for reserved blocks
    uint32_t   s_first_ino;            // First non-reserved inode in file system (usually 11)
    uint16_t   s_inode_size;           // Size of inode structure in bytes (usually 128)
    uint16_t   s_block_group_nr;       // Block group number of this superblock copy
    uint32_t   s_feature_compat;       // Compatible feature set flags (see EXT2_FEATURE_COMPAT_*)
    uint32_t   s_feature_incompat;     // Incompatible feature set flags (see EXT2_FEATURE_INCOMPAT_*)
    uint32_t   s_feature_ro_compat;    // Read-only compatible feature set flags (see EXT2_FEATURE_RO_COMPAT_*)
    uint8_t    s_uuid[16];             // Volume UUID
    char       s_volume_name[16];      // Volume name (C-style string: null-terminated)
    char       s_last_mounted[64];     // Path where last mounted (C-style string: null-terminated)
    uint32_t   s_algo_bitmap;          // For compression (usage varies)
    uint8_t    s_prealloc_blocks;      // Number of blocks to preallocate for files
    uint8_t    s_prealloc_dir_blocks;  // Number of blocks to preallocate for directories
    uint16_t   s_reserved_gdt_blocks;  // Number of reserved BLOCK_GROUP_DESCRIPTOR_TABLE blocks for future expansion
    uint8_t    s_journal_uuid[16];     // UUID of journal superblock
    uint32_t   s_journal_inum;         // Inode number of journal file
    uint32_t   s_journal_dev;          // Device number of journal file
    uint32_t   s_last_orphan;          // Start of list of orphaned inodes to delete
    uint32_t   s_hash_seed[4];         // HTREE hash seed
    uint8_t    s_def_hash_version;     // Default hash algorithm used for directory hashing
    uint8_t    s_jnl_backup_type;
    uint16_t   s_desc_size;            // Size of group descriptors, if EXT2_FEATURE_INCOMPAT_META_BG set.
    uint32_t   s_default_mount_opts;
    uint32_t   s_first_meta_bg;        // First metablock group
    uint32_t   s_mkfs_time;            // When the filesystem was created
    uint32_t   s_jnl_blocks[17];       // Backup of the journal inode's i_block array
    uint32_t   s_blocks_count_hi;      // High 32 bits of blocks count
    uint32_t   s_r_blocks_count_hi;    // High 32 bits of reserved blocks count
    uint32_t   s_free_blocks_count_hi; // High 32 bits of free blocks count
    uint16_t   s_min_extra_isize;      // All inodes have at least # bytes
    uint16_t   s_want_extra_isize;     // New inodes should reserve # bytes
    uint32_t   s_flags;                // Miscellaneous flags
    uint16_t   s_raid_stride;          // RAID stride
    uint16_t   s_mmp_update_interval;  // # seconds to wait in MMP checking
    uint64_t   s_mmp_block;            // Block for multi-mount protection
    uint32_t   s_raid_stripe_width;    // blocks on all data disks (N * stride)
    uint8_t    s_log_groups_per_flex;  // FLEX_BG group size
    uint8_t    s_checksum_type;        // metadata checksum algorithm type
    uint16_t   s_reserved_pad;         // Padding to a 1024-byte boundary
    uint64_t   s_kbytes_written;       // Number of KiB written to this filesystem
    uint32_t   s_snapshot_inum;        // Inode number of active snapshot
    uint32_t   s_snapshot_id;          // Sequential ID of active snapshot
    uint64_t   s_snapshot_r_blocks_count; // Reserved blocks in active snapshot
    uint32_t   s_snapshot_list;        // Inode number of head of snapshot list
    uint32_t   s_error_count;          // Number of errors seen
    uint32_t   s_first_error_time;     // First time an error happened
    uint32_t   s_first_error_ino;      // Inode involved in first error
    uint64_t   s_first_error_block;    // Block involved of first error
    uint8_t    s_first_error_func[32]; // Function where the error happened
    uint32_t   s_first_error_line;     // Line number where error happened
    uint32_t   s_last_error_time;      // Last time an error happened
    uint32_t   s_last_error_ino;       // Inode involved in last error
    uint32_t   s_last_error_line;      // Line number where last error happened
    uint64_t   s_last_error_block;     // Block involved of last error
    uint8_t    s_last_error_func[32];  // Function where the error happened
    uint8_t    s_mount_opts[64];
    uint32_t   s_usr_quota_inum;       // Inode for tracking user quota
    uint32_t   s_grp_quota_inum;       // Inode for tracking group quota
    uint32_t   s_overhead_clusters;    // Overhead blocks/clusters in fs
    uint32_t   s_backup_bgs[2];        // Groups with sparse_super2
    uint8_t    s_encrypt_algos[4];     // Encryption algorithms in use
    uint8_t    s_encrypt_pw_salt[16];  // Salt for string2key algorithm
    uint32_t   s_lpf_ino;              // Location of the lost+found inode
    uint32_t   s_prj_quota_inum;       // Inode for tracking project quota
    uint32_t   s_checksum_seed;        // Checksum seed
    uint8_t    s_padding[300];         // Padding to 1024 bytes
} ext2_super_block;

/**
 * @brief Represents the entire state of a mounted ext2 filesystem.
 *
 * This structure encapsulates the file pointer to the device, the superblock,
 * and the block group descriptor table, providing a single context object
 * for all filesystem operations.
 */
typedef struct {
    FILE *device;
    ext2_super_block *superblock;
    ext2_group_desc_table *bgdt;
} ext2_filesystem;

// Minimum size of a directory entry's fixed part (inode + rec_len + name_len + file_type)
#define EXT2_DIR_ENTRY_FIXED_SIZE (sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint8_t))

// Macro to calculate the padded length of a directory entry based on name length.
// Directory entries must be 4-byte aligned.
// (8 bytes for fixed fields + name_len, then rounded up to a multiple of 4)
#define EXT2_DIR_REC_LEN(name_len) (((name_len) + EXT2_DIR_ENTRY_FIXED_SIZE + 3) & ~3)


#endif //TYPES_H
