namespace Filesystem.Enums;

// https://wiki.osdev.org/Ext2#Superblock
public enum FileSystemState : ushort
{
    Clean = 1,
    Errors = 2,
}