namespace Filesystem;

using Models;
using Serializers;
using System.Text;

public class Disk : IDisposable, IAsyncDisposable
{
    public const long SuperblockOffset = 1024;

    public readonly Stream Stream;
    private readonly Superblock _superblock;
    private readonly BlockGroupDescriptor[] _blockGroupDescriptors;

    public uint InodeCount => _superblock.InodeCount;
    public uint BlockCount => _superblock.BlockCount;
    public uint BlockSize => _superblock.BlockSize;
    public uint FragmentSize => _superblock.FragmentSize;
    public uint BlocksPerGroup => _superblock.BlocksPerGroup;
    public uint InodesPerGroup => _superblock.InodesPerGroup;
    public uint BlockGroupCount => (uint)Math.Ceiling(BlockCount / (double)BlocksPerGroup);

    private uint BlockGroupDescriptorTableOffset => BlockSize == 1024 ? 2 * BlockSize : BlockSize;

    public Disk(Stream stream)
    {
        Stream = stream;

        Stream.Position = SuperblockOffset;
        using var reader = new BinaryReader(Stream, Encoding.UTF8, leaveOpen: true);
        _superblock = reader.ReadSuperblock();

        _blockGroupDescriptors = new BlockGroupDescriptor[BlockGroupCount];
        Stream.Position = BlockGroupDescriptorTableOffset;
        for (uint i = 0; i < BlockGroupCount; i++)
        {
            _blockGroupDescriptors[i] = reader.ReadBlockGroupDescriptor();
        }
    }

    public byte[] ReadBlock(int blockNumber)
    {
        var buffer = new byte[BlockSize];
        var offset = blockNumber * BlockSize;
        Stream.Seek(offset, SeekOrigin.Begin);
        Stream.ReadExactly(buffer, 0, (int)BlockSize);
        return buffer;
    }

    public void WriteBlock(int blockNumber, byte[] data)
    {
        if (data.Length != BlockSize)
            throw new ArgumentException($"Data must be exactly {BlockSize} bytes long.", nameof(data));

        var offset = blockNumber * BlockSize;
        Stream.Seek(offset, SeekOrigin.Begin);
        Stream.Write(data, 0, (int)BlockSize);
    }

    public Bitmap ReadBlockBitmap(BlockGroupDescriptor descriptor) =>
        new(ReadBlock((int)descriptor.BlockUsageBitmapBlockAddress), (int)BlocksPerGroup);

    public void WriteBlockBitmap(BlockGroupDescriptor descriptor, Bitmap bitmap) =>
        WriteBlock((int)descriptor.BlockUsageBitmapBlockAddress, bitmap.ToByteArray());

    public Bitmap ReadInodeBitmap(BlockGroupDescriptor descriptor) =>
        new(ReadBlock((int)descriptor.InodeUsageBitmapBlockAddress), (int)InodesPerGroup);

    public void WriteInodeBitmap(BlockGroupDescriptor descriptor, Bitmap bitmap) =>
        WriteBlock((int)descriptor.InodeUsageBitmapBlockAddress, bitmap.ToByteArray());

    public Inode ReadRootInode() => ReadInode(2);

    public Inode ReadInode(uint inodeIndex)
    {
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(inodeIndex);
        ArgumentOutOfRangeException.ThrowIfGreaterThan(inodeIndex, InodeCount);

        var groupIndex = (inodeIndex - 1) / InodesPerGroup;
        var localIndex = (inodeIndex - 1) % InodesPerGroup;

        var descriptor = _blockGroupDescriptors[(int)groupIndex];
        var inodeTableBlock = descriptor.InodeTableStartingBlockAddress;

        var inodeTableOffset = inodeTableBlock * BlockSize;

        Stream.Position = inodeTableOffset + localIndex * Inode.Size;

        using var reader = new BinaryReader(Stream, Encoding.UTF8, leaveOpen: true);
        return reader.ReadInode();
    }

    private void WriteInode(Inode inode, uint index)
    {
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(index);
        ArgumentOutOfRangeException.ThrowIfGreaterThan(index, InodeCount);

        Stream.Position = GetInodeOffset(index);
        using var writer = new BinaryWriter(Stream, Encoding.UTF8, leaveOpen: true);
        writer.Write(inode);
    }

    public long GetInodeOffset(uint index)
    {
        var groupIndex = (index - 1) / InodesPerGroup;
        var localIndex = (index - 1) % InodesPerGroup;

        var descriptor = _blockGroupDescriptors[(int)groupIndex];
        var inodeTableBlock = descriptor.InodeTableStartingBlockAddress;

        var inodeTableOffset = GetBlockOffset(inodeTableBlock);
        return inodeTableOffset + localIndex * Inode.Size;
    }

    public long GetBlockOffset(uint blockNumber) => blockNumber * BlockSize;

    #region IDisposable & IAsyncDisposable

    public void Dispose()
    {
        GC.SuppressFinalize(this);
        Stream.Dispose();
    }

    public async ValueTask DisposeAsync()
    {
        GC.SuppressFinalize(this);
        await Stream.DisposeAsync();
    }

    #endregion
}