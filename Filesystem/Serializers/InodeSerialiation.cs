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
        var lastAccessed = DateTimeOffset.FromUnixTimeSeconds(reader.ReadUInt32());
        var createdAt = DateTimeOffset.FromUnixTimeSeconds(reader.ReadUInt32());
        var lastModified = DateTimeOffset.FromUnixTimeSeconds(reader.ReadUInt32());
        var deletedAt = DateTimeOffset.FromUnixTimeSeconds(reader.ReadUInt32());
        var groupID = reader.ReadUInt16();
        var hardLinkCount = reader.ReadUInt16();
        var diskSectorCount = reader.ReadUInt32();
        var flags = (InodeFlags)reader.ReadUInt32();
        reader.ReadUInt32(); // TODO: OperatingSystemSpecificValues #1
        var blockPointers = new uint[12];
        for (var i = 0; i < 12; i++)
        {
            blockPointers[i] = reader.ReadUInt32();
        }
        var singlyIndirectBlockPointer = reader.ReadUInt32();
        var doublyIndirectBlockPointer = reader.ReadUInt32();
        var triplyIndirectBlockPointer = reader.ReadUInt32();
        reader.ReadUInt32(); // (reserved)
        reader.ReadUInt32(); // (reserved)
        var generationNumber = reader.ReadUInt32();
        var fragmentBlockAddress = reader.ReadUInt32();
        reader.ReadBytes(12); // TODO: OperatingSystemSpecificValues #2

        return new Inode
        {
            Type = type,
            Permissions = permissions,
            UserID = userID,
            SizeBytes = sizeInBytes,
            LastAccessed = lastAccessed,
            CreatedAt = createdAt,
            LastModified = lastModified,
            DeletedAt = deletedAt,
            GroupID = groupID,
            HardLinkCount = hardLinkCount,
            DiskSectorCount = diskSectorCount,
            Flags = flags,
            BlockPointers = blockPointers,
            SinglyIndirectBlockPointer = singlyIndirectBlockPointer,
            DoublyIndirectBlockPointer = doublyIndirectBlockPointer,
            TriplyIndirectBlockPointer = triplyIndirectBlockPointer,
            GenerationNumber = generationNumber,
            FragmentBlockAddress = fragmentBlockAddress,
            OperatingSystemSpecificValues = [],
        };
    }

    public static void WriteInode(this BinaryWriter writer, Inode inode)
    {
        var typeAndPermissions = (ushort)((ushort)inode.Type | (ushort)inode.Permissions);
        writer.Write(typeAndPermissions);
        writer.Write(inode.UserID);
        writer.Write(inode.SizeBytes);
        writer.Write(Convert.ToUInt32(inode.LastAccessed.ToUnixTimeSeconds()));
        writer.Write(Convert.ToUInt32(inode.CreatedAt.ToUnixTimeSeconds()));
        writer.Write(Convert.ToUInt32(inode.LastModified.ToUnixTimeSeconds()));
        writer.Write(Convert.ToUInt32(inode.DeletedAt.ToUnixTimeSeconds()));
        writer.Write(inode.GroupID);
        writer.Write(inode.HardLinkCount);
        writer.Write(inode.DiskSectorCount);
        writer.Write((uint)inode.Flags);
        writer.Write((uint)0x00);
        foreach (var blockPointer in inode.BlockPointers)
        {
            writer.Write(blockPointer);
        }
        
        writer.Write(inode.SinglyIndirectBlockPointer);
        writer.Write(inode.DoublyIndirectBlockPointer);
        writer.Write(inode.TriplyIndirectBlockPointer);
        writer.Write((uint)0x00); // (reserved)
        writer.Write((uint)0x00); // (reserved)
        writer.Write(inode.GenerationNumber);
        writer.Write(inode.FragmentBlockAddress);
        writer.Write(new byte[12]);
    }
}