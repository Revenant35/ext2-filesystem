namespace Filesystem.Serialization.Serializers;

using Enums;
using Filesystem.Models;
using Mapping;
using Models;

// https://wiki.osdev.org/Ext2#Superblock
public static class SuperblockSerializer
{
    public static Superblock ReadSuperblock(this BinaryReader reader)
    {
        var inodeCount = reader.ReadUInt32();
        var blockCount = reader.ReadUInt32();
        var superuserReservedBlockCount = reader.ReadUInt32();
        var unallocatedBlockCount = reader.ReadUInt32();
        var unallocatedInodeCount = reader.ReadUInt32();
        var startingBlockNumber = reader.ReadUInt32();
        var blockSize = reader.ReadInt32();
        var fragmentSize = reader.ReadInt32();
        var blocksPerGroup = reader.ReadUInt32();
        var fragmentsPerGroup = reader.ReadUInt32();
        var inodesPerGroup = reader.ReadUInt32();
        var lastMountTime = reader.ReadUInt32();
        var lastWrittenTime = reader.ReadUInt32();
        var mountsSinceLastConsistencyCheck = reader.ReadUInt16();
        var mountsBeforeNextConsistencyCheck = reader.ReadUInt16();
        var signature = reader.ReadUInt16();
        if (signature != BinarySuperblock.Ext2Signature)
        {
            throw new Exception($"Invalid ext2 signature. Expected {BinarySuperblock.Ext2Signature}, got {signature}");
        }

        var fileSystemState = reader.ReadUInt16();
        if (!Enum.IsDefined(typeof(FileSystemState), fileSystemState))
        {
            throw new Exception($"Invalid file system state: {fileSystemState}");
        }

        var errorHandling = reader.ReadUInt16();
        if (!Enum.IsDefined(typeof(ErrorHandling), errorHandling))
        {
            throw new Exception($"Invalid error handling: {errorHandling}");
        }

        var minorVersion = reader.ReadUInt16();
        var lastCheckTime = reader.ReadUInt32();
        var checkInterval = reader.ReadUInt32();
        var operatingSystemId = reader.ReadUInt32();
        if (!Enum.IsDefined(typeof(OperatingSystemID), operatingSystemId))
        {
            throw new Exception($"Invalid operating system ID: {operatingSystemId}");
        }

        var majorVersion = reader.ReadUInt32();
        var reservedUserId = reader.ReadUInt16();
        var reservedGroupId = reader.ReadUInt16();

        reader.ReadBytes(940); // (unused)

        var binary = new BinarySuperblock
        {
            InodeCount = inodeCount,
            BlockCount = blockCount,
            SuperuserReservedBlockCount = superuserReservedBlockCount,
            UnallocatedBlockCount = unallocatedBlockCount,
            UnallocatedInodeCount = unallocatedInodeCount,
            StartingBlockNumber = startingBlockNumber,
            BlockSize = blockSize,
            FragmentSize = fragmentSize,
            BlocksPerGroup = blocksPerGroup,
            FragmentsPerGroup = fragmentsPerGroup,
            InodesPerGroup = inodesPerGroup,
            LastMountTime = lastMountTime,
            LastWrittenTime = lastWrittenTime,
            MountsSinceLastConsistencyCheck = mountsSinceLastConsistencyCheck,
            MountsBeforeNextConsistencyCheck = mountsBeforeNextConsistencyCheck,
            FileSystemState = fileSystemState,
            ErrorHandling = errorHandling,
            MinorVersion = minorVersion,
            LastCheckTime = lastCheckTime,
            CheckInterval = checkInterval,
            OperatingSystemId = operatingSystemId,
            MajorVersion = majorVersion,
            ReservedUserId = reservedUserId,
            ReservedGroupId = reservedGroupId,
        };

        return binary.ToSuperblock();
    }

    public static void Write(this BinaryWriter writer, Superblock superblock)
    {
        var binary = superblock.ToBinarySuperblock();

        writer.Write(binary.InodeCount);
        writer.Write(binary.BlockCount);
        writer.Write(binary.SuperuserReservedBlockCount);
        writer.Write(binary.UnallocatedBlockCount);
        writer.Write(binary.UnallocatedInodeCount);
        writer.Write(binary.StartingBlockNumber);
        writer.Write(binary.BlockSize);
        writer.Write(binary.FragmentSize);
        writer.Write(binary.BlocksPerGroup);
        writer.Write(binary.FragmentsPerGroup);
        writer.Write(binary.InodesPerGroup);
        writer.Write(binary.LastMountTime);
        writer.Write(binary.LastWrittenTime);
        writer.Write(binary.MountsSinceLastConsistencyCheck);
        writer.Write(binary.MountsBeforeNextConsistencyCheck);
        writer.Write(BinarySuperblock.Ext2Signature);
        writer.Write(binary.FileSystemState);
        writer.Write(binary.ErrorHandling);
        writer.Write(binary.MinorVersion);
        writer.Write(binary.LastCheckTime);
        writer.Write(binary.CheckInterval);
        writer.Write(binary.OperatingSystemId);
        writer.Write(binary.MajorVersion);
        writer.Write(binary.ReservedUserId);
        writer.Write(binary.ReservedGroupId);
        writer.Write(new byte[940]); // (unused)
    }
}