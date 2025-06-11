namespace Filesystem.Services;

using AutoMapper;
using Models;
using System.Text;

public class BlockGroupDescriptorService : IBlockGroupDescriptorService
{
    private readonly Stream _stream;
    private readonly ISuperblockService _superblockService;
    private readonly IMapper _mapper;
    
    public BlockGroupDescriptor[] BlockGroupDescriptors { get; private set; }
    
    private long BlockGroupDescriptorCount => _superblockService.BlockSize / BinaryBlockGroupDescriptor.SizeOnDiskInBytes;
    private long BlockGroupDescriptorTableOffset => _superblockService.BlockSize == 1024 ? 2 * _superblockService.BlockSize : _superblockService.BlockSize;

    public BlockGroupDescriptorService(
        Stream stream, 
        ISuperblockService superblockService,
        IMapper mapper
    )
    {
        _stream = stream;
        _superblockService = superblockService;
        _mapper = mapper;
        
        BlockGroupDescriptors = ReadBlockGroupDescriptorTable();
    }
    
    public BlockGroupDescriptor[] ReadBlockGroupDescriptorTable()
    {
        var descriptors = new BlockGroupDescriptor[BlockGroupDescriptorCount];
        var previousPosition = _stream.Position;
        _stream.Seek(BlockGroupDescriptorTableOffset, SeekOrigin.Begin);
        using var reader = new BinaryReader(_stream, Encoding.UTF8, true);
        
        for (var i = 0; i < BlockGroupDescriptorCount; i++)
        {
            descriptors[i] = ReadBlockGroupDescriptor(reader);
        }

        _stream.Seek(previousPosition, SeekOrigin.Begin);
        
        return descriptors;
    }
    
    public void WriteBlockGroupDescriptorTable(BlockGroupDescriptor[] descriptors)
    {
        ArgumentOutOfRangeException.ThrowIfNotEqual(BlockGroupDescriptorCount, descriptors.Length);
    
        var previousPosition = _stream.Position;
        _stream.Seek(BlockGroupDescriptorTableOffset, SeekOrigin.Begin);
        using var writer = new BinaryWriter(_stream, Encoding.UTF8, true);
        
        foreach (var descriptor in descriptors)
        {
            Write(writer, descriptor);
        }
    
        _stream.Flush();
        _stream.Seek(previousPosition, SeekOrigin.Begin);
        BlockGroupDescriptors = descriptors;
    }
    
    private BlockGroupDescriptor ReadBlockGroupDescriptor(BinaryReader reader)
    {
        var blockUsageBitmapBlockAddress = reader.ReadUInt32();
        var inodeUsageBitmapBlockAddress = reader.ReadUInt32();
        var inodeTableStartingBlockAddress = reader.ReadUInt32();
        var unallocatedBlocksInGroup = reader.ReadUInt16();
        var unallocatedInodesInGroup = reader.ReadUInt16();
        var directoriesInGroup = reader.ReadUInt16();
        reader.ReadBytes(14); // (unused)

        var binary = new BinaryBlockGroupDescriptor
        {
            BlockUsageBitmapBlockAddress = blockUsageBitmapBlockAddress,
            InodeUsageBitmapBlockAddress = inodeUsageBitmapBlockAddress,
            InodeTableStartingBlockAddress = inodeTableStartingBlockAddress,
            UnallocatedBlocksInGroup = unallocatedBlocksInGroup,
            UnallocatedInodesInGroup = unallocatedInodesInGroup,
            DirectoriesInGroup = directoriesInGroup,
        };

        return _mapper.Map<BlockGroupDescriptor>(binary);
    }

    private void Write(BinaryWriter writer, BlockGroupDescriptor blockGroupDescriptor)
    {
        var binary = _mapper.Map<BinaryBlockGroupDescriptor>(blockGroupDescriptor);

        writer.Write(binary.BlockUsageBitmapBlockAddress);
        writer.Write(binary.InodeUsageBitmapBlockAddress);
        writer.Write(binary.InodeTableStartingBlockAddress);
        writer.Write(binary.UnallocatedBlocksInGroup);
        writer.Write(binary.UnallocatedInodesInGroup);
        writer.Write(binary.DirectoriesInGroup);
        writer.Write(new byte[14]); // (unused)
    }
}