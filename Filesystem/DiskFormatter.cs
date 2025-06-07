namespace Filesystem;

using Models;
using Serializers;
using System.Text;

public class DiskFormatter(Stream stream)
{
    public void Format(Superblock superblock)
    {
        stream.Position = Disk.SuperblockOffset;
        using (var writer = new BinaryWriter(stream, Encoding.UTF8, leaveOpen: true))
        {
            writer.Write(superblock);
        }

        var blockCount = superblock.BlockCount;
        var blocksPerGroup = superblock.BlocksPerGroup;
        var blockSize = superblock.BlockSize;
        var inodesPerGroup = superblock.InodesPerGroup;

        var blockGroupCount = (uint)Math.Ceiling(blockCount / (double)blocksPerGroup);

        var descriptorOffset = blockSize == 1024 ? 2 * blockSize : blockSize;
        stream.Position = descriptorOffset;

        using (var writer = new BinaryWriter(stream, Encoding.UTF8, leaveOpen: true))
        {
            for (uint i = 0; i < blockGroupCount; i++)
            {
                var blockGroup = new BlockGroupDescriptor
                {
                    BlockUsageBitmapBlockAddress = 5 + i * 10,
                    InodeUsageBitmapBlockAddress = 6 + i * 10,
                    InodeTableStartingBlockAddress = 7 + i * 10,
                    UnallocatedBlocksInGroup = (ushort)blocksPerGroup,
                    UnallocatedInodesInGroup = (ushort)inodesPerGroup,
                    DirectoriesInGroup = 0
                };

                writer.Write(blockGroup);
            }
        }

        for (uint i = 0; i < blockGroupCount; i++)
        {
            var blockBitmapBlock = 5 + i * 10;
            var inodeBitmapBlock = 6 + i * 10;
            var inodeTableStart = 7 + i * 10;

            ZeroBlock(blockBitmapBlock, superblock.BlockSize);
            ZeroBlock(inodeBitmapBlock, superblock.BlockSize);

            for (var j = 0; j < (inodesPerGroup * Inode.Size + blockSize - 1) / blockSize; j++)
            {
                ZeroBlock(inodeTableStart + (uint)j, superblock.BlockSize);
            }
        }

        stream.Flush();
    }
    
    private void ZeroBlock(uint blockNumber, uint blockSize)
    {
        var buffer = new byte[blockSize];
        var offset = blockNumber * blockSize;

        stream.Position = offset;
        stream.Write(buffer, 0, buffer.Length);
    }
}