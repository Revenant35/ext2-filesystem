/**
 * @file Superblock.h
 * @brief Defines the structure of the ext2 superblock and related constants.
 *
 * The superblock is a critical data structure in an ext2 filesystem. It contains
 * metadata about the filesystem as a whole, such as the total number of inodes and
 * blocks, block size, filesystem state, and feature flags.
 */
#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <stdint.h>
#include <stdio.h>

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
 * @brief Reads the superblock from an open file stream into memory.
 * @param file Pointer to an open FILE stream (e.g., from an ext2 image file).
 * @return Pointer to an ext2_super_block structure containing the superblock data,
 */
ext2_super_block *read_superblock(
    FILE *file
);

/**
 * @brief Writes the superblock from memory to an open file stream.
 * @param file Pointer to an open FILE stream (e.g., for an ext2 image file).
 * @param superblock Pointer to an ext2_super_block structure containing the data to write.
 * @return 0 on success, non-zero on failure (e.g., I/O error, invalid superblock data).
 */
int write_superblock(
    FILE *file,
    const ext2_super_block *superblock
);

/**
 * @brief Calculates the block size in bytes from the superblock's log field.
 * @param superblock Pointer to a populated ext2_super_block structure.
 * @return The block size in bytes (e.g., 1024, 2048, 4096).
 */
uint32_t get_block_size(const ext2_super_block *superblock);


/**
 * @brief Calculates the fragment size in bytes from the superblock's log field.
 * @param superblock Pointer to a populated ext2_super_block structure.
 * @return The fragment size in bytes.
 */
uint32_t get_fragment_size(const ext2_super_block *superblock);

/**
 * @brief Calculates the total number of block groups in the filesystem.
 * @param superblock Pointer to a populated ext2_super_block structure.
 * @return The total number of block groups.
 */
uint32_t get_block_group_count(const ext2_super_block *superblock);

#endif //SUPERBLOCK_H
