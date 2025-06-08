namespace Filesystem.Mapping;

using Enums;
using Models;
using Serialization.Models;

public static class SuperblockMapper
{
    public static Superblock ToSuperblock(this BinarySuperblock binary) => new()
    {
        InodeCount = binary.InodeCount,
        BlockCount = binary.BlockCount,
        SuperuserReservedBlockCount = binary.SuperuserReservedBlockCount,
        UnallocatedBlockCount = binary.UnallocatedBlockCount,
        UnallocatedInodeCount = binary.UnallocatedInodeCount,
        StartingBlockNumber = binary.StartingBlockNumber,
        BlockSize = 1024u << Convert.ToInt32(binary.BlockSize),
        FragmentSize = 1024u << Convert.ToInt32(binary.FragmentSize),
        BlocksPerGroup = binary.BlocksPerGroup,
        FragmentsPerGroup = binary.FragmentsPerGroup,
        InodesPerGroup = binary.InodesPerGroup,
        LastMountTime = DateTimeOffset.FromUnixTimeSeconds(binary.LastMountTime),
        LastWrittenTime = DateTimeOffset.FromUnixTimeSeconds(binary.LastWrittenTime),
        MountsSinceLastConsistencyCheck = binary.MountsSinceLastConsistencyCheck,
        MountsBeforeNextConsistencyCheck = binary.MountsBeforeNextConsistencyCheck,
        FileSystemState = (FileSystemState)binary.FileSystemState,
        ErrorHandling = (ErrorHandling)binary.ErrorHandling,
        MinorVersion = binary.MinorVersion,
        LastCheckTime = DateTimeOffset.FromUnixTimeSeconds(binary.LastCheckTime),
        CheckInterval = TimeSpan.FromSeconds(binary.CheckInterval),
        OperatingSystemId = (OperatingSystemID)binary.OperatingSystemId,
        MajorVersion = binary.MajorVersion,
        ReservedUserId = binary.ReservedUserId,
        ReservedGroupId = binary.ReservedGroupId,
    };

    public static BinarySuperblock ToBinarySuperblock(this Superblock superblock) => new()
    {
        InodeCount = superblock.InodeCount,
        BlockCount = superblock.BlockCount,
        SuperuserReservedBlockCount = superblock.SuperuserReservedBlockCount,
        UnallocatedBlockCount = superblock.UnallocatedBlockCount,
        UnallocatedInodeCount = superblock.UnallocatedInodeCount,
        StartingBlockNumber = superblock.StartingBlockNumber,
        BlockSize = (int)uint.Log2(superblock.BlockSize) - 10,
        FragmentSize = (int)uint.Log2(superblock.FragmentSize) - 10,
        BlocksPerGroup = superblock.BlocksPerGroup,
        FragmentsPerGroup = superblock.FragmentsPerGroup,
        InodesPerGroup = superblock.InodesPerGroup,
        LastMountTime = Convert.ToUInt32(superblock.LastMountTime.ToUnixTimeSeconds()),
        LastWrittenTime = Convert.ToUInt32(superblock.LastWrittenTime.ToUnixTimeSeconds()),
        MountsSinceLastConsistencyCheck = superblock.MountsSinceLastConsistencyCheck,
        MountsBeforeNextConsistencyCheck = superblock.MountsBeforeNextConsistencyCheck,
        FileSystemState = (ushort)superblock.FileSystemState,
        ErrorHandling = (ushort)superblock.ErrorHandling,
        MinorVersion = superblock.MinorVersion,
        LastCheckTime = Convert.ToUInt32(superblock.LastCheckTime.ToUnixTimeSeconds()),
        CheckInterval = Convert.ToUInt32(superblock.CheckInterval.TotalSeconds),
        OperatingSystemId = (uint)superblock.OperatingSystemId,
        MajorVersion = superblock.MajorVersion,
        ReservedUserId = superblock.ReservedUserId,
        ReservedGroupId = superblock.ReservedGroupId,
    };
}