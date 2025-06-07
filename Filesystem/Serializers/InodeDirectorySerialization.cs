namespace Filesystem.Serializers;

using Models;

// https://wiki.osdev.org/Ext2#Directory_Entry
public static class InodeDirectorySerialization
{
    private const int Ext2MaxNameLength = 255;
    private const int DirectoryEntryFixedFieldsSize = 8;
    private const int DirectoryEntryAlignment = 4;

    public static InodeDirectory ReadInodeDirectory(this BinaryReader reader)
    {
        var inode = reader.ReadUInt32();
        var entrySize = reader.ReadUInt16();
        var nameLength = reader.ReadByte();
        var directoryType = (InodeDirectoryType)reader.ReadByte();
        
        if (nameLength > Ext2MaxNameLength)
        {
            throw new InvalidDataException($"Corrupt directory entry: nameLength {nameLength} exceeds maximum allowed length of {Ext2MaxNameLength}.");
        }

        var nameBytes = reader.ReadBytes(nameLength);
        var name = System.Text.Encoding.UTF8.GetString(nameBytes);

        var consumedDataSize = DirectoryEntryFixedFieldsSize + nameLength;
        var padding = entrySize - consumedDataSize;

        if (padding < 0) 
        {
            throw new InvalidDataException("Directory entry size (rec_len) is smaller than the sum of its fixed parts and name length.");
        }
        if (padding > 0)
        {
            reader.ReadBytes(padding);
        }

        return new InodeDirectory
        {
            Inode = inode,
            EntrySize = entrySize,
            Type = directoryType,
            Name = name
        };
    }

    public static void Write(this BinaryWriter writer, InodeDirectory inodeDirectory)
    {
        var nameBytes = System.Text.Encoding.UTF8.GetBytes(inodeDirectory.Name ?? string.Empty);
        var nameLength = (byte)nameBytes.Length;

        var minLengthWithoutPadding = DirectoryEntryFixedFieldsSize + nameLength;
        var actualEntrySize = (ushort)((minLengthWithoutPadding + DirectoryEntryAlignment - 1) & ~(DirectoryEntryAlignment - 1));
        var paddingBytesToWrite = actualEntrySize - minLengthWithoutPadding;

        writer.Write(inodeDirectory.Inode);
        writer.Write(actualEntrySize);
        writer.Write(nameLength);
        writer.Write((byte)inodeDirectory.Type);
        writer.Write(nameBytes);

        if (paddingBytesToWrite > 0)
        {
            writer.Write(new byte[paddingBytesToWrite]);
        }
    }
}