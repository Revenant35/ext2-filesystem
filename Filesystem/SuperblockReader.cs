namespace Filesystem;

using Exceptions;
using Models;
using System.Text;

// https://wiki.osdev.org/Ext2#Superblock
public class SuperblockReader(Stream stream, Encoding encoding, bool leaveOpen = false)
{
    public Superblock Read()
    {
        using var reader = new BinaryReader(stream, encoding, leaveOpen);

        var inodeCount = reader.ReadUInt32();
        var blockCount = reader.ReadUInt32();
        var superuserReservedBlockCount = reader.ReadUInt32();
        var unallocatedBlockCount = reader.ReadUInt32();
        var unallocatedInodeCount = reader.ReadUInt32();
        var startingBlockNumber = reader.ReadUInt32();
        var blockSize = 1024 << Convert.ToInt32(reader.ReadInt32());
        var fragmentSize = 1024 << Convert.ToInt32(reader.ReadInt32());
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

        var fileSystemState = reader.ReadUInt16();
        if (fileSystemState is < 1 or > 2)
        {
            throw new SuperblockFormatException($"Invalid file system state: {fileSystemState}");
        }

        var errorHandling = reader.ReadUInt16();
        if (errorHandling is < 1 or > 3)
        {
            throw new SuperblockFormatException($"Invalid error handling: {errorHandling}");
        }

        var minorVersion = reader.ReadUInt16();
        var lastCheckTime = DateTimeOffset.FromUnixTimeSeconds(reader.ReadUInt32());
        var checkInterval = TimeSpan.FromSeconds(reader.ReadUInt32());
        var operatingSystemId = reader.ReadUInt32();
        if (operatingSystemId > 4)
        {
            throw new SuperblockFormatException($"Invalid operating system ID: {operatingSystemId}");
        }

        var majorVersion = reader.ReadUInt32();
        var reservedUserId = reader.ReadUInt16();
        var reservedGroupId = reader.ReadUInt16();

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
            FileSystemState = (FileSystemState)fileSystemState,
            ErrorHandling = (ErrorHandling)errorHandling,
            MinorVersion = minorVersion,
            LastCheckTime = lastCheckTime,
            CheckInterval = checkInterval,
            OperatingSystemId = (OperatingSystemID)operatingSystemId,
            MajorVersion = majorVersion,
            ReservedUserId = reservedUserId,
            ReservedGroupId = reservedGroupId,
        };
    }
}