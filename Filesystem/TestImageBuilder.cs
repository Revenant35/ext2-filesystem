namespace Filesystem;

using Models;
using Serializers;
using System.Text;

public static class TestImageBuilder
{
    public static (Disk disk, FileSystem fs, Inode inode, byte[] expected) CreateSingleIndirectFile()
    {
        const int blockSize = 1024;
        const int indirectBlockIndex = 12;

        using var ms = new MemoryStream();
        var formatter = new DiskFormatter(ms);
        formatter.Format(new Superblock());
        ms.Position = 0;

        var disk = new Disk(ms);
        var fs = new FileSystem(disk);

        // Prepare file content
        var fileBlocks = new List<byte[]>
        {
            Encoding.UTF8.GetBytes("BlockA".PadRight(blockSize, '.')),
            Encoding.UTF8.GetBytes("BlockB".PadRight(blockSize, '.')),
        };

        // Write data blocks
        uint dataBlockStart = 100;
        for (var i = 0; i < fileBlocks.Count; i++)
        {
            ms.Position = dataBlockStart * blockSize + i * blockSize;
            ms.Write(fileBlocks[i]);
        }

        // Write indirect block (block 99)
        var indirectBlockData = new byte[blockSize];
        using (var writer = new BinaryWriter(new MemoryStream(indirectBlockData)))
        {
            writer.Write(dataBlockStart); // -> BlockA
            writer.Write(dataBlockStart + 1); // -> BlockB
        }

        ms.Position = 99 * blockSize;
        ms.Write(indirectBlockData);

        // Create inode with single indirect pointer
        var inode = new Inode
        {
            Type = InodeType.File,
            SizeBytes = (uint)(fileBlocks.Count * blockSize),
            BlockPointers = new uint[15]
        };
        inode.BlockPointers[indirectBlockIndex] = 99;

        // Write inode into position 1
        var inodeOffset = disk.GetInodeOffset(2);

        ms.Position = inodeOffset!;
        using (var writer = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            writer.Write(inode);
        }

        return (disk, fs, inode, fileBlocks.SelectMany(b => b).ToArray());
    }

    public static (FileSystem fs, Inode fileInode, byte[] fileContent) CreateFileUnderRoot(string fileName, byte[] fileContent)
    {
        const int blockSize = 1024;
        const uint indirectBlockIndex = 12;
        const uint indirectBlock = 99;
        const uint dataBlock = 100;

        var rootInodeIndex = 2u;
        var fileInodeIndex = 3u;

        var ms = new MemoryStream();
        var formatter = new DiskFormatter(ms);
        formatter.Format(new Superblock());
        ms.Position = 0;

        var disk = new Disk(ms);
        var fs = new FileSystem(disk);

        // 1. Write file content block
        ms.Position = dataBlock * blockSize;
        ms.Write(fileContent);

        // 2. Write indirect block pointing to dataBlock
        var indirectBlockData = new byte[blockSize];
        using (var writer = new BinaryWriter(new MemoryStream(indirectBlockData)))
        {
            writer.Write(dataBlock);
        }
        ms.Position = indirectBlock * blockSize;
        ms.Write(indirectBlockData);

        // 3. Write file inode
        var fileInode = new Inode
        {
            Type = InodeType.File,
            SizeBytes = (uint)fileContent.Length,
            BlockPointers = new uint[15]
        };
        fileInode.BlockPointers[indirectBlockIndex] = indirectBlock;

        var fileInodeOffset = disk.GetInodeOffset(fileInodeIndex);

        ms.Position = fileInodeOffset!;
        using (var writer = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            writer.Write(fileInode);
        }

        // 4. Write root directory inode with pointer to one block
        var dirBlock = 105u;
        var rootInode = new Inode
        {
            Type = InodeType.Directory,
            SizeBytes = blockSize,
            BlockPointers = new uint[15]
        };
        rootInode.BlockPointers[0] = dirBlock;

        var rootInodeOffset = disk.GetInodeOffset(rootInodeIndex);

        ms.Position = rootInodeOffset!;
        using (var writer = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            writer.Write(rootInode);
        }

        // 5. Write directory entry pointing to fileInode
        ms.Position = dirBlock * blockSize;
        using (var writer = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            var entry = new InodeDirectory
            {
                Inode = fileInodeIndex,
                EntrySize = (ushort)(8 + fileName.Length),
                Name = fileName,
                Type = InodeDirectoryType.RegularFile,
            };
            writer.Write(entry);
        }

        return (fs, fileInode, fileContent);
    }
}