namespace Filesystem.Models;

// https://wiki.osdev.org/Ext2#Inode_Data_Structure
[Flags]
public enum InodePermissions : ushort
{
    None = 0,
    OtherExecute = 1 << 0,
    OtherWrite = 1 << 1,
    OtherRead = 1 << 2,
    GroupExecute = 1 << 3,
    GroupWrite = 1 << 4,
    GroupRead = 1 << 5,
    UserExecute = 1 << 6,
    UserWrite = 1 << 7,
    UserRead = 1 << 8,
    Sticky = 1 << 9,
    SetGroupID = 1 << 10,
    SetUserID = 1 << 11,
}
