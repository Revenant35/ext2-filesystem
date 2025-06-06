namespace Filesystem.Models;

// https://wiki.osdev.org/Ext2#Inode_Data_Structure
public struct Inode
{
    public InodeType Type;
    public InodePermissions Permissions;
    public ushort UserID;
    public uint SizeBytes;
    public DateTimeOffset LastAccessed;
    public DateTimeOffset CreatedAt;
    public DateTimeOffset LastModified;
    public DateTimeOffset DeletedAt;
    public ushort GroupID;
    public ushort HardLinkCount;
    public uint DiskSectorCount;
    public InodeFlags Flags;
    public uint[] BlockPointers; // TODO: uint -> Block
    public uint SinglyIndirectBlockPointer;
    public uint DoublyIndirectBlockPointer;
    public uint TriplyIndirectBlockPointer;
    public uint GenerationNumber;
    public uint FragmentBlockAddress;
    public byte[] OperatingSystemSpecificValues; // TODO: This weirdness
}