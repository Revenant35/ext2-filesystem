namespace Filesystem.Models;

// https://wiki.osdev.org/Ext2#Inode_Data_Structure
public struct Inode
{
    public const uint Size = 128;

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
    public uint[] BlockPointers;
    public uint SinglyIndirectBlockPointer;
    public uint DoublyIndirectBlockPointer;
    public uint TriplyIndirectBlockPointer;
    public uint GenerationNumber;
    public uint FragmentBlockAddress;
    public byte[] OperatingSystemSpecificValues; // TODO: This weirdness

    public static readonly Inode Default = new Inode
    {
        Type = InodeType.FIFO,
        Permissions = InodePermissions.None,
        UserID = 0,
        SizeBytes = 0,
        LastAccessed = DateTimeOffset.Now,
        CreatedAt = DateTimeOffset.Now,
        LastModified = DateTimeOffset.Now,
        DeletedAt = DateTimeOffset.Now,
        GroupID = 0,
        HardLinkCount = 0,
        DiskSectorCount = 0,
        Flags = InodeFlags.None,
        BlockPointers = new uint[12],
        SinglyIndirectBlockPointer = 0,
        DoublyIndirectBlockPointer = 0,
        TriplyIndirectBlockPointer = 0,
        GenerationNumber = 0,
        FragmentBlockAddress = 0,
        OperatingSystemSpecificValues = [],
    };
}