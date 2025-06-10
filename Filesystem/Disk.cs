namespace Filesystem;

using Enums;
using Mapping;
using Models;
using Serialization.Models;
using Serialization.Serializers;
using System.Text;

public class Disk : IDisposable, IAsyncDisposable
{
    public const long SuperblockOffset = 1024;
    private readonly BinaryReader _reader;
    private readonly Stream _stream;
    private readonly BinaryWriter _writer;

    public Disk(Stream stream)
    {
        _stream = stream;
        _reader = new BinaryReader(_stream, Encoding.UTF8, true);
        _writer = new BinaryWriter(_stream, Encoding.UTF8, true);

        Superblock = ReadSuperblock();
        BlockGroupDescriptors = ReadBlockGroupDescriptorTable();
    }

    public Superblock Superblock { get; }
    public BlockGroupDescriptor[] BlockGroupDescriptors { get; }

    public uint InodeCount => Superblock.InodeCount;
    public uint BlockCount => Superblock.BlockCount;
    public uint BlockSize => Superblock.BlockSize;
    public uint FragmentSize => Superblock.FragmentSize;
    public uint BlocksPerGroup => Superblock.BlocksPerGroup;
    public uint InodesPerGroup => Superblock.InodesPerGroup;
    public uint BlockGroupCount => (uint)Math.Ceiling(BlockCount / (double)BlocksPerGroup);

    public long BlockBitmapSizeBytes => BlockSize;
    public long InodeBitmapSizeBytes => BlockSize;
    public long InodeTableSizeBytes => InodesPerGroup * Superblock.InodeSize;
    public long DataBlocksSizeBytes => BlocksPerGroup * BlockSize - InodeTableSizeBytes - InodeBitmapSizeBytes - BlockBitmapSizeBytes;
    private long BlockGroupDescriptorTableOffset => BlockSize == 1024 ? 2 * BlockSize : BlockSize;
    private long BlockGroupDescriptorCount => BlockSize / BinaryBlockGroupDescriptor.SizeOnDiskInBytes;
    public long GetBlockBitmapOffset(long blockGroupNumber)
    {
        if (blockGroupNumber == 0)
        {
            return BlockGroupDescriptorTableOffset + BlockSize;
        }

        return blockGroupNumber * BlocksPerGroup * BlockSize;
    }

    public long GetInodeBitmapOffset(long blockGroupNumber) => GetBlockBitmapOffset(blockGroupNumber) + BlockBitmapSizeBytes;
    public long GetInodeTableOffset(long blockGroupNumber) => GetInodeBitmapOffset(blockGroupNumber) + InodeBitmapSizeBytes;
    public long GetDataBlocksOffset(long blockGroupNumber) => GetInodeTableOffset(blockGroupNumber) + InodeTableSizeBytes;


    #region Superblock I/O

    public Superblock ReadSuperblock()
    {
        _stream.Seek(SuperblockOffset, SeekOrigin.Begin);

        return _reader.ReadSuperblock();
    }

    // public void WriteSuperblock(Superblock superblock)
    // {
    //     _stream.Seek(SuperblockOffset, SeekOrigin.Begin);
    //
    //     _writer.Write(superblock);
    //
    //     Superblock = superblock;
    // }

    #endregion


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
        return _reader.ReadBytes((int)BlockSize);
    }

    public void WriteBlock(uint blockNumber, byte[] data)
    {
        if (data.Length != BlockSize)
            throw new ArgumentException($"Data must be exactly {BlockSize} bytes long.", nameof(data));

        _stream.Seek(GetBlockOffset(blockNumber), SeekOrigin.Begin);
        _writer.Write(data);
    }

    private long GetBlockOffset(uint blockNumber) => blockNumber * BlockSize;

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
        
        while(reader.BaseStream.Position + BinaryInodeDirectory.DirectoryEntryFixedFieldsSize < stream.Length)
        {
            var entryStart = reader.BaseStream.Position;

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
        var blockPointers = new List<uint>(inode.BlockAddresses);
        if (inode.SinglyIndirectBlockAddress.HasValue)
        {
            blockPointers.AddRange(ReadIndirectBlockPointers(inode.SinglyIndirectBlockAddress.Value, 1));
        }
        if (inode.DoublyIndirectBlockAddress.HasValue)
        {
            blockPointers.AddRange(ReadIndirectBlockPointers(inode.DoublyIndirectBlockAddress.Value, 2));
        }
        if (inode.TriplyIndirectBlockAddress.HasValue)
        {
            blockPointers.AddRange(ReadIndirectBlockPointers(inode.TriplyIndirectBlockAddress.Value, 3));
        }
        return blockPointers.Where(ptr => ptr != 0);
    }
    
    public IEnumerable<uint> ReadIndirectBlockPointers(uint blockAddress, byte depth)
    {
        ArgumentOutOfRangeException.ThrowIfZero(blockAddress);

        _stream.Seek(GetBlockOffset(blockAddress), SeekOrigin.Begin);
        
        var pointerCount = BlockSize / 4;
        for (var i = 0; i < pointerCount; i++)
        {
            var ptr = _reader.ReadUInt32();
            if (ptr == 0)
            {
                continue;
            }
            
            if (depth > 1)
            {
                foreach (var subPtr in ReadIndirectBlockPointers(ptr, (byte)(depth - 1)))
                {
                    yield return subPtr;
                }
            } 
            else
            {
                yield return ptr;
            }
        }
    }

    #endregion


    #region Inode I/O

    public Inode ReadInode(uint inodeIndex)
    {
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(inodeIndex);
        ArgumentOutOfRangeException.ThrowIfGreaterThan(inodeIndex, InodeCount);

        _stream.Position = GetInodeOffset(inodeIndex);
        return _reader.ReadInode();
    }

    public Inode ReadRootInode() => ReadInode(2);

    public void WriteInode(Inode inode, uint index)
    {
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(index);
        ArgumentOutOfRangeException.ThrowIfGreaterThan(index, InodeCount);

        _stream.Position = GetInodeOffset(index);
        _writer.Write(inode);
    }

    private long GetInodeOffset(uint index)
    {
        var groupIndex = (index - 1) / InodesPerGroup;
        var localIndex = (index - 1) % InodesPerGroup;

        var descriptor = BlockGroupDescriptors[(int)groupIndex];
        var inodeTableBlock = descriptor.InodeTableStartingBlockAddress;

        var inodeTableOffset = GetBlockOffset(inodeTableBlock);
        return inodeTableOffset + localIndex * Superblock.InodeSize;
    }

    #endregion


    #region Block Bitmap I/O

    public Bitmap ReadBlockBitmap(BlockGroupDescriptor descriptor) =>
        new(ReadBlock(descriptor.BlockUsageBitmapBlockAddress), (int)BlocksPerGroup);

    public void WriteBlockBitmap(BlockGroupDescriptor descriptor, Bitmap bitmap) =>
        WriteBlock(descriptor.BlockUsageBitmapBlockAddress, bitmap.ToByteArray());

    #endregion


    #region Inode Bitmap I/O

    public Bitmap ReadInodeBitmap(BlockGroupDescriptor descriptor) =>
        new(ReadBlock(descriptor.InodeUsageBitmapBlockAddress), (int)InodesPerGroup);

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