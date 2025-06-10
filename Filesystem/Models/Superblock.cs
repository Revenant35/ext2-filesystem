namespace Filesystem.Models;

using Enums;

// https://wiki.osdev.org/Ext2#Superblock
public struct Superblock
{
    public required uint InodeCount;
    public required uint BlockCount;
    public required uint SuperuserReservedBlockCount;
    public required uint UnallocatedBlockCount;
    public required uint UnallocatedInodeCount;
    public required uint StartingBlockNumber;
    public required uint BlockSize;
    public required uint FragmentSize;
    public required uint BlocksPerGroup;
    public required uint FragmentsPerGroup;
    public required uint InodesPerGroup;
    public required DateTimeOffset LastMountTime;
    public required DateTimeOffset LastWrittenTime;
    public required ushort MountsSinceLastConsistencyCheck;
    public required ushort MountsBeforeNextConsistencyCheck;
    public required FileSystemState FileSystemState;
    public required ErrorHandling ErrorHandling;
    public required ushort MinorVersion;
    public required DateTimeOffset LastCheckTime;
    public required TimeSpan CheckInterval;
    public required OperatingSystemID OperatingSystemId;
    public required uint MajorVersion;
    public required ushort ReservedUserId;
    public required ushort ReservedGroupId;
    public required uint FirstNonReservedInode;
    public required ushort InodeSize;
    public required ushort BlockGroupNumber;
    public required uint OptionalFeatures;
    public required uint RequiredFeatures;
    public required uint IncompatibleFeatures;
    public required byte[] FileSystemId;
    public required byte[] VolumeName;
    public required byte[] LastMountedPath;
    public required uint CompressionAlgorithms;
    public required byte PreallocatedBlocksForFiles;
    public required byte PreallocatedBlocksForDirectories;
    public required byte[] JournalId;
    public required uint JournalInode;
    public required uint JournalDevice;
}