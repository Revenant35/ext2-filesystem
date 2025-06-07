namespace Filesystem;

using Models;
using Serializers;
using System.Text;

public class FileSystem
{
    private readonly Disk _disk;
    
    public FileSystem(Disk disk)
    {
        _disk = disk;
    }
    
    public List<InodeDirectory> ReadDirectoryEntries(Inode directoryInode)
    {
        if (directoryInode.Type != InodeType.Directory)
            throw new ArgumentException("Inode is not a directory.");

        var entries = new List<InodeDirectory>();

        foreach (var blockAddress in directoryInode.BlockPointers.Where(ptr => ptr != 0))
        {
            var block = _disk.ReadBlock((int)blockAddress);

            using var reader = new BinaryReader(new MemoryStream(block), Encoding.UTF8, leaveOpen: false);

            long totalRead = 0;
            while (totalRead < block.Length)
            {
                var startPos = reader.BaseStream.Position;
                var entry = reader.ReadInodeDirectory();

                if (entry.Inode == 0 || entry.EntrySize == 0)
                    break;

                entries.Add(entry);
                totalRead += entry.EntrySize;

                reader.BaseStream.Seek(startPos + entry.EntrySize, SeekOrigin.Begin);
            }
        }

        return entries;
    }
    
    public Inode ResolvePath(string path)
    {
        if (string.IsNullOrWhiteSpace(path))
            throw new ArgumentException("Path is null or empty.", nameof(path));

        var components = path.Split('/', StringSplitOptions.RemoveEmptyEntries);

        var currentInode = _disk.ReadRootInode();

        foreach (var component in components)
        {
            if (currentInode.Type != InodeType.Directory)
                throw new InvalidOperationException($"'{component}' is not a directory.");

            var entries = ReadDirectoryEntries(currentInode);
            var match = entries.FirstOrDefault(e => e.Name == component);

            if (match.Inode == 0)
                throw new FileNotFoundException($"'{component}' not found in directory.");

            currentInode = _disk.ReadInode(match.Inode);
        }

        return currentInode;
    }
    
    public List<string> ListDirectory(string path)
    {
        var dirInode = ResolvePath(path);

        if (dirInode.Type != InodeType.Directory)
            throw new InvalidOperationException("Path does not point to a directory.");

        var entries = ReadDirectoryEntries(dirInode);
        return entries.Select(e => e.Name).ToList();
    }

    public byte[] ReadFile(Inode inode)
    {
        if (inode.Type != InodeType.File)
            throw new ArgumentException("Inode is not a file.");

        var data = new List<byte>();

        foreach (var blockNumber in GetAllDataBlockPointers(inode))
        {
            var block = _disk.ReadBlock((int)blockNumber);
            data.AddRange(block);
        }

        return data.Take((int)inode.SizeBytes).ToArray(); // Trim to actual file size
    }

    
    private IEnumerable<uint> GetAllDataBlockPointers(Inode inode)
    {
        var blockSize = _disk.BlockSize;
        var ptrs = new List<uint>();

        // 1. Direct blocks
        for (int i = 0; i < 12; i++)
        {
            var ptr = inode.BlockPointers[i];
            if (ptr != 0)
                ptrs.Add(ptr);
        }

        // 2. Single indirect block
        var indirectBlock = inode.BlockPointers[12];
        if (indirectBlock != 0)
        {
            var block = _disk.ReadBlock((int)indirectBlock);
            using var reader = new BinaryReader(new MemoryStream(block), Encoding.UTF8, leaveOpen: false);

            var pointerCount = blockSize / 4;
            for (int i = 0; i < pointerCount; i++)
            {
                var ptr = reader.ReadUInt32();
                if (ptr != 0)
                    ptrs.Add(ptr);
            }
        }

        return ptrs;
    }

}