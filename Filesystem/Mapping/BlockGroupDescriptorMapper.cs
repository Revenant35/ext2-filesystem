namespace Filesystem.Mapping;

using Models;
using Serialization.Models;

public static class BlockGroupDescriptorMapper
{
    public static BlockGroupDescriptor ToBlockGroupDescriptor(this BinaryBlockGroupDescriptor binary) => new()
    {
        BlockUsageBitmapBlockAddress = binary.BlockUsageBitmapBlockAddress,
        InodeUsageBitmapBlockAddress = binary.InodeUsageBitmapBlockAddress,
        InodeTableStartingBlockAddress = binary.InodeTableStartingBlockAddress,
        UnallocatedBlocksInGroup = binary.UnallocatedBlocksInGroup,
        UnallocatedInodesInGroup = binary.UnallocatedInodesInGroup,
        DirectoriesInGroup = binary.DirectoriesInGroup,
    };

    public static BinaryBlockGroupDescriptor ToBinaryBlockGroupDescriptor(this BlockGroupDescriptor blockGroupDescriptor) => new()
    {
        BlockUsageBitmapBlockAddress = blockGroupDescriptor.BlockUsageBitmapBlockAddress,
        InodeUsageBitmapBlockAddress = blockGroupDescriptor.InodeUsageBitmapBlockAddress,
        InodeTableStartingBlockAddress = blockGroupDescriptor.InodeTableStartingBlockAddress,
        UnallocatedBlocksInGroup = blockGroupDescriptor.UnallocatedBlocksInGroup,
        UnallocatedInodesInGroup = blockGroupDescriptor.UnallocatedInodesInGroup,
        DirectoriesInGroup = blockGroupDescriptor.DirectoriesInGroup,
    };
}