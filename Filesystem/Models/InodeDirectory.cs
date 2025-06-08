namespace Filesystem.Models;

using Enums;

// https://wiki.osdev.org/Ext2#Directory_Entry
public struct InodeDirectory
{
    public required uint InodeAddress;
    public required ushort EntrySize;
    public required InodeDirectoryType Type;
    public required string Name;
}