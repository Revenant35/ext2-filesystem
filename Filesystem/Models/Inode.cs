namespace Filesystem.Models;

// https://wiki.osdev.org/Ext2#Inode_Data_Structure
public struct Inode
{
    public const uint Size = 128;

    public InodeType Type = InodeType.File;
    public InodePermissions Permissions = InodePermissions.None;
    public ushort UserID = 0;
    public uint SizeBytes = 0;
    public DateTimeOffset? LastAccessed = null;
    public DateTimeOffset CreatedAt = DateTimeOffset.Now;
    public DateTimeOffset? LastModified = null;
    public DateTimeOffset? DeletedAt = null;
    public ushort GroupID = 0;
    public ushort HardLinkCount = 0;
    public uint DiskSectorCount = 0;
    public InodeFlags Flags = InodeFlags.None;
    public uint[] BlockPointers = new uint[15];
    public uint GenerationNumber = 0;
    public uint FileACLBlock = 0;
    public uint DirectoryACLBlock = 0;
    public uint FragmentBlockAddress = 0;
    public byte[] OperatingSystemSpecificValues = new byte[16]; // TODO: This weirdness

    public Inode()
    {
        Array.Fill(BlockPointers, 0u);
    }
}