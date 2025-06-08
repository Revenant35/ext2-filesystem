namespace Filesystem.Enums;

// https://wiki.osdev.org/Ext2#Superblock
public enum ErrorHandling : ushort
{
    Continue = 1,
    RemountReadOnly = 2,
    Panic = 3,
}