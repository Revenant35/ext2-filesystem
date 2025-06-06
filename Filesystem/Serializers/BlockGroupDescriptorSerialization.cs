namespace Filesystem.Serializers;

using Models;

// https://wiki.osdev.org/Ext2#Block_Group_Descriptor_Table
public static class BlockGroupDescriptorSerialization
{
    public static BlockGroupDescriptor ReadBlockGroupDescriptor(this BinaryReader reader)
    {
        var blockUsageBitmapBlockAddress = reader.ReadUInt32();
        var inodeUsageBitmapBlockAddress = reader.ReadUInt32();
        var inodeTableStartingBlockAddress = reader.ReadUInt32();
        var unallocatedBlocksInGroup = reader.ReadUInt16();
        var unallocatedInodesInGroup = reader.ReadUInt16();
        var directoriesInGroup = reader.ReadUInt16();
        reader.ReadBytes(14);

        return new BlockGroupDescriptor
        {
            BlockUsageBitmapBlockAddress = blockUsageBitmapBlockAddress,
            InodeUsageBitmapBlockAddress = inodeUsageBitmapBlockAddress,
            InodeTableStartingBlockAddress = inodeTableStartingBlockAddress,
            UnallocatedBlocksInGroup = unallocatedBlocksInGroup,
            UnallocatedInodesInGroup = unallocatedInodesInGroup,
            DirectoriesInGroup = directoriesInGroup,
        };
    }

    public static void Write(this BinaryWriter writer, BlockGroupDescriptor blockGroupDescriptor)
    {
        writer.Write(blockGroupDescriptor.BlockUsageBitmapBlockAddress);
        writer.Write(blockGroupDescriptor.InodeUsageBitmapBlockAddress);
        writer.Write(blockGroupDescriptor.InodeTableStartingBlockAddress);
        writer.Write(blockGroupDescriptor.UnallocatedBlocksInGroup);
        writer.Write(blockGroupDescriptor.UnallocatedInodesInGroup);
        writer.Write(blockGroupDescriptor.DirectoriesInGroup);
        writer.Write(new byte[14]);
    }
}
