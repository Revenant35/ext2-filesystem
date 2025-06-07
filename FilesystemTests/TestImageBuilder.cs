namespace FilesystemTests;

using Filesystem;
using Filesystem.Models;
using Filesystem.Serializers;
using System.Text;

public static class TestImageBuilder
{
    private const int BlockSize = 1024;
    private const uint RootInodeIndex = 2u;

    private static ushort CalculateMinimalRecLen(string name)
    {
        int nameLength = Encoding.UTF8.GetByteCount(name);
        if (nameLength > 255) throw new ArgumentOutOfRangeException(nameof(name), "Name too long for ext2 directory entry.");
        int minLengthWithoutPadding = 8 + nameLength;
        return (ushort)((minLengthWithoutPadding + 3) & ~3);
    }

    private static void WriteDirectoryEntryHelper(BinaryWriter writer, uint inode, string name, InodeDirectoryType type, ushort recordLength)
    {
        writer.Write(inode);
        writer.Write(recordLength);

        byte[] nameBytes = Encoding.UTF8.GetBytes(name);
        byte nameLength = (byte)nameBytes.Length;
        if (nameLength > 255) throw new ArgumentOutOfRangeException(nameof(name), "Name too long for ext2 directory entry name byte count.");

        writer.Write(nameLength);
        writer.Write((byte)type);
        writer.Write(nameBytes);

        int fixedAndNameSize = 8 + nameLength;
        int paddingBytes = recordLength - fixedAndNameSize;

        if (paddingBytes < 0)
            throw new ArgumentException("Record length is too small for the given name and fixed fields.", nameof(recordLength));

        if (paddingBytes > 0)
        {
            writer.Write(new byte[paddingBytes]);
        }
    }

    public static (Disk disk, FileSystem fs, Inode inode, byte[] expected) CreateSingleIndirectFile()
    {
        const int indirectBlockIndex = 12;

        using var ms = new MemoryStream();
        var formatter = new DiskFormatter(ms);
        formatter.Format(new Superblock());
        ms.Position = 0;

        var disk = new Disk(ms);
        var fs = new FileSystem(disk);

        var fileBlocks = new List<byte[]>
        {
            Encoding.UTF8.GetBytes("BlockA".PadRight(BlockSize, '.')),
            Encoding.UTF8.GetBytes("BlockB".PadRight(BlockSize, '.')),
        };

        uint dataBlockStart = 100;
        for (var i = 0; i < fileBlocks.Count; i++)
        {
            ms.Position = dataBlockStart * BlockSize + i * BlockSize;
            ms.Write(fileBlocks[i]);
        }

        var indirectBlockData = new byte[BlockSize];
        using (var writer = new BinaryWriter(new MemoryStream(indirectBlockData)))
        {
            writer.Write(dataBlockStart);
            writer.Write(dataBlockStart + 1);
        }

        ms.Position = 99 * BlockSize;
        ms.Write(indirectBlockData);

        var inode = new Inode
        {
            Type = InodeType.File,
            SizeBytes = (uint)(fileBlocks.Count * BlockSize),
            BlockPointers = new uint[15]
        };
        inode.BlockPointers[indirectBlockIndex] = 99;

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
        const uint fileInodeIndex = 3u;
        const uint fileContentDataBlock = 100u;
        const uint fileIndirectBlock = 99u;
        const uint rootDirDataBlock = 105u;
        const int indirectBlockIndex = 12;

        var ms = new MemoryStream();
        var formatter = new DiskFormatter(ms);
        formatter.Format(new Superblock());
        ms.Position = 0;

        var disk = new Disk(ms);
        var fs = new FileSystem(disk);

        ms.Position = fileContentDataBlock * BlockSize;
        ms.Write(fileContent);

        var indirectBlockData = new byte[BlockSize];
        using (var indirectWriter = new BinaryWriter(new MemoryStream(indirectBlockData)))
        {
            indirectWriter.Write(fileContentDataBlock);
        }
        ms.Position = fileIndirectBlock * BlockSize;
        ms.Write(indirectBlockData);

        var fileInode = new Inode
        {
            Type = InodeType.File,
            SizeBytes = (uint)fileContent.Length,
            HardLinkCount = 1,
            BlockPointers = new uint[15]
        };
        fileInode.BlockPointers[indirectBlockIndex] = fileIndirectBlock;
        var fileInodeOffset = disk.GetInodeOffset(fileInodeIndex);
        ms.Position = fileInodeOffset;
        using (var inodeWriter = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            inodeWriter.Write(fileInode);
        }

        var rootInode = new Inode
        {
            Type = InodeType.Directory,
            SizeBytes = BlockSize,
            HardLinkCount = 2,
            BlockPointers = new uint[15]
        };
        rootInode.BlockPointers[0] = rootDirDataBlock;
        var rootInodeOffset = disk.GetInodeOffset(RootInodeIndex);
        ms.Position = rootInodeOffset;
        using (var inodeWriter = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            inodeWriter.Write(rootInode);
        }

        ms.Position = rootDirDataBlock * BlockSize;
        using (var dirWriter = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            ushort fileEntryRecLen = CalculateMinimalRecLen(fileName);
            ushort terminatorRecLen = (ushort)(BlockSize - fileEntryRecLen);

            WriteDirectoryEntryHelper(dirWriter, fileInodeIndex, fileName, InodeDirectoryType.RegularFile, fileEntryRecLen);
            WriteDirectoryEntryHelper(dirWriter, 0, "", InodeDirectoryType.UnknownType, terminatorRecLen);
        }

        return (fs, fileInode, fileContent);
    }

    public static (FileSystem fs, uint subDirInodeNumber) CreateSubDirectoryInRoot(string dirName, string preExistingFileNameInRoot = "existing.txt")
    {
        const uint fileInRootInodeIndex = 3u;
        const uint subDirInodeIndex = 4u;

        const uint rootDirDataBlock = 105u;
        const uint subDirDataBlock = 110u;
        const uint preExistingFileContentBlock = 100u;
        const uint preExistingFileIndirectBlock = 99u;
        const int indirectBlockIndex = 12;

        var ms = new MemoryStream();
        var formatter = new DiskFormatter(ms);
        formatter.Format(new Superblock());
        ms.Position = 0;

        var disk = new Disk(ms);
        var fs = new FileSystem(disk);

        byte[] preExistingFileContent = Encoding.UTF8.GetBytes("Content of pre-existing file.");
        ms.Position = preExistingFileContentBlock * BlockSize;
        ms.Write(preExistingFileContent);

        var indirectBlockData = new byte[BlockSize];
        using (var indirectWriter = new BinaryWriter(new MemoryStream(indirectBlockData))) { indirectWriter.Write(preExistingFileContentBlock); }
        ms.Position = preExistingFileIndirectBlock * BlockSize;
        ms.Write(indirectBlockData);

        var preExistingFileInode = new Inode
        {
            Type = InodeType.File,
            SizeBytes = (uint)preExistingFileContent.Length,
            HardLinkCount = 1,
            BlockPointers = new uint[15]
        };
        preExistingFileInode.BlockPointers[indirectBlockIndex] = preExistingFileIndirectBlock;
        ms.Position = disk.GetInodeOffset(fileInRootInodeIndex);
        using (var inodeWriter = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true)) { inodeWriter.Write(preExistingFileInode); }

        ms.Position = subDirDataBlock * BlockSize;
        using (var subDirBlockWriter = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            ushort dotRecLen = CalculateMinimalRecLen(".");
            ushort dotDotRecLen = CalculateMinimalRecLen("..");
            ushort subDirTerminatorRecLen = (ushort)(BlockSize - dotRecLen - dotDotRecLen);

            WriteDirectoryEntryHelper(subDirBlockWriter, subDirInodeIndex, ".", InodeDirectoryType.Directory, dotRecLen);
            WriteDirectoryEntryHelper(subDirBlockWriter, RootInodeIndex, "..", InodeDirectoryType.Directory, dotDotRecLen);
            WriteDirectoryEntryHelper(subDirBlockWriter, 0, "", InodeDirectoryType.UnknownType, subDirTerminatorRecLen);
        }

        var subDirInode = new Inode
        {
            Type = InodeType.Directory,
            SizeBytes = BlockSize,
            HardLinkCount = 2,
            BlockPointers = new uint[15]
        };
        subDirInode.BlockPointers[0] = subDirDataBlock;
        ms.Position = disk.GetInodeOffset(subDirInodeIndex);
        using (var inodeWriter = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true)) { inodeWriter.Write(subDirInode); }

        var rootInode = disk.ReadInode(RootInodeIndex);
        rootInode.HardLinkCount++; 
        if (rootInode.BlockPointers[0] == 0) rootInode.BlockPointers[0] = rootDirDataBlock;
        if (rootInode.SizeBytes == 0) rootInode.SizeBytes = BlockSize; 
        if (rootInode.Type == 0) rootInode.Type = InodeType.Directory;


        ms.Position = disk.GetInodeOffset(RootInodeIndex);
        using (var inodeWriter = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true)) { inodeWriter.Write(rootInode); }

        ms.Position = rootDirDataBlock * BlockSize;
        using (var rootDirBlockWriter = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            ushort preExistingFileEntryRecLen = CalculateMinimalRecLen(preExistingFileNameInRoot);
            ushort subDirEntryRecLen = CalculateMinimalRecLen(dirName);
            ushort rootTerminatorRecLen = (ushort)(BlockSize - preExistingFileEntryRecLen - subDirEntryRecLen);

            WriteDirectoryEntryHelper(rootDirBlockWriter, fileInRootInodeIndex, preExistingFileNameInRoot, InodeDirectoryType.RegularFile, preExistingFileEntryRecLen);
            WriteDirectoryEntryHelper(rootDirBlockWriter, subDirInodeIndex, dirName, InodeDirectoryType.Directory, subDirEntryRecLen);
            WriteDirectoryEntryHelper(rootDirBlockWriter, 0, "", InodeDirectoryType.UnknownType, rootTerminatorRecLen);
        }
        
        return (fs, subDirInodeIndex);
    }

    public static void AddFileToDirectory(
        Disk disk, 
        MemoryStream ms, 
        uint parentDirInodeNumber, 
        string fileName, 
        byte[] fileContent, 
        uint newFileInodeIndex, 
        uint newFileContentDataBlock)
    {
        ms.Position = newFileContentDataBlock * BlockSize;
        ms.Write(fileContent);

        var fileInode = new Inode
        {
            Type = InodeType.File,
            SizeBytes = (uint)fileContent.Length,
            HardLinkCount = 1,
            BlockPointers = new uint[15]
        };
        fileInode.BlockPointers[0] = newFileContentDataBlock; 
        
        long fileInodeOffset = disk.GetInodeOffset(newFileInodeIndex);
        ms.Position = fileInodeOffset;
        using (var inodeWriter = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            inodeWriter.Write(fileInode);
        }

        Inode parentDirInode = disk.ReadInode(parentDirInodeNumber);
        uint parentDirDataBlockNumber = parentDirInode.BlockPointers[0]; 

        ms.Position = parentDirDataBlockNumber * BlockSize; 
        using (var writer = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            ushort currentOffsetInBlock = 0;
            ushort dotRecLen = CalculateMinimalRecLen(".");
            WriteDirectoryEntryHelper(writer, parentDirInodeNumber, ".", InodeDirectoryType.Directory, dotRecLen);
            currentOffsetInBlock += dotRecLen;

            ushort dotDotRecLen = CalculateMinimalRecLen("..");
            WriteDirectoryEntryHelper(writer, RootInodeIndex, "..", InodeDirectoryType.Directory, dotDotRecLen);
            currentOffsetInBlock += dotDotRecLen;
            
            ushort newFileEntryRecLen = CalculateMinimalRecLen(fileName);
            InodeDirectoryType newFileType = fileContent.Length > 0 ? InodeDirectoryType.RegularFile : InodeDirectoryType.UnknownType;
            WriteDirectoryEntryHelper(writer, newFileInodeIndex, fileName, newFileType, newFileEntryRecLen);
            currentOffsetInBlock += newFileEntryRecLen;
            
            ushort finalTerminatorRecLen = (ushort)(BlockSize - currentOffsetInBlock);
            WriteDirectoryEntryHelper(writer, 0, "", InodeDirectoryType.UnknownType, finalTerminatorRecLen);
        }
    }
}