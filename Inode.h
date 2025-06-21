#ifndef INODE_H
#define INODE_H

#include <stdint.h>
#include <stdio.h>

struct ext2_super_block;
struct ext2_group_desc;

#define EXT2_N_BLOCKS 15 //!< Number of block pointers in an inode (12 direct, 1 indirect, 1 dbl-indirect, 1 trpl-indirect)

#define EXT2_ROOT_INO 2          //!< Inode number for the root directory

/**
 * @brief The ext2 inode structure (on-disk, 128 bytes for rev 0, potentially larger for rev 1+).
 *
 * This structure contains all metadata for a file or directory, including mode,
 * ownership, size, timestamps, and pointers to data blocks.
 */
struct ext2_inode {
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

/**
 * @brief Reads an inode from the filesystem into memory.
 *
 * @param file Pointer to an open FILE stream for the filesystem image.
 * @param superblock Pointer to the filesystem's superblock.
 * @param block_group_descriptor_table Pointer to the array of block group descriptors (the BLOCK_GROUP_DESCRIPTOR_TABLE).
 * @param inode_num The number of the inode to read (1-based).
 * @param inode_out Pointer to an `ext2_inode` structure to populate with the read data.
 * @return 0 on success, or a negative error code on failure.
 */
int read_inode(
    FILE *file,
    const struct ext2_super_block *superblock,
    const struct ext2_group_desc *block_group_descriptor_table,
    uint32_t inode_num,
    struct ext2_inode *inode_out
);

/**
 * @brief Writes an inode from memory to the filesystem.
 *
 * @param file Pointer to an open FILE stream for the filesystem image.
 * @param superblock Pointer to the filesystem's superblock.
 * @param block_group_descriptor_table Pointer to the array of block group descriptors (the BLOCK_GROUP_DESCRIPTOR_TABLE).
 * @param inode_num The number of the inode to write (1-based).
 * @param inode_in Pointer to an `ext2_inode` structure containing the data to write.
 * @return 0 on success, or a negative error code on failure.
 */
int write_inode(
    FILE *file,
    const struct ext2_super_block *superblock,
    const struct ext2_group_desc *block_group_descriptor_table,
    uint32_t inode_num,
    const struct ext2_inode *inode_in
);


#endif //INODE_H
