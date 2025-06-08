namespace Filesystem.Serialization.Serializers;

using Filesystem.Models;
using Mapping;
using Models;

// https://wiki.osdev.org/Ext2#Directory_Entry
public static class InodeDirectorySerializer
{
    private const int Ext2MaxNameLength = 255;
    private const int DirectoryEntryFixedFieldsSize = 8;
    private const int DirectoryEntryAlignment = 4;

    public static InodeDirectory ReadInodeDirectory(this BinaryReader reader)
    {
        var inodeAddress = reader.ReadUInt32();
        var entrySize = reader.ReadUInt16();
        var nameLength = reader.ReadByte();
        var directoryType = reader.ReadByte();
        var nameBytes = reader.ReadBytes(nameLength);

        var consumedDataSize = DirectoryEntryFixedFieldsSize + nameLength;
        var padding = entrySize - consumedDataSize;

        if (padding < 0)
        {
            throw new InvalidDataException("Directory entry size (rec_len) is smaller than the sum of its fixed parts and name length.");
        }

        reader.ReadBytes(padding);

        var binary = new BinaryInodeDirectory
        {
            InodeAddress = inodeAddress,
            EntrySize = entrySize,
            NameLength = nameLength,
            Type = directoryType,
            Name = nameBytes,
        };

        return binary.ToInodeDirectory();
    }

    public static void Write(this BinaryWriter writer, InodeDirectory inodeDirectory)
    {
        var binary = inodeDirectory.ToBinaryInodeDirectory();

        var minLengthWithoutPadding = DirectoryEntryFixedFieldsSize + binary.NameLength;
        var actualEntrySize = (ushort)(minLengthWithoutPadding + DirectoryEntryAlignment - 1 & ~(DirectoryEntryAlignment - 1));
        var paddingBytesToWrite = actualEntrySize - minLengthWithoutPadding;

        writer.Write(inodeDirectory.InodeAddress);
        writer.Write(actualEntrySize);
        writer.Write(binary.NameLength);
        writer.Write((byte)inodeDirectory.Type);
        writer.Write(binary.Name);

        if (paddingBytesToWrite > 0)
        {
            writer.Write(new byte[paddingBytesToWrite]);
        }
    }
}