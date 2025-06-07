namespace Filesystem.Models;

// https://wiki.osdev.org/Ext2#Superblock
public struct Superblock
{
    public const ushort Ext2Signature = 0xef53;

    public uint InodeCount = 2048;
    public uint BlockCount = 32768;
    public uint SuperuserReservedBlockCount = 1024;
    public uint UnallocatedBlockCount = 31744;
    public uint UnallocatedInodeCount = 2000;
    public uint StartingBlockNumber = 1;
    public uint BlockSize = 1024;
    public uint FragmentSize = 1024;
    public uint BlocksPerGroup = 8192;
    public uint FragmentsPerGroup = 8192;
    public uint InodesPerGroup = 2048;
    public DateTimeOffset LastMountTime = DateTimeOffset.UtcNow;
    public DateTimeOffset LastWrittenTime = DateTimeOffset.UtcNow;
    public ushort MountsSinceLastConsistencyCheck = 0;
    public ushort MountsBeforeNextConsistencyCheck = 20;
    public FileSystemState FileSystemState = FileSystemState.Clean;
    public ErrorHandling ErrorHandling = ErrorHandling.Continue;
    public ushort MinorVersion = 0;
    public DateTimeOffset LastCheckTime = DateTimeOffset.UtcNow;
    public TimeSpan CheckInterval = TimeSpan.FromDays(180);
    public OperatingSystemID OperatingSystemId = OperatingSystemID.Linux;
    public uint MajorVersion = 0;
    public ushort ReservedUserId = 0;
    public ushort ReservedGroupId = 0;

    public Superblock()
    {

    }
}