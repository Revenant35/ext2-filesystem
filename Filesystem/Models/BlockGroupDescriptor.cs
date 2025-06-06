namespace Filesystem.Models;

// https://wiki.osdev.org/Ext2#Block_Group_Descriptor_Table
public struct BlockGroupDescriptor
{
    public uint BlockUsageBitmapBlockAddress;
    public uint InodeUsageBitmapBlockAddress;
    public uint InodeTableStartingBlockAddress;
    public ushort UnallocatedBlocksInGroup;
    public ushort UnallocatedInodesInGroup;
    public ushort DirectoriesInGroup;
}