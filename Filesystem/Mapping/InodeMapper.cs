namespace Filesystem.Mapping;

using Enums;
using Models;
using Serialization.Models;

public static class InodeMapper
{
    public static BinaryInode ToBinaryInode(this Inode inode) => new()
    {
        TypeAndPermissions = (ushort)((ushort)inode.Type | (ushort)inode.Permissions),
        UserID = inode.UserID,
        SizeBytes = inode.SizeBytes,
        LastAccessed = inode.LastAccessed != null ? Convert.ToUInt32(inode.LastAccessed.Value.ToUnixTimeSeconds()) : 0,
        CreatedAt = Convert.ToUInt32(inode.CreatedAt.ToUnixTimeSeconds()),
        LastModified = inode.LastModified != null ? Convert.ToUInt32(inode.LastModified.Value.ToUnixTimeSeconds()) : 0,
        DeletedAt = inode.DeletedAt != null ? Convert.ToUInt32(inode.DeletedAt.Value.ToUnixTimeSeconds()) : 0,
        GroupID = inode.GroupID,
        HardLinkCount = inode.HardLinkCount,
        DiskSectorCount = inode.DiskSectorCount,
        Flags = (uint)inode.Flags,
        BlockAddresses = inode.BlockAddresses
            .Take(12)
            .Concat(Enumerable.Repeat(0u, 12 - inode.BlockAddresses.Count))
            .ToArray(),
        SinglyIndirectBlockAddress = inode.SinglyIndirectBlockAddress ?? 0,
        DoublyIndirectBlockAddress = inode.DoublyIndirectBlockAddress ?? 0,
        TriplyIndirectBlockAddress = inode.TriplyIndirectBlockAddress ?? 0,
        GenerationNumber = inode.GenerationNumber,
        FileACLBlock = inode.FileACLBlock,
        DirectoryACLBlock = inode.DirectoryACLBlock,
        FragmentBlockAddress = inode.FragmentBlockAddress,
        OperatingSystemSpecificValues = inode.OperatingSystemSpecificValues,
    };

    public static Inode ToInode(this BinaryInode binary) => new()
    {
        Type = (InodeType)(binary.TypeAndPermissions & 0xF000),
        Permissions = (InodePermissions)(binary.TypeAndPermissions & 0x0FFF),
        UserID = binary.UserID,
        SizeBytes = binary.SizeBytes,
        LastAccessed = binary.LastAccessed != 0 ? DateTimeOffset.FromUnixTimeSeconds(binary.LastAccessed) : null,
        CreatedAt = DateTimeOffset.FromUnixTimeSeconds(binary.CreatedAt),
        LastModified = binary.LastModified != 0 ? DateTimeOffset.FromUnixTimeSeconds(binary.LastModified) : null,
        DeletedAt = binary.DeletedAt != 0 ? DateTimeOffset.FromUnixTimeSeconds(binary.DeletedAt) : null,
        GroupID = binary.GroupID,
        HardLinkCount = binary.HardLinkCount,
        DiskSectorCount = binary.DiskSectorCount,
        Flags = (InodeFlags)binary.Flags,
        BlockAddresses = binary.BlockAddresses
            .Take(12)
            .Where(u => u != 0)
            .ToList(),
        SinglyIndirectBlockAddress = binary.SinglyIndirectBlockAddress != 0 ? binary.SinglyIndirectBlockAddress : null,
        DoublyIndirectBlockAddress = binary.DoublyIndirectBlockAddress != 0 ? binary.DoublyIndirectBlockAddress : null,
        TriplyIndirectBlockAddress = binary.TriplyIndirectBlockAddress != 0 ? binary.TriplyIndirectBlockAddress : null,
        GenerationNumber = binary.GenerationNumber,
        FileACLBlock = binary.FileACLBlock,
        DirectoryACLBlock = binary.DirectoryACLBlock,
        FragmentBlockAddress = binary.FragmentBlockAddress,
        OperatingSystemSpecificValues = binary.OperatingSystemSpecificValues,
    };
}