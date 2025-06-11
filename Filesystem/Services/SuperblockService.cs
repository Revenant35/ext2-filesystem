using AutoMapper;
using System.Text;

namespace Filesystem.Services;

using Enums;
using Models;

public class SuperblockService : ISuperblockService
{
    public const long SuperblockOffset = 1024;
    
    public Superblock Superblock { get; private set; }
    
    public uint InodeCount => Superblock.InodeCount;
    public uint InodeSize => Superblock.InodeSize;
    public uint BlockCount => Superblock.BlockCount;
    public uint BlockSize => Superblock.BlockSize;
    public uint FragmentSize => Superblock.FragmentSize;
    public uint BlocksPerGroup => Superblock.BlocksPerGroup;
    public uint InodesPerGroup => Superblock.InodesPerGroup;

    private readonly Stream _stream;
    private readonly IMapper _mapper;

    public SuperblockService(Stream stream, IMapper mapper)
    {
        _stream = stream;
        _mapper = mapper;
        Superblock = ReadSuperblock();
    }
    
    public Superblock ReadSuperblock()
    {
        var previousPosition = _stream.Position;
        _stream.Seek(SuperblockOffset, SeekOrigin.Begin);
        using var reader = new BinaryReader(_stream, Encoding.UTF8, true);

        var superblock = ReadSuperblock(reader);
        _stream.Seek(previousPosition, SeekOrigin.Begin);

        return superblock;
    }

    public void WriteSuperblock(Superblock superblock)
    {
        var previousPosition = _stream.Position;
        _stream.Seek(SuperblockOffset, SeekOrigin.Begin);
        using var writer = new BinaryWriter(_stream, Encoding.UTF8, true);
    
        Write(writer, superblock);
        _stream.Flush();
        _stream.Seek(previousPosition, SeekOrigin.Begin);
        
        Superblock = superblock;
    }
    
    private Superblock ReadSuperblock(BinaryReader reader)
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
        var firstNonReservedInode = reader.ReadUInt32();
        var inodeSize = reader.ReadUInt16();
        var blockGroupNumber = reader.ReadUInt16();
        var optionalFeatures = reader.ReadUInt32();
        var requiredFeatures = reader.ReadUInt32();
        var incompatibleFeatures = reader.ReadUInt32();
        var fileSystemId = reader.ReadBytes(16);
        var volumeName = reader.ReadBytes(16);
        var lastMountedPath = reader.ReadBytes(64);
        var compressionAlgorithms = reader.ReadUInt32();
        var preallocatedBlocksForFiles = reader.ReadByte();
        var preallocatedBlocksForDirectories = reader.ReadByte();
        reader.ReadBytes(2);
        var journalId = reader.ReadBytes(16);
        var journalInode = reader.ReadUInt32();
        var journalDevice = reader.ReadUInt32();

        reader.ReadBytes(788); // (unused)

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
            FirstNonReservedInode = firstNonReservedInode,
            InodeSize = inodeSize,
            BlockGroupNumber = blockGroupNumber,
            OptionalFeatures = optionalFeatures,
            RequiredFeatures = requiredFeatures,
            IncompatibleFeatures = incompatibleFeatures,
            FileSystemId = fileSystemId,
            VolumeName = volumeName,
            LastMountedPath = lastMountedPath,
            CompressionAlgorithms = compressionAlgorithms,
            PreallocatedBlocksForFiles = preallocatedBlocksForFiles,
            PreallocatedBlocksForDirectories = preallocatedBlocksForDirectories,
            JournalId = journalId,
            JournalInode = journalInode,
            JournalDevice = journalDevice,
        };

        return _mapper.Map<Superblock>(binary);
    }

    private void Write(BinaryWriter writer, Superblock superblock)
    {
        var binary = _mapper.Map<BinarySuperblock>(superblock);

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
        writer.Write(binary.FirstNonReservedInode);
        writer.Write(binary.InodeSize);
        writer.Write(binary.BlockGroupNumber);
        writer.Write(binary.OptionalFeatures);
        writer.Write(binary.RequiredFeatures);
        writer.Write(binary.IncompatibleFeatures);
        writer.Write(binary.FileSystemId);
        writer.Write(binary.VolumeName);
        writer.Write(binary.LastMountedPath);
        writer.Write(binary.CompressionAlgorithms);
        writer.Write(binary.PreallocatedBlocksForFiles);
        writer.Write(binary.PreallocatedBlocksForDirectories);
        writer.Write(new byte[2]); // (unused)
        writer.Write(binary.JournalId);
        writer.Write(binary.JournalInode);
        writer.Write(binary.JournalDevice);
        writer.Write(new byte[788]); // (unused)
    }
}