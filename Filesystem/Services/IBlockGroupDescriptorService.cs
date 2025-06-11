namespace Filesystem.Services;

using Models;

public interface IBlockGroupDescriptorService
{
    public BlockGroupDescriptor[] BlockGroupDescriptors { get; }
    
    public BlockGroupDescriptor[] ReadBlockGroupDescriptorTable();
    public void WriteBlockGroupDescriptorTable(BlockGroupDescriptor[] descriptors);
}