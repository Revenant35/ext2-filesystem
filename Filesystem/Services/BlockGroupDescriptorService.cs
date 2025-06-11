namespace Filesystem.Services;

using Models;
using Serialization.Models;
using Serialization.Serializers;
using System.Text;

public class BlockGroupDescriptorService : IBlockGroupDescriptorService
{
    private readonly Stream _stream;
    private readonly ISuperblockService _superblockService;
    
    public BlockGroupDescriptor[] BlockGroupDescriptors { get; private set; }
    
    private long BlockGroupDescriptorCount => _superblockService.BlockSize / BinaryBlockGroupDescriptor.SizeOnDiskInBytes;
    private long BlockGroupDescriptorTableOffset => _superblockService.BlockSize == 1024 ? 2 * _superblockService.BlockSize : _superblockService.BlockSize;

    public BlockGroupDescriptorService(Stream stream, ISuperblockService superblockService)
    {
        _stream = stream;
        _superblockService = superblockService;
        
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
            descriptors[i] = reader.ReadBlockGroupDescriptor();
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
            writer.Write(descriptor);
        }
    
        _stream.Flush();
        _stream.Seek(previousPosition, SeekOrigin.Begin);
        BlockGroupDescriptors = descriptors;
    }
}