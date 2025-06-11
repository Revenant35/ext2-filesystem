namespace Filesystem.Services;

using Models;
using Serialization.Serializers;
using System.Text;

public class InodeService : IInodeService
{
    private readonly IBlockGroupDescriptorService _blockGroupDescriptorService;
    private readonly ISuperblockService _superblockService;
    private readonly Stream _stream;
    
    public InodeService(
        Stream stream,
        ISuperblockService superblockService,
        IBlockGroupDescriptorService blockGroupDescriptorService
    )
    {
        _stream = stream;
        _superblockService = superblockService;
        _blockGroupDescriptorService = blockGroupDescriptorService;
    }
    
    public Inode ReadInode(uint inodeIndex)
    {
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(inodeIndex);
        ArgumentOutOfRangeException.ThrowIfGreaterThan(inodeIndex, _superblockService.InodeCount);
        
        var previousPosition = _stream.Position;
        _stream.Seek(GetInodeOffset(inodeIndex), SeekOrigin.Begin);
        using var reader = new BinaryReader(_stream, Encoding.UTF8, true);

        var inode = reader.ReadInode();
        
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
        
        writer.Write(inode);
        
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
}