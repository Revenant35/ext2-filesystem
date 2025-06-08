namespace Filesystem.Serialization.Models;

// https://wiki.osdev.org/Ext2#Superblock
public struct BinarySuperblock
{
    public const long SizeOnDiskInBytes = 1024;
    public const ushort Ext2Signature = 0xef53;

    public required uint InodeCount;
    public required uint BlockCount;
    public required uint SuperuserReservedBlockCount;
    public required uint UnallocatedBlockCount;
    public required uint UnallocatedInodeCount;
    public required uint StartingBlockNumber;
    public required int BlockSize;
    public required int FragmentSize;
    public required uint BlocksPerGroup;
    public required uint FragmentsPerGroup;
    public required uint InodesPerGroup;
    public required uint LastMountTime;
    public required uint LastWrittenTime;
    public required ushort MountsSinceLastConsistencyCheck;
    public required ushort MountsBeforeNextConsistencyCheck;
    public required ushort FileSystemState;
    public required ushort ErrorHandling;
    public required ushort MinorVersion;
    public required uint LastCheckTime;
    public required uint CheckInterval;
    public required uint OperatingSystemId;
    public required uint MajorVersion;
    public required ushort ReservedUserId;
    public required ushort ReservedGroupId;
}