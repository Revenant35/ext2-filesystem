namespace Filesystem;

using Enums;
using Mapping;
using Models;
using Serialization.Models;
using Serialization.Serializers;
using Services;
using System.Text;

public class Disk : IDisposable, IAsyncDisposable
{
    private readonly BinaryReader _reader;
    private readonly Stream _stream;
    private readonly SuperblockService _superblockService;
    private readonly BinaryWriter _writer;

    public BlockGroupDescriptor[] BlockGroupDescriptors { get; }
        
    public long BlockBitmapSizeBytes => _superblockService.BlockSize;
    public long InodeBitmapSizeBytes => _superblockService.BlockSize;
    public long InodeTableSizeBytes => _superblockService.InodesPerGroup * _superblockService.InodeSize;
    public long DataBlocksSizeBytes => _superblockService.BlocksPerGroup * _superblockService.BlockSize - InodeTableSizeBytes - InodeBitmapSizeBytes - BlockBitmapSizeBytes;
    public uint BlockGroupCount => (uint)Math.Ceiling(_superblockService.BlockCount / (double)_superblockService.BlocksPerGroup);

    private long BlockGroupDescriptorCount => _superblockService.BlockSize / BinaryBlockGroupDescriptor.SizeOnDiskInBytes;
    private long BlockGroupDescriptorTableOffset => _superblockService.BlockSize == 1024 ? 2 * _superblockService.BlockSize : _superblockService.BlockSize;
        
    public long GetBlockBitmapOffset(long blockGroupNumber)
    {
        if (blockGroupNumber == 0)
        {
            return BlockGroupDescriptorTableOffset + _superblockService.BlockSize;
        }

        return blockGroupNumber * _superblockService.BlocksPerGroup * _superblockService.BlockSize;
    }

    public long GetInodeBitmapOffset(long blockGroupNumber) => GetBlockBitmapOffset(blockGroupNumber) + BlockBitmapSizeBytes;
    public long GetInodeTableOffset(long blockGroupNumber) => GetInodeBitmapOffset(blockGroupNumber) + InodeBitmapSizeBytes;
    public long GetDataBlocksOffset(long blockGroupNumber) => GetInodeTableOffset(blockGroupNumber) + InodeTableSizeBytes;

    
    public Disk(Stream stream, SuperblockService superblockService)
    {
        _stream = stream;
        _superblockService = superblockService;
        _reader = new BinaryReader(_stream, Encoding.UTF8, true);
        _writer = new BinaryWriter(_stream, Encoding.UTF8, true);

        _superblockService.ReadSuperblock();
        BlockGroupDescriptors = ReadBlockGroupDescriptorTable();
    }




    #region BlockGroupDescriptor I/O

    public BlockGroupDescriptor[] ReadBlockGroupDescriptorTable()
    {
        var descriptors = new BlockGroupDescriptor[BlockGroupDescriptorCount];

        _stream.Seek(BlockGroupDescriptorTableOffset, SeekOrigin.Begin);
        for (var i = 0; i < BlockGroupDescriptorCount; i++)
        {
            descriptors[i] = _reader.ReadBlockGroupDescriptor();
        }

        return descriptors;
    }

    // public void WriteBlockGroupDescriptorTable(BlockGroupDescriptor[] descriptors)
    // {
    //     ArgumentOutOfRangeException.ThrowIfNotEqual(BlockGroupDescriptorCount, descriptors.Length);
    //
    //     _stream.Seek(BlockGroupDescriptorTableOffset, SeekOrigin.Begin);
    //     foreach (var descriptor in descriptors)
    //     {
    //         _writer.Write(descriptor);
    //     }
    //
    //     BlockGroupDescriptors = descriptors;
    // }

    #endregion


    #region Block I/O

    public byte[] ReadBlock(uint blockNumber)
    {
        _stream.Seek(GetBlockOffset(blockNumber), SeekOrigin.Begin);
        return _reader.ReadBytes((int)_superblockService.BlockSize);
    }

    public void WriteBlock(uint blockNumber, byte[] data)
    {
        if (data.Length != _superblockService.BlockSize)
            throw new ArgumentException($"Data must be exactly {_superblockService.BlockSize} bytes long.", nameof(data));

        _stream.Seek(GetBlockOffset(blockNumber), SeekOrigin.Begin);
        _writer.Write(data);
    }

    private long GetBlockOffset(uint blockNumber) => blockNumber * _superblockService.BlockSize;

    #endregion


    #region Directory I/O
    
    public IEnumerable<InodeDirectory> ReadRootDirectoryEntries() => ReadDirectoryEntries(ReadRootInode());

    public IEnumerable<InodeDirectory> ReadDirectoryEntries(Inode inode)
    {
        if (inode.Type != InodeType.Directory)
            throw new ArgumentException("Inode is not a directory.", nameof(inode));

        return GetAllDataBlockPointers(inode)
            .Select(ReadBlock)
            .SelectMany(ReadDirectoryEntries);
    }
    
    private IEnumerable<InodeDirectory> ReadDirectoryEntries(byte[] block)
    {
        var stream = new MemoryStream(block);
        using var reader = new BinaryReader(stream, Encoding.UTF8, false);
        
        while(stream.Position + BinaryInodeDirectory.DirectoryEntryFixedFieldsSize < stream.Length)
        {
            var entryStart = stream.Position;

            var inodeAddress = reader.ReadUInt32();
            var entrySize = reader.ReadUInt16();
            var nameLength = reader.ReadByte();
            var directoryType = reader.ReadByte();

            if (entrySize < BinaryInodeDirectory.DirectoryEntryFixedFieldsSize || entryStart + entrySize > stream.Length)
            {
                yield break;
            }

            var nameBytes = reader.ReadBytes(nameLength);

            var padding = entrySize - (BinaryInodeDirectory.DirectoryEntryFixedFieldsSize + nameLength);
            if (padding > 0)
            {
                reader.ReadBytes(padding);
            }

            if (inodeAddress == 0)
            {
                continue;
            }
            
            var binary = new BinaryInodeDirectory
            {
                InodeAddress = inodeAddress,
                EntrySize = entrySize,
                NameLength = nameLength,
                Type = directoryType,
                Name = nameBytes,
            };

            yield return binary.ToInodeDirectory();
        }
    }

    
    private IEnumerable<uint> GetAllDataBlockPointers(Inode inode)
    {
        return inode.BlockAddresses
            .Concat(Indirect(inode.SinglyIndirectBlockAddress, 1))
            .Concat(Indirect(inode.DoublyIndirectBlockAddress, 2))
            .Concat(Indirect(inode.TriplyIndirectBlockAddress, 3))
            .Where(ptr => ptr != 0);

        IEnumerable<uint> Indirect(uint? address, byte depth) =>
            address.HasValue ? ReadIndirectBlockPointers(address.Value, depth) : [];
    }
    
    private IEnumerable<uint> ReadIndirectBlockPointers(uint blockAddress, byte depth)
    {
        ArgumentOutOfRangeException.ThrowIfZero(depth);

        var block = ReadBlock(blockAddress);
        using var stream = new MemoryStream(block);
        using var reader = new BinaryReader(stream, Encoding.UTF8, false);
        
        for (var i = 0; i < _superblockService.BlockSize / 4; i++)
        {
            var ptr = reader.ReadUInt32();
            if (ptr == 0) continue;
            if (depth == 1) yield return ptr;
            foreach (var subPtr in ReadIndirectBlockPointers(ptr, (byte)(depth - 1)))
            {
                yield return subPtr;
            }
        }
    }

    #endregion


    #region Inode I/O

    public Inode ReadInode(uint inodeIndex)
    {
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(inodeIndex);
        ArgumentOutOfRangeException.ThrowIfGreaterThan(inodeIndex, _superblockService.InodeCount);

        _stream.Position = GetInodeOffset(inodeIndex);
        return _reader.ReadInode();
    }

    public Inode ReadRootInode() => ReadInode(2);

    public void WriteInode(Inode inode, uint index)
    {
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(index);
        ArgumentOutOfRangeException.ThrowIfGreaterThan(index, _superblockService.InodeCount);

        _stream.Position = GetInodeOffset(index);
        _writer.Write(inode);
    }

    private long GetInodeOffset(uint index)
    {
        var groupIndex = (index - 1) / _superblockService.InodesPerGroup;
        var localIndex = (index - 1) % _superblockService.InodesPerGroup;

        var descriptor = BlockGroupDescriptors[(int)groupIndex];
        var inodeTableBlock = descriptor.InodeTableStartingBlockAddress;

        var inodeTableOffset = GetBlockOffset(inodeTableBlock);
        return inodeTableOffset + localIndex * _superblockService.InodeSize;
    }

    #endregion


    #region Block Bitmap I/O

    public Bitmap ReadBlockBitmap(BlockGroupDescriptor descriptor) =>
        new(ReadBlock(descriptor.BlockUsageBitmapBlockAddress), (int)_superblockService.BlocksPerGroup);

    public void WriteBlockBitmap(BlockGroupDescriptor descriptor, Bitmap bitmap) =>
        WriteBlock(descriptor.BlockUsageBitmapBlockAddress, bitmap.ToByteArray());

    #endregion


    #region Inode Bitmap I/O

    public Bitmap ReadInodeBitmap(BlockGroupDescriptor descriptor) =>
        new(ReadBlock(descriptor.InodeUsageBitmapBlockAddress), (int)_superblockService.InodesPerGroup);

    public void WriteInodeBitmap(BlockGroupDescriptor descriptor, Bitmap bitmap) =>
        WriteBlock(descriptor.InodeUsageBitmapBlockAddress, bitmap.ToByteArray());

    #endregion


    #region IDisposable & IAsyncDisposable

    public void Dispose()
    {
        GC.SuppressFinalize(this);
        _writer.Dispose();
        _reader.Dispose();
    }

    public async ValueTask DisposeAsync()
    {
        GC.SuppressFinalize(this);
        _reader.Dispose();
        await _writer.DisposeAsync();
    }

    #endregion
}