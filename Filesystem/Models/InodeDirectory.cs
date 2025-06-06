namespace Filesystem.Models;

// https://wiki.osdev.org/Ext2#Directory_Entry
public struct InodeDirectory
{
    public uint Inode;
    public ushort EntrySize;
    public InodeDirectoryType Type;
    public string Name;
}
