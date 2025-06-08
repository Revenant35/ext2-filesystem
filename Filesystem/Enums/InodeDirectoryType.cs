namespace Filesystem.Enums;

// https://wiki.osdev.org/Ext2#Directory_Entry
public enum InodeDirectoryType : byte
{
    UnknownType = 0,
    RegularFile = 1,
    Directory = 2,
    CharacterDevice = 3,
    BlockDevice = 4,
    FIFO = 5,
    Socket = 6,
    SymbolicLink = 7,
}