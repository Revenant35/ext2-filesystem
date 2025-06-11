namespace Filesystem.Services;

using Models;
using Serialization.Serializers;
using System.Text;

public class SuperblockService : ISuperblockService
{
    public const long SuperblockOffset = 1024;
    
    public Superblock Superblock { get; private set; }
    
    public uint InodeCount => Superblock.InodeCount;
    public uint InodeSize => Superblock.InodeSize;
    public uint BlockCount => Superblock.BlockCount;
    public uint BlockSize => Superblock.BlockSize;
    public uint FragmentSize => Superblock.FragmentSize;
    public uint BlocksPerGroup => Superblock.BlocksPerGroup;
    public uint InodesPerGroup => Superblock.InodesPerGroup;

    private readonly Stream _stream;

    public SuperblockService(Stream stream)
    {
        _stream = stream;
        Superblock = ReadSuperblock();
    }
    
    public Superblock ReadSuperblock()
    {
        var previousPosition = _stream.Position;
        _stream.Seek(SuperblockOffset, SeekOrigin.Begin);
        using var reader = new BinaryReader(_stream, Encoding.UTF8, true);

        var superblock = reader.ReadSuperblock();
        _stream.Seek(previousPosition, SeekOrigin.Begin);

        return superblock;
    }

    public void WriteSuperblock(Superblock superblock)
    {
        var previousPosition = _stream.Position;
        _stream.Seek(SuperblockOffset, SeekOrigin.Begin);
        using var writer = new BinaryWriter(_stream, Encoding.UTF8, true);
    
        writer.Write(superblock);
        _stream.Flush();
        _stream.Seek(previousPosition, SeekOrigin.Begin);
        
        Superblock = superblock;
    }
}