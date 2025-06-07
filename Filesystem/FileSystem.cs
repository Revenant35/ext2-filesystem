namespace Filesystem;

using Models;
using Serializers;
using System.Text;

public class FileSystem
{
    public readonly Disk Disk;
    
    public FileSystem(Disk disk)
    {
        Disk = disk;
    }
    
    public List<InodeDirectory> ReadDirectoryEntries(Inode directoryInode)
    {
        if (directoryInode.Type != InodeType.Directory)
            throw new ArgumentException("Inode is not a directory.");

        var entries = new List<InodeDirectory>();

        foreach (var blockAddress in directoryInode.BlockPointers.Where(ptr => ptr != 0))
        {
            var block = Disk.ReadBlock((int)blockAddress);

            using var reader = new BinaryReader(new MemoryStream(block), Encoding.UTF8, leaveOpen: false);

            while (reader.BaseStream.Position < block.Length)
            {
                var entry = reader.ReadInodeDirectory();

                if (entry.Inode == 0) 
                    break;

                entries.Add(entry);
            }
        }

        return entries;
    }
    
    public Inode ResolvePath(string path)
    {
        if (string.IsNullOrWhiteSpace(path))
            throw new ArgumentException("Path is null or empty.", nameof(path));

        var components = path.Split('/', StringSplitOptions.RemoveEmptyEntries);

        var currentInode = Disk.ReadRootInode();

        foreach (var component in components)
        {
            if (currentInode.Type != InodeType.Directory)
                throw new InvalidOperationException($"'{component}' is not a directory.");

            var entries = ReadDirectoryEntries(currentInode);
            var match = entries.FirstOrDefault(e => e.Name == component);

            if (match.Inode == 0)
                throw new FileNotFoundException($"'{component}' not found in directory.");

            currentInode = Disk.ReadInode(match.Inode);
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
            var block = Disk.ReadBlock((int)blockNumber);
            data.AddRange(block);
        }

        return data.Take((int)inode.SizeBytes).ToArray();
    }

    
    private IEnumerable<uint> GetAllDataBlockPointers(Inode inode) => 
    [
        ..GetAllDirectPointers(inode),
        ..GetAllIndirectPointers(inode),
    ];

    private IEnumerable<uint> GetAllDirectPointers(Inode inode) =>
        inode.BlockPointers.Take(12).Where(ptr => ptr != 0);

    private IEnumerable<uint> GetAllIndirectPointers(Inode inode)
    {
        var indirectBlock = inode.BlockPointers[12];
        if (indirectBlock != 0)
        {
            var block = Disk.ReadBlock((int)indirectBlock);
            using var reader = new BinaryReader(new MemoryStream(block), Encoding.UTF8, leaveOpen: false);

            var pointerCount = Disk.BlockSize / 4;
            for (var i = 0; i < pointerCount; i++)
            {
                var ptr = reader.ReadUInt32();
                if (ptr != 0)
                    yield return ptr;
            }
        }
    }

}