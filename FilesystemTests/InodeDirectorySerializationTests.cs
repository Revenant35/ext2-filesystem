using System.Text;
using Filesystem.Enums;
using Filesystem.Models;
using Filesystem.Serialization.Serializers;

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
            InodeAddress = 42,
            Name = "home",
            Type = InodeDirectoryType.Directory,
            EntrySize = 0,
        };

        using var ms = new MemoryStream();

        // Act
        using (var writer = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            writer.Write(directory);
        }

        ms.Position = 0;

        // Assert
        using var reader = new BinaryReader(ms);
        var inode = reader.ReadUInt32();
        var recLen = reader.ReadUInt16();
        var nameLen = reader.ReadByte();
        var type = (InodeDirectoryType)reader.ReadByte();
        var name = Encoding.UTF8.GetString(reader.ReadBytes(nameLen));
        
        var expectedNameActualLength = (byte)directory.Name.Length;
        var minLengthWithoutPadding = 8 + expectedNameActualLength;
        var expectedRecLen = (ushort)((minLengthWithoutPadding + 3) & ~3);
        var expectedPaddingBytesCount = expectedRecLen - minLengthWithoutPadding;

        Assert.Multiple(() =>
        {
            Assert.That(inode, Is.EqualTo(directory.InodeAddress));
            Assert.That(recLen, Is.EqualTo(expectedRecLen), "Record length mismatch.");
            Assert.That(nameLen, Is.EqualTo(expectedNameActualLength), "Name length mismatch.");
            Assert.That(type, Is.EqualTo(directory.Type), "Directory type mismatch.");
            Assert.That(name, Is.EqualTo(directory.Name), "Name mismatch.");
        });
        
        // Assert that padding was written and consumed correctly
        if (expectedPaddingBytesCount > 0)
        {
            var paddingBytes = reader.ReadBytes(expectedPaddingBytesCount);
            Assert.That(paddingBytes, Is.All.EqualTo(0), "Padding bytes should be null.");
        }
        Assert.That(ms.Position, Is.EqualTo(ms.Length), "Stream should be fully consumed, indicating padding was handled.");
    }

    [Test]
    public void ReadDirectory_Should_ReturnCorrectDirectory_When_ValidBinaryDataProvided()
    {
        // Arrange
        var expectedInodeVal = 99u;
        var expectedNameVal = "etc";
        var expectedTypeVal = InodeDirectoryType.Directory;
        var expectedNameLengthVal = (byte)expectedNameVal.Length;

        var minLengthForDisk = 8 + expectedNameLengthVal;
        var expectedEntrySizeVal = (ushort)((minLengthForDisk + 3) & ~3);
        var paddingBytesCount = expectedEntrySizeVal - minLengthForDisk;

        var nameBytes = Encoding.UTF8.GetBytes(expectedNameVal);
        using var ms = new MemoryStream();
        using (var writer = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            writer.Write(expectedInodeVal);
            writer.Write(expectedEntrySizeVal);
            writer.Write(expectedNameLengthVal);
            writer.Write((byte)expectedTypeVal);
            writer.Write(nameBytes);
            if (paddingBytesCount > 0)
            {
                writer.Write(new byte[paddingBytesCount]);
            }
        }

        ms.Position = 0;

        // Act
        using var reader = new BinaryReader(ms);
        var actualDirectory = reader.ReadInodeDirectory();

        // Assert
        Assert.Multiple(() =>
        {
            Assert.That(actualDirectory.InodeAddress, Is.EqualTo(expectedInodeVal));
            Assert.That(actualDirectory.EntrySize, Is.EqualTo(expectedEntrySizeVal), "EntrySize (rec_len) from disk should be preserved.");
            // NameLength is no longer part of the struct
            Assert.That(actualDirectory.Type, Is.EqualTo(expectedTypeVal));
            Assert.That(actualDirectory.Name, Is.EqualTo(expectedNameVal));
        });
        Assert.That(ms.Position, Is.EqualTo(ms.Length), "Stream should be fully consumed by ReadInodeDirectory, including padding.");
    }

    [Test]
    public void InodeDirectorySerialization_Should_PreserveData_When_WritingAndReading()
    {
        // Arrange
        var originalDirectory = new InodeDirectory
        {
            InodeAddress = 1234,
            Name = "usr",
            Type = InodeDirectoryType.Directory,
            EntrySize = 0,
        };
        var anotherDirectory = new InodeDirectory
        {
            InodeAddress = 5678,
            Name = "local",
            Type = InodeDirectoryType.Directory,
            EntrySize = 0,
        };


        using var ms = new MemoryStream();

        // Act
        using (var writer = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            writer.Write(originalDirectory);
            writer.Write(anotherDirectory);
        }
        ms.Position = 0;
        using var reader = new BinaryReader(ms);
        var deserializedFirst = reader.ReadInodeDirectory();
        var deserializedSecond = reader.ReadInodeDirectory();

        // Assert
        var expectedNameLenFirst = (byte)originalDirectory.Name.Length;
        var minLengthFirst = 8 + expectedNameLenFirst;
        var expectedEntrySizeFirst = (ushort)((minLengthFirst + 3) & ~3);

        Assert.Multiple(() =>
        {
            Assert.That(deserializedFirst.InodeAddress, Is.EqualTo(originalDirectory.InodeAddress), "First Inode mismatch");
            Assert.That(deserializedFirst.EntrySize, Is.EqualTo(expectedEntrySizeFirst), "First EntrySize mismatch");
            Assert.That(deserializedFirst.Type, Is.EqualTo(originalDirectory.Type), "First Type mismatch");
            Assert.That(deserializedFirst.Name, Is.EqualTo(originalDirectory.Name), "First Name mismatch");
        });

        var expectedNameLenSecond = (byte)anotherDirectory.Name.Length;
        var minLengthSecond = 8 + expectedNameLenSecond;
        var expectedEntrySizeSecond = (ushort)((minLengthSecond + 3) & ~3);
        Assert.Multiple(() =>
        {
            Assert.That(deserializedSecond.InodeAddress, Is.EqualTo(anotherDirectory.InodeAddress), "Second Inode mismatch");
            Assert.That(deserializedSecond.EntrySize, Is.EqualTo(expectedEntrySizeSecond), "Second EntrySize mismatch");
            Assert.That(deserializedSecond.Type, Is.EqualTo(anotherDirectory.Type), "Second Type mismatch");
            Assert.That(deserializedSecond.Name, Is.EqualTo(anotherDirectory.Name), "Second Name mismatch");
        });

        Assert.That(ms.Position, Is.EqualTo(ms.Length), "Stream should be fully consumed after reading both entries.");
    }
}