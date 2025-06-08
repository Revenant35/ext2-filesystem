namespace Filesystem.Serialization.Serializers;

using Filesystem.Models;
using Mapping;
using Models;

// https://wiki.osdev.org/Ext2#Block_Group_Descriptor_Table
public static class BlockGroupDescriptorSerializer
{
    public static BlockGroupDescriptor ReadBlockGroupDescriptor(this BinaryReader reader)
    {
        var blockUsageBitmapBlockAddress = reader.ReadUInt32();
        var inodeUsageBitmapBlockAddress = reader.ReadUInt32();
        var inodeTableStartingBlockAddress = reader.ReadUInt32();
        var unallocatedBlocksInGroup = reader.ReadUInt16();
        var unallocatedInodesInGroup = reader.ReadUInt16();
        var directoriesInGroup = reader.ReadUInt16();
        reader.ReadBytes(14); // (unused)

        var binary = new BinaryBlockGroupDescriptor
        {
            BlockUsageBitmapBlockAddress = blockUsageBitmapBlockAddress,
            InodeUsageBitmapBlockAddress = inodeUsageBitmapBlockAddress,
            InodeTableStartingBlockAddress = inodeTableStartingBlockAddress,
            UnallocatedBlocksInGroup = unallocatedBlocksInGroup,
            UnallocatedInodesInGroup = unallocatedInodesInGroup,
            DirectoriesInGroup = directoriesInGroup,
        };

        return binary.ToBlockGroupDescriptor();
    }

    public static void Write(this BinaryWriter writer, BlockGroupDescriptor blockGroupDescriptor)
    {
        var binary = blockGroupDescriptor.ToBinaryBlockGroupDescriptor();

        writer.Write(binary.BlockUsageBitmapBlockAddress);
        writer.Write(binary.InodeUsageBitmapBlockAddress);
        writer.Write(binary.InodeTableStartingBlockAddress);
        writer.Write(binary.UnallocatedBlocksInGroup);
        writer.Write(binary.UnallocatedInodesInGroup);
        writer.Write(binary.DirectoriesInGroup);
        writer.Write(new byte[14]); // (unused)
    }
}