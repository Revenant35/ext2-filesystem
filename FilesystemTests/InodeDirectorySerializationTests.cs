using System.Text;
using Filesystem.Models;
using Filesystem.Serializers;

namespace FilesystemTests;

[TestFixture]
public class InodeDirectorySerializationTests
{
    [Test]
    public void WriteInodeDirectory_Should_WriteCorrectBinaryData_When_ValidDirectoryProvided()
    {
        // Arrange
        var directory = new InodeDirectory
        {
            Inode = 42,
            Name = "home",
            Type = InodeDirectoryType.Directory,
            EntrySize = 555,
        };

        using var ms = new MemoryStream();

        // Act
        using (var writer = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            writer.WriteInodeDirectory(directory);
        }

        ms.Position = 0;

        // Assert
        using var reader = new BinaryReader(ms);
        var inode = reader.ReadUInt32();
        var recLen = reader.ReadUInt16();
        var nameLen = reader.ReadByte();
        var type = (InodeDirectoryType)reader.ReadByte();
        var name = Encoding.UTF8.GetString(reader.ReadBytes(nameLen));

        Assert.Multiple(() =>
        {
            Assert.That(inode, Is.EqualTo(directory.Inode));
            Assert.That(recLen, Is.EqualTo(directory.EntrySize));
            Assert.That(nameLen, Is.EqualTo(directory.Name.Length));
            Assert.That(type, Is.EqualTo(directory.Type));
            Assert.That(name, Is.EqualTo(directory.Name));
        });
    }

    [Test]
    public void ReadDirectory_Should_ReturnCorrectDirectory_When_ValidBinaryDataProvided()
    {
        // Arrange
        var expected = new InodeDirectory
        {
            Inode = 99,
            Name = "etc",
            Type = InodeDirectoryType.Directory,
            EntrySize = 12 + 3 // 12 header + 3 name bytes = 15, should be padded to 16 externally
        };

        var nameBytes = Encoding.UTF8.GetBytes(expected.Name);
        using var ms = new MemoryStream();
        using (var writer = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            writer.Write(expected.Inode);
            writer.Write((ushort)expected.EntrySize);
            writer.Write((byte)nameBytes.Length);
            writer.Write((byte)expected.Type);
            writer.Write(nameBytes);
        }

        ms.Position = 0;

        // Act
        using var reader = new BinaryReader(ms);
        var actual = reader.ReadInodeDirectory();

        // Assert
        Assert.Multiple(() =>
        {
            Assert.That(actual.Inode, Is.EqualTo(expected.Inode));
            Assert.That(actual.EntrySize, Is.EqualTo(expected.EntrySize));
            Assert.That(actual.Type, Is.EqualTo(expected.Type));
            Assert.That(actual.Name, Is.EqualTo(expected.Name));
        });
    }

    [Test]
    public void InodeDirectorySerialization_Should_PreserveData_When_WritingAndReading()
    {
        // Arrange
        var original = new InodeDirectory
        {
            Inode = 1234,
            Name = "usr",
            Type = InodeDirectoryType.Directory,
            EntrySize = 12 + 3 // header + name
        };

        using var ms = new MemoryStream();

        // Act
        using (var writer = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            writer.WriteInodeDirectory(original);
        }

        ms.Position = 0;

        using var reader = new BinaryReader(ms);
        var deserialized = reader.ReadInodeDirectory();

        // Assert
        Assert.Multiple(() =>
        {
            Assert.That(deserialized.Inode, Is.EqualTo(original.Inode));
            Assert.That(deserialized.EntrySize, Is.EqualTo(original.EntrySize));
            Assert.That(deserialized.Type, Is.EqualTo(original.Type));
            Assert.That(deserialized.Name, Is.EqualTo(original.Name));
        });
    }
}