namespace Filesystem.Mapping;

using AutoMapper;
using Enums;
using Models;
using System.Text;

public class InodeDirectoryProfile : Profile
{
    public InodeDirectoryProfile()
    {
        CreateMap<BinaryInodeDirectory, InodeDirectory>()
            .ForMember(dest => dest.Name, opt => opt.MapFrom(src => Encoding.Latin1.GetString(src.Name)))
            .ForMember(dest => dest.Type, opt => opt.MapFrom(src => (InodeDirectoryType)src.Type));
        
        CreateMap<InodeDirectory, BinaryInodeDirectory>()
            .ForMember(dest => dest.Name, opt => opt.MapFrom(src => Encoding.Latin1.GetBytes(src.Name)))
            .ForMember(dest => dest.Type, opt => opt.MapFrom(src => (byte)src.Type))
            .ForMember(dest => dest.NameLength, opt => opt.MapFrom(src => (byte)src.Name.Length));
    }
}