namespace Filesystem.Serialization.Models;

// https://wiki.osdev.org/Ext2#Inode_Data_Structure
public struct BinaryInode
{
    public required ushort TypeAndPermissions;
    public required ushort UserID;
    public required uint SizeBytes;
    public required uint LastAccessed;
    public required uint CreatedAt;
    public required uint LastModified;
    public required uint DeletedAt;
    public required ushort GroupID;
    public required ushort HardLinkCount;
    public required uint DiskSectorCount;
    public required uint Flags;
    public required uint[] BlockAddresses;
    public required uint SinglyIndirectBlockAddress;
    public required uint DoublyIndirectBlockAddress;
    public required uint TriplyIndirectBlockAddress;
    public required uint GenerationNumber;
    public required uint FileACLBlock;
    public required uint DirectoryACLBlock;
    public required uint FragmentBlockAddress;
    public required byte[] OperatingSystemSpecificValues;
}