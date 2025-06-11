using AutoMapper;
using System.Numerics;

namespace Filesystem.Mapping;

using Enums;
using Models;
using Serialization.Models;

public class SuperblockProfile : Profile
{
    public SuperblockProfile()
    {
        CreateMap<BinarySuperblock, Superblock>()
            .ForMember(dest => dest.BlockSize, opt => opt.MapFrom(src => 1024u << Convert.ToInt32(src.BlockSize)))
            .ForMember(dest => dest.FragmentSize, opt => opt.MapFrom(src => 1024u << Convert.ToInt32(src.FragmentSize)))
            .ForMember(dest => dest.LastMountTime, opt => opt.MapFrom(src => DateTimeOffset.FromUnixTimeSeconds(src.LastMountTime)))
            .ForMember(dest => dest.LastWrittenTime, opt => opt.MapFrom(src => DateTimeOffset.FromUnixTimeSeconds(src.LastWrittenTime)))
            .ForMember(dest => dest.LastCheckTime, opt => opt.MapFrom(src => DateTimeOffset.FromUnixTimeSeconds(src.LastCheckTime)))
            .ForMember(dest => dest.CheckInterval, opt => opt.MapFrom(src => TimeSpan.FromSeconds(src.CheckInterval)))
            .ForMember(dest => dest.FileSystemState, opt => opt.MapFrom(src => (FileSystemState)src.FileSystemState))
            .ForMember(dest => dest.ErrorHandling, opt => opt.MapFrom(src => (ErrorHandling)src.ErrorHandling))
            .ForMember(dest => dest.OperatingSystemId, opt => opt.MapFrom(src => (OperatingSystemID)src.OperatingSystemId));

        CreateMap<Superblock, BinarySuperblock>()
            .ForMember(dest => dest.BlockSize, opt => opt.MapFrom(src => BitOperations.Log2(src.BlockSize) - 10))
            .ForMember(dest => dest.FragmentSize, opt => opt.MapFrom(src => BitOperations.Log2(src.FragmentSize) - 10))
            .ForMember(dest => dest.LastMountTime, opt => opt.MapFrom(src => Convert.ToUInt32(src.LastMountTime.ToUnixTimeSeconds())))
            .ForMember(dest => dest.LastWrittenTime, opt => opt.MapFrom(src => Convert.ToUInt32(src.LastWrittenTime.ToUnixTimeSeconds())))
            .ForMember(dest => dest.LastCheckTime, opt => opt.MapFrom(src => Convert.ToUInt32(src.LastCheckTime.ToUnixTimeSeconds())))
            .ForMember(dest => dest.CheckInterval, opt => opt.MapFrom(src => Convert.ToUInt32(src.CheckInterval.TotalSeconds)))
            .ForMember(dest => dest.FileSystemState, opt => opt.MapFrom(src => (ushort)src.FileSystemState))
            .ForMember(dest => dest.ErrorHandling, opt => opt.MapFrom(src => (ushort)src.ErrorHandling))
            .ForMember(dest => dest.OperatingSystemId, opt => opt.MapFrom(src => (uint)src.OperatingSystemId));
            
    }
}