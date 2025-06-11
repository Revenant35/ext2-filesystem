namespace Filesystem.Services;

using AutoMapper;
using Enums;
using Models;
using Serialization.Models;
using Serialization.Serializers;
using System.Text;

public class InodeService : IInodeService
{
    private readonly IBlockGroupDescriptorService _blockGroupDescriptorService;
    private readonly ISuperblockService _superblockService;
    private readonly Stream _stream;
    private readonly IMapper _mapper;
    
    public InodeService(
        Stream stream,
        ISuperblockService superblockService,
        IBlockGroupDescriptorService blockGroupDescriptorService,
        IMapper mapper
    )
    {
        _stream = stream;
        _superblockService = superblockService;
        _blockGroupDescriptorService = blockGroupDescriptorService;
        _mapper = mapper;
    }
    
    public Inode ReadInode(uint inodeIndex)
    {
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(inodeIndex);
        ArgumentOutOfRangeException.ThrowIfGreaterThan(inodeIndex, _superblockService.InodeCount);
        
        var previousPosition = _stream.Position;
        _stream.Seek(GetInodeOffset(inodeIndex), SeekOrigin.Begin);
        using var reader = new BinaryReader(_stream, Encoding.UTF8, true);

        var inode = ReadInode(reader);
        
        _stream.Seek(previousPosition, SeekOrigin.Begin);
        return inode;
    }

    public Inode ReadRootInode() => ReadInode(2);

    public void WriteInode(Inode inode, uint index)
    {
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(index);
        ArgumentOutOfRangeException.ThrowIfGreaterThan(index, _superblockService.InodeCount);

        var previousPosition = _stream.Position;
        _stream.Seek(GetInodeOffset(index), SeekOrigin.Begin);
        using var writer = new BinaryWriter(_stream, Encoding.UTF8, true);
        
        Write(writer, inode);
        
        _stream.Flush();
        _stream.Seek(previousPosition, SeekOrigin.Begin);
    }

    private long GetInodeOffset(uint index)
    {
        var groupIndex = (index - 1) / _superblockService.InodesPerGroup;
        var localIndex = (index - 1) % _superblockService.InodesPerGroup;

        var descriptor = _blockGroupDescriptorService.BlockGroupDescriptors[(int)groupIndex];
        var inodeTableBlock = descriptor.InodeTableStartingBlockAddress;

        var inodeTableOffset = inodeTableBlock * _superblockService.BlockSize;
        return inodeTableOffset + localIndex * _superblockService.InodeSize;
    }
    
    private Inode ReadInode(BinaryReader reader)
    {
        var typeAndPermissions = reader.ReadUInt16();
        var inodeType = typeAndPermissions & 0xF000;
        if (!Enum.IsDefined(typeof(InodeType), (ushort)inodeType))
        {
            throw new Exception($"Invalid inode type: {inodeType}");
        }

        var userID = reader.ReadUInt16();
        var sizeInBytes = reader.ReadUInt32();
        var lastAccessed = reader.ReadUInt32();
        var createdAt = reader.ReadUInt32();
        var lastModified = reader.ReadUInt32();
        var deletedAt = reader.ReadUInt32();
        var groupID = reader.ReadUInt16();
        var hardLinkCount = reader.ReadUInt16();
        var diskSectorCount = reader.ReadUInt32();
        var flags = reader.ReadUInt32();
        if (!Enum.IsDefined(typeof(InodeFlags), flags))
        {
            throw new Exception($"Invalid inode flags: {flags}");
        }
        
        var osd1LinuxReserved = reader.ReadBytes(4);

        var blockAddresses = new uint[12];
        for (var i = 0; i < 12; i++)
        {
            blockAddresses[i] = reader.ReadUInt32();
        }

        var singlyIndirectBlockAddress = reader.ReadUInt32();
        var doublyIndirectBlockAddress = reader.ReadUInt32();
        var triplyIndirectBlockAddress = reader.ReadUInt32();

        var generationNumber = reader.ReadUInt32();
        var fileACLBlock = reader.ReadUInt32();
        var directoryACLBlock = reader.ReadUInt32();
        var fragmentBlockAddress = reader.ReadUInt32();
        var osd2LinuxSpecific = reader.ReadBytes(12);

        var binary = new BinaryInode
        {
            TypeAndPermissions = typeAndPermissions,
            UserID = userID,
            SizeBytes = sizeInBytes,
            LastAccessed = lastAccessed,
            CreatedAt = createdAt,
            LastModified = lastModified,
            DeletedAt = deletedAt,
            GroupID = groupID,
            HardLinkCount = hardLinkCount,
            DiskSectorCount = diskSectorCount,
            Flags = flags,
            BlockAddresses = blockAddresses,
            SinglyIndirectBlockAddress = singlyIndirectBlockAddress,
            DoublyIndirectBlockAddress = doublyIndirectBlockAddress,
            TriplyIndirectBlockAddress = triplyIndirectBlockAddress,
            GenerationNumber = generationNumber,
            FileACLBlock = fileACLBlock,
            DirectoryACLBlock = directoryACLBlock,
            FragmentBlockAddress = fragmentBlockAddress,
            OperatingSystemSpecificValues = [..osd1LinuxReserved, ..osd2LinuxSpecific],
        };

        return _mapper.Map<Inode>(binary);
    }

    private void Write(BinaryWriter writer, Inode inode)
    {
        var binary = _mapper.Map<BinaryInode>(inode);

        writer.Write(binary.TypeAndPermissions);
        writer.Write(binary.UserID);
        writer.Write(binary.SizeBytes);
        writer.Write(binary.LastAccessed);
        writer.Write(binary.CreatedAt);
        writer.Write(binary.LastModified);
        writer.Write(binary.DeletedAt);
        writer.Write(binary.GroupID);
        writer.Write(binary.HardLinkCount);
        writer.Write(binary.DiskSectorCount);
        writer.Write(binary.Flags);
        writer.Write(binary.OperatingSystemSpecificValues.AsSpan(0, 4));

        foreach (var blockPointer in binary.BlockAddresses)
        {
            writer.Write(blockPointer);
        }

        writer.Write(binary.SinglyIndirectBlockAddress);
        writer.Write(binary.DoublyIndirectBlockAddress);
        writer.Write(binary.TriplyIndirectBlockAddress);
        writer.Write(binary.GenerationNumber);
        writer.Write(binary.FileACLBlock);
        writer.Write(binary.DirectoryACLBlock);
        writer.Write(binary.FragmentBlockAddress);
        writer.Write(binary.OperatingSystemSpecificValues.AsSpan(4, 12));
    }
}