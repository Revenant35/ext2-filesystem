namespace Filesystem.Serializers;

using Models;

// https://wiki.osdev.org/Ext2#Inode_Data_Structure
public static class InodeSerialization
{
    public static Inode ReadInode(this BinaryReader reader)
    {
        var typeAndPermissions = reader.ReadUInt16();
        var type = (InodeType)(typeAndPermissions & 0xF000);
        var permissions = (InodePermissions)(typeAndPermissions & 0x0FFF);
        var userID = reader.ReadUInt16();
        var sizeInBytes = reader.ReadUInt32();
        var lastAccessed = reader.ReadUInt32();
        var createdAt = reader.ReadUInt32();
        var lastModified = reader.ReadUInt32();
        var deletedAt = reader.ReadUInt32();
        var groupID = reader.ReadUInt16();
        var hardLinkCount = reader.ReadUInt16();
        var diskSectorCount = reader.ReadUInt32();
        var flags = (InodeFlags)reader.ReadUInt32();
        var osd1LinuxReserved = reader.ReadBytes(4);

        var blockPointers = new uint[15];
        for (var i = 0; i < 15; i++)
        {
            blockPointers[i] = reader.ReadUInt32();
        }

        var generationNumber = reader.ReadUInt32();
        var fileACLBlock = reader.ReadUInt32();
        var directoryACLBlock = reader.ReadUInt32();
        var fragmentBlockAddress = reader.ReadUInt32();
        var osd2LinuxSpecific = reader.ReadBytes(12);

        return new Inode
        {
            Type = type,
            Permissions = permissions,
            UserID = userID,
            SizeBytes = sizeInBytes,
            LastAccessed = (lastAccessed != 0) ? DateTimeOffset.FromUnixTimeSeconds(lastAccessed) : null,
            CreatedAt = DateTimeOffset.FromUnixTimeSeconds(createdAt),
            LastModified = (lastModified != 0) ? DateTimeOffset.FromUnixTimeSeconds(lastModified) : null,
            DeletedAt = (deletedAt != 0) ? DateTimeOffset.FromUnixTimeSeconds(deletedAt) : null,
            GroupID = groupID,
            HardLinkCount = hardLinkCount,
            DiskSectorCount = diskSectorCount,
            Flags = flags,
            BlockPointers = blockPointers,
            GenerationNumber = generationNumber,
            FileACLBlock = fileACLBlock,
            DirectoryACLBlock = directoryACLBlock,
            FragmentBlockAddress = fragmentBlockAddress,
            OperatingSystemSpecificValues = [..osd1LinuxReserved, ..osd2LinuxSpecific],
        };
    }

    public static void Write(this BinaryWriter writer, Inode inode)
    {
        var typeAndPermissions = (ushort)((ushort)inode.Type | (ushort)inode.Permissions);
        writer.Write(typeAndPermissions);
        writer.Write(inode.UserID);
        writer.Write(inode.SizeBytes);
        writer.Write((inode.LastAccessed != null) ? Convert.ToUInt32(inode.LastAccessed.Value.ToUnixTimeSeconds()) : 0);
        writer.Write(Convert.ToUInt32(inode.CreatedAt.ToUnixTimeSeconds()));
        writer.Write((inode.LastModified != null) ? Convert.ToUInt32(inode.LastModified.Value.ToUnixTimeSeconds()) : 0);
        writer.Write((inode.DeletedAt != null) ? Convert.ToUInt32(inode.DeletedAt.Value.ToUnixTimeSeconds()) : 0);
        writer.Write(inode.GroupID);
        writer.Write(inode.HardLinkCount);
        writer.Write(inode.DiskSectorCount);
        writer.Write((uint)inode.Flags);
        writer.Write(inode.OperatingSystemSpecificValues.AsSpan(0, 4));

        foreach (var blockPointer in inode.BlockPointers)
        {
            writer.Write(blockPointer);
        }

        writer.Write(inode.GenerationNumber);
        writer.Write(inode.FileACLBlock);
        writer.Write(inode.DirectoryACLBlock);
        writer.Write(inode.FragmentBlockAddress);
        writer.Write(inode.OperatingSystemSpecificValues.AsSpan(4, 12));
    }
}