namespace Filesystem.Services;

using Models;

public interface ISuperblockService
{
    Superblock Superblock { get; }
    uint InodeCount { get; }
    uint InodeSize { get; }
    uint BlockCount { get; }
    uint BlockSize { get; }
    uint FragmentSize { get; }
    uint BlocksPerGroup { get; }
    uint InodesPerGroup { get; }
    
    Superblock ReadSuperblock();
    void WriteSuperblock(Superblock superblock);
}