namespace Filesystem.Enums;

// https://wiki.osdev.org/Ext2#Superblock
public enum OperatingSystemID : uint
{
    Linux = 0,
    GNUHurd = 1,
    MASIX = 2,
    FreeBSD = 3,
    Other = 4,
}