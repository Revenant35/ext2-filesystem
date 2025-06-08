namespace Filesystem.Serialization.Models;

// https://wiki.osdev.org/Ext2#Block_Group_Descriptor_Table
public struct BinaryBlockGroupDescriptor
{
    public const long SizeOnDiskInBytes = 32;

    public required uint BlockUsageBitmapBlockAddress;
    public required uint InodeUsageBitmapBlockAddress;
    public required uint InodeTableStartingBlockAddress;
    public required ushort UnallocatedBlocksInGroup;
    public required ushort UnallocatedInodesInGroup;
    public required ushort DirectoriesInGroup;
}