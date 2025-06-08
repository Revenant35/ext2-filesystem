namespace Filesystem.Mapping;

using Enums;
using Models;
using Serialization.Models;
using System.Text;

public static class InodeDirectoryMapper
{
    public static BinaryInodeDirectory ToBinaryInodeDirectory(this InodeDirectory directory) => new()
    {
        InodeAddress = directory.InodeAddress,
        EntrySize = directory.EntrySize,
        NameLength = (byte)directory.Name.Length,
        Type = (byte)directory.Type,
        Name = Encoding.Latin1.GetBytes(directory.Name),
    };

    public static InodeDirectory ToInodeDirectory(this BinaryInodeDirectory binary) => new()
    {
        InodeAddress = binary.InodeAddress,
        EntrySize = binary.EntrySize,
        Type = (InodeDirectoryType)binary.Type,
        Name = Encoding.Latin1.GetString(binary.Name),
    };
}