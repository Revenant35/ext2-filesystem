namespace Filesystem.Enums;

// https://wiki.osdev.org/Ext2#Inode_Data_Structure
public enum InodeType : ushort
{
    FIFO = 0x1000,
    CharacterDevice = 0x2000,
    Directory = 0x4000,
    BlockDevice = 0x6000,
    File = 0x8000,
    SymbolicLink = 0xA000,
    UnixSocket = 0xC000,
}