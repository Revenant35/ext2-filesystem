namespace Filesystem.Mapping;

using AutoMapper;
using Models;

public class BlockGroupDescriptorProfile : Profile
{
    public BlockGroupDescriptorProfile()
    {
        CreateMap<BinaryBlockGroupDescriptor, BlockGroupDescriptor>();

        CreateMap<BlockGroupDescriptor, BinaryBlockGroupDescriptor>();
    }
}