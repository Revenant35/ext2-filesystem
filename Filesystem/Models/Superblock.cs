namespace Filesystem.Models;

// https://wiki.osdev.org/Ext2#Superblock
public struct Superblock
{
    public const ushort Ext2Signature = 0xef53;

    public uint InodeCount;
    public uint BlockCount;
    public uint SuperuserReservedBlockCount;
    public uint UnallocatedBlockCount;
    public uint UnallocatedInodeCount;
    public uint StartingBlockNumber;
    public int BlockSize;
    public int FragmentSize;
    public uint BlocksPerGroup;
    public uint FragmentsPerGroup;
    public uint InodesPerGroup;
    public DateTimeOffset LastMountTime;
    public DateTimeOffset LastWrittenTime;
    public ushort MountsSinceLastConsistencyCheck;
    public ushort MountsBeforeNextConsistencyCheck;
    public FileSystemState FileSystemState;
    public ErrorHandling ErrorHandling;
    public ushort MinorVersion;
    public DateTimeOffset LastCheckTime;
    public TimeSpan CheckInterval;
    public OperatingSystemID OperatingSystemId;
    public uint MajorVersion;
    public ushort ReservedUserId;
    public ushort ReservedGroupId;
}