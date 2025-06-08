namespace Filesystem;

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
    public long InodeTableSizeBytes => InodesPerGroup * BinaryInode.SizeOnDiskInBytes;
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

    public byte[] ReadBlock(int blockNumber)
    {
        _stream.Seek(GetBlockOffset(blockNumber), SeekOrigin.Begin);
        return _reader.ReadBytes((int)BlockSize);
    }

    public void WriteBlock(int blockNumber, byte[] data)
    {
        if (data.Length != BlockSize)
            throw new ArgumentException($"Data must be exactly {BlockSize} bytes long.", nameof(data));

        _stream.Seek(GetBlockOffset(blockNumber), SeekOrigin.Begin);
        _writer.Write(data);
    }

    private long GetBlockOffset(int blockNumber) => blockNumber * BlockSize;

    #endregion


    #region Inode I/O

    public Inode ReadInode(uint inodeIndex)
    {
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(inodeIndex);
        ArgumentOutOfRangeException.ThrowIfGreaterThan(inodeIndex, InodeCount);

        var groupIndex = (inodeIndex - 1) / InodesPerGroup;
        var localIndex = (inodeIndex - 1) % InodesPerGroup;

        var descriptor = BlockGroupDescriptors[(int)groupIndex];
        var inodeTableBlock = descriptor.InodeTableStartingBlockAddress;

        var inodeTableOffset = inodeTableBlock * BlockSize;

        _stream.Position = inodeTableOffset + localIndex * BinaryInode.SizeOnDiskInBytes;

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

        var inodeTableOffset = GetBlockOffset((int)inodeTableBlock);
        return inodeTableOffset + localIndex * BinaryInode.SizeOnDiskInBytes;
    }

    #endregion


    #region Block Bitmap I/O

    public Bitmap ReadBlockBitmap(BlockGroupDescriptor descriptor) =>
        new(ReadBlock((int)descriptor.BlockUsageBitmapBlockAddress), (int)BlocksPerGroup);

    public void WriteBlockBitmap(BlockGroupDescriptor descriptor, Bitmap bitmap) =>
        WriteBlock((int)descriptor.BlockUsageBitmapBlockAddress, bitmap.ToByteArray());

    #endregion


    #region Inode Bitmap I/O

    public Bitmap ReadInodeBitmap(BlockGroupDescriptor descriptor) =>
        new(ReadBlock((int)descriptor.InodeUsageBitmapBlockAddress), (int)InodesPerGroup);

    public void WriteInodeBitmap(BlockGroupDescriptor descriptor, Bitmap bitmap) =>
        WriteBlock((int)descriptor.InodeUsageBitmapBlockAddress, bitmap.ToByteArray());

    #endregion


    #region IDisposable & IAsyncDisposable

    public void Dispose()
    {
        GC.SuppressFinalize(this);
        _writer.Dispose();
        _reader.Dispose();
        _stream.Dispose();
    }

    public async ValueTask DisposeAsync()
    {
        GC.SuppressFinalize(this);
        _reader.Dispose();
        await _writer.DisposeAsync();
        await _stream.DisposeAsync();
    }

    #endregion
}