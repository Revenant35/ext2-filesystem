namespace Filesystem.Serializers;

using Exceptions;
using Models;

// https://wiki.osdev.org/Ext2#Superblock
public static class SuperblockSerialization
{
    public static Superblock ReadSuperblock(this BinaryReader reader)
    {
        var inodeCount = reader.ReadUInt32();
        var blockCount = reader.ReadUInt32();
        var superuserReservedBlockCount = reader.ReadUInt32();
        var unallocatedBlockCount = reader.ReadUInt32();
        var unallocatedInodeCount = reader.ReadUInt32();
        var startingBlockNumber = reader.ReadUInt32();
        var blockSize = 1024u << Convert.ToInt32(reader.ReadInt32());
        var fragmentSize = 1024u << Convert.ToInt32(reader.ReadInt32());
        var blocksPerGroup = reader.ReadUInt32();
        var fragmentsPerGroup = reader.ReadUInt32();
        var inodesPerGroup = reader.ReadUInt32();
        var lastMountTime = DateTimeOffset.FromUnixTimeSeconds(reader.ReadUInt32());
        var lastWrittenTime = DateTimeOffset.FromUnixTimeSeconds(reader.ReadUInt32());
        var mountsSinceLastConsistencyCheck = reader.ReadUInt16();
        var mountsBeforeNextConsistencyCheck = reader.ReadUInt16();
        var signature = reader.ReadUInt16();
        if (signature != Superblock.Ext2Signature)
        {
            throw new SuperblockFormatException($"Invalid ext2 signature. Expected {Superblock.Ext2Signature}, got {signature}");
        }

        var rawFileSystemState = reader.ReadUInt16();
        if (!Enum.IsDefined(typeof(FileSystemState), rawFileSystemState))
        {
            throw new SuperblockFormatException($"Invalid file system state: {rawFileSystemState}");
        }

        var rawErrorHandling = reader.ReadUInt16();
        if (!Enum.IsDefined(typeof(ErrorHandling), rawErrorHandling))
        {
            throw new SuperblockFormatException($"Invalid error handling: {rawErrorHandling}");
        }

        var minorVersion = reader.ReadUInt16();
        var lastCheckTime = DateTimeOffset.FromUnixTimeSeconds(reader.ReadUInt32());
        var checkInterval = TimeSpan.FromSeconds(reader.ReadUInt32());
        var rawOperatingSystemId = reader.ReadUInt32();
        if (!Enum.IsDefined(typeof(OperatingSystemID), rawOperatingSystemId))
        {
            throw new SuperblockFormatException($"Invalid operating system ID: {rawOperatingSystemId}");
        }

        var majorVersion = reader.ReadUInt32();
        var reservedUserId = reader.ReadUInt16();
        var reservedGroupId = reader.ReadUInt16();

        reader.ReadBytes(940); // (unused)

        return new Superblock
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
            FileSystemState = (FileSystemState)rawFileSystemState,
            ErrorHandling = (ErrorHandling)rawErrorHandling,
            MinorVersion = minorVersion,
            LastCheckTime = lastCheckTime,
            CheckInterval = checkInterval,
            OperatingSystemId = (OperatingSystemID)rawOperatingSystemId,
            MajorVersion = majorVersion,
            ReservedUserId = reservedUserId,
            ReservedGroupId = reservedGroupId,
        };
    }
    
    public static void Write(this BinaryWriter writer, Superblock superblock)
    {
        writer.Write(superblock.InodeCount);
        writer.Write(superblock.BlockCount);
        writer.Write(superblock.SuperuserReservedBlockCount);
        writer.Write(superblock.UnallocatedBlockCount);
        writer.Write(superblock.UnallocatedInodeCount);
        writer.Write(superblock.StartingBlockNumber);
        writer.Write(uint.Log2(superblock.BlockSize) - 10);
        writer.Write(uint.Log2(superblock.FragmentSize) - 10);
        writer.Write(superblock.BlocksPerGroup);
        writer.Write(superblock.FragmentsPerGroup);
        writer.Write(superblock.InodesPerGroup);
        writer.Write(Convert.ToUInt32(superblock.LastMountTime.ToUnixTimeSeconds()));
        writer.Write(Convert.ToUInt32(superblock.LastWrittenTime.ToUnixTimeSeconds()));
        writer.Write(superblock.MountsSinceLastConsistencyCheck);
        writer.Write(superblock.MountsBeforeNextConsistencyCheck);
        writer.Write(Superblock.Ext2Signature);
        writer.Write((ushort)superblock.FileSystemState);
        writer.Write((ushort)superblock.ErrorHandling);
        writer.Write(superblock.MinorVersion);
        writer.Write(Convert.ToUInt32(superblock.LastCheckTime.ToUnixTimeSeconds()));
        writer.Write(Convert.ToUInt32(superblock.CheckInterval.TotalSeconds));
        writer.Write((uint)superblock.OperatingSystemId);
        writer.Write(superblock.MajorVersion);
        writer.Write(superblock.ReservedUserId);
        writer.Write(superblock.ReservedGroupId);
        writer.Write(new byte[940]); // (unused)
    }
}