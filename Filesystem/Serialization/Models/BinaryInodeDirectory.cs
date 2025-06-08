namespace Filesystem.Serialization.Models;

// https://wiki.osdev.org/Ext2#Directory_Entry
public struct BinaryInodeDirectory
{
    public required uint InodeAddress;
    public required ushort EntrySize;
    public required byte NameLength;
    public required byte Type;
    public required byte[] Name;
}