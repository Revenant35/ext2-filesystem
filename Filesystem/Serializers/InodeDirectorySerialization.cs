namespace Filesystem.Serializers;

using Models;

// https://wiki.osdev.org/Ext2#Directory_Entry
public static class InodeDirectorySerialization
{
    public static InodeDirectory ReadInodeDirectory(this BinaryReader reader)
    {
        var inode = reader.ReadUInt32();
        var entrySize = reader.ReadUInt16();
        var nameLength = reader.ReadByte();
        var directoryType = (InodeDirectoryType)reader.ReadByte();
        var nameBytes = reader.ReadBytes(nameLength);
        var name = System.Text.Encoding.UTF8.GetString(nameBytes);

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
        var nameBytes = System.Text.Encoding.UTF8.GetBytes(inodeDirectory.Name);
        writer.Write(inodeDirectory.Inode);
        writer.Write(inodeDirectory.EntrySize);
        writer.Write((byte)nameBytes.Length);
        writer.Write((byte)inodeDirectory.Type);
        writer.Write(nameBytes);
    }
}