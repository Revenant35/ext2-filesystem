namespace Filesystem.Models;

using Enums;

// https://wiki.osdev.org/Ext2#Inode_Data_Structure
public struct Inode
{
    public required InodeType Type;
    public required InodePermissions Permissions;
    public required ushort UserID;
    public required uint SizeBytes;
    public required DateTimeOffset? LastAccessed;
    public required DateTimeOffset CreatedAt;
    public required DateTimeOffset? LastModified;
    public required DateTimeOffset? DeletedAt;
    public required ushort GroupID;
    public required ushort HardLinkCount;
    public required uint DiskSectorCount;
    public required InodeFlags Flags;
    public required List<uint> BlockAddresses;
    public required uint? SinglyIndirectBlockAddress;
    public required uint? DoublyIndirectBlockAddress;
    public required uint? TriplyIndirectBlockAddress;
    public required uint GenerationNumber;
    public required uint FileACLBlock;
    public required uint DirectoryACLBlock;
    public required uint FragmentBlockAddress;
    public required byte[] OperatingSystemSpecificValues;
}