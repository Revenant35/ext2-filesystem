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
    public uint BlockSize;
    public uint FragmentSize;
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
    
    public static readonly Superblock Default = new()
    {
        InodeCount = 2048,
        BlockCount = 32768,
        SuperuserReservedBlockCount = 1024,
        UnallocatedBlockCount = 31744,
        UnallocatedInodeCount = 2000,
        StartingBlockNumber = 1,
        BlockSize = 1024,
        FragmentSize = 1024,
        BlocksPerGroup = 8192,
        FragmentsPerGroup = 8192,
        InodesPerGroup = 2048,
        LastMountTime = DateTimeOffset.UtcNow,
        LastWrittenTime = DateTimeOffset.UtcNow,
        MountsSinceLastConsistencyCheck = 0,
        MountsBeforeNextConsistencyCheck = 20,
        FileSystemState = FileSystemState.Clean,
        ErrorHandling = ErrorHandling.Continue,
        MinorVersion = 0,
        LastCheckTime = DateTimeOffset.UtcNow,
        CheckInterval = TimeSpan.FromDays(180),
        OperatingSystemId = OperatingSystemID.Linux,
        MajorVersion = 1,
        ReservedUserId = 0,
        ReservedGroupId = 0,
    };
}