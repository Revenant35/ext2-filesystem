namespace Filesystem.Mapping;

using AutoMapper;
using Enums;
using Models;
using Serialization.Models;


public class InodeProfile : Profile
{
    public InodeProfile()
    {
        CreateMap<BinaryInode, Inode>()
            .ForMember(dest => dest.Type, opt => opt.MapFrom(src => (InodeType)(src.TypeAndPermissions & 0xF000)))
            .ForMember(dest => dest.Permissions, opt => opt.MapFrom(src => (InodePermissions)(src.TypeAndPermissions & 0x0FFF)))
            .ForMember(dest => dest.LastAccessed, opt => opt.MapFrom<DateTimeOffset?>(src => src.LastAccessed != 0 ? DateTimeOffset.FromUnixTimeSeconds(src.LastAccessed) : null))
            .ForMember(dest => dest.CreatedAt, opt => opt.MapFrom(src => DateTimeOffset.FromUnixTimeSeconds(src.CreatedAt)))
            .ForMember(dest => dest.LastModified, opt => opt.MapFrom<DateTimeOffset?>(src => src.LastModified != 0 ? DateTimeOffset.FromUnixTimeSeconds(src.LastModified) : null))
            .ForMember(dest => dest.DeletedAt, opt => opt.MapFrom<DateTimeOffset?>(src => src.DeletedAt != 0 ? DateTimeOffset.FromUnixTimeSeconds(src.DeletedAt) : null))
            .ForMember(dest => dest.Flags, opt => opt.MapFrom(src => (InodeFlags)src.Flags))
            .ForMember(dest => dest.BlockAddresses, opt => opt.MapFrom(src => src.BlockAddresses.Take(12).Where(u => u != 0).ToList()))
            .ForMember(dest => dest.SinglyIndirectBlockAddress, opt => opt.MapFrom<uint?>(src => src.SinglyIndirectBlockAddress != 0 ? src.SinglyIndirectBlockAddress : null))
            .ForMember(dest => dest.DoublyIndirectBlockAddress, opt => opt.MapFrom<uint?>(src => src.DoublyIndirectBlockAddress != 0 ? src.DoublyIndirectBlockAddress : null))
            .ForMember(dest => dest.TriplyIndirectBlockAddress, opt => opt.MapFrom<uint?>(src => src.TriplyIndirectBlockAddress != 0 ? src.TriplyIndirectBlockAddress : null));

        CreateMap<Inode, BinaryInode>()
            .ForMember(dest => dest.TypeAndPermissions, opt => opt.MapFrom(src => (ushort)((ushort)src.Type | (ushort)src.Permissions)))
            .ForMember(dest => dest.LastAccessed, opt => opt.MapFrom(src => src.LastAccessed != null ? Convert.ToUInt32(src.LastAccessed.Value.ToUnixTimeSeconds()) : 0))
            .ForMember(dest => dest.CreatedAt, opt => opt.MapFrom(src => Convert.ToUInt32(src.CreatedAt.ToUnixTimeSeconds())))
            .ForMember(dest => dest.LastModified, opt => opt.MapFrom(src => src.LastModified != null ? Convert.ToUInt32(src.LastModified.Value.ToUnixTimeSeconds()) : 0))
            .ForMember(dest => dest.DeletedAt, opt => opt.MapFrom(src => src.DeletedAt != null ? Convert.ToUInt32(src.DeletedAt.Value.ToUnixTimeSeconds()) : 0))
            .ForMember(dest => dest.Flags, opt => opt.MapFrom(src => (uint)src.Flags))
            .ForMember(dest => dest.BlockAddresses, opt => opt.MapFrom(src => src.BlockAddresses.Take(12).Concat(Enumerable.Repeat(0u, 12 - src.BlockAddresses.Count)).ToArray()))
            .ForMember(dest => dest.SinglyIndirectBlockAddress, opt => opt.MapFrom(src => src.SinglyIndirectBlockAddress ?? 0))
            .ForMember(dest => dest.DoublyIndirectBlockAddress, opt => opt.MapFrom(src => src.DoublyIndirectBlockAddress ?? 0))
            .ForMember(dest => dest.TriplyIndirectBlockAddress, opt => opt.MapFrom(src => src.TriplyIndirectBlockAddress ?? 0));
    }
}
