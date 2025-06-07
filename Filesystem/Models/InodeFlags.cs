namespace Filesystem.Models;

// https://wiki.osdev.org/Ext2#Inode_Data_Structure
[Flags]
public enum InodeFlags : uint
{
    None = 0,
    SecureDeletion = 1 << 0,
    KeepCopyOfDataWhenDeleted = 1 << 1,
    FileCompression = 1 << 2,
    SynchronousUpdates = 1 << 3,
    ImmutableFile = 1 << 4,
    AppendOnly = 1 << 5,
    FileExcludedFromDump = 1 << 6,
    LastAccessTimeShouldNotBeUpdated = 1 << 7,
    HashIndexedDirectory = 1 << 16,
    AFSDirectory = 1 << 17,
    JournalFileData = 1 << 18,
}