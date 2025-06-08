namespace FilesystemTests;

using Filesystem.Models;
using Filesystem.Serialization.Serializers;
using NUnit.Framework;
using System.IO;

[TestFixture]
public class BlockGroupDescriptorSerializationTests
{
    [Test]
    public void ReadBlockGroupDescriptor_Should_ReturnCorrectValues_When_ValidBinaryDataProvided()
    {
        // Arrange
        var expected = new BlockGroupDescriptor
        {
            BlockUsageBitmapBlockAddress = 1,
            InodeUsageBitmapBlockAddress = 2,
            InodeTableStartingBlockAddress = 3,
            UnallocatedBlocksInGroup = 4,
            UnallocatedInodesInGroup = 5,
            DirectoriesInGroup = 6,
        };

        using var ms = new MemoryStream();
        using (var writer = new BinaryWriter(ms, System.Text.Encoding.Default, leaveOpen: true))
        {
            writer.Write(expected.BlockUsageBitmapBlockAddress);
            writer.Write(expected.InodeUsageBitmapBlockAddress);
            writer.Write(expected.InodeTableStartingBlockAddress);
            writer.Write(expected.UnallocatedBlocksInGroup);
            writer.Write(expected.UnallocatedInodesInGroup);
            writer.Write(expected.DirectoriesInGroup);
            writer.Write(new byte[14]);
        }

        ms.Position = 0;

        // Act
        using var reader = new BinaryReader(ms);
        var actual = reader.ReadBlockGroupDescriptor();

        // Assert
        Assert.Multiple(() =>
        {
            Assert.That(actual.BlockUsageBitmapBlockAddress, Is.EqualTo(expected.BlockUsageBitmapBlockAddress));
            Assert.That(actual.InodeUsageBitmapBlockAddress, Is.EqualTo(expected.InodeUsageBitmapBlockAddress));
            Assert.That(actual.InodeTableStartingBlockAddress, Is.EqualTo(expected.InodeTableStartingBlockAddress));
            Assert.That(actual.UnallocatedBlocksInGroup, Is.EqualTo(expected.UnallocatedBlocksInGroup));
            Assert.That(actual.UnallocatedInodesInGroup, Is.EqualTo(expected.UnallocatedInodesInGroup));
            Assert.That(actual.DirectoriesInGroup, Is.EqualTo(expected.DirectoriesInGroup));
        });
    }

    [Test]
    public void WriteBlockGroupDescriptor_Should_WriteCorrectBinaryData_When_ValidStructProvided()
    {
        // Arrange
        var input = new BlockGroupDescriptor
        {
            BlockUsageBitmapBlockAddress = 10,
            InodeUsageBitmapBlockAddress = 20,
            InodeTableStartingBlockAddress = 30,
            UnallocatedBlocksInGroup = 40,
            UnallocatedInodesInGroup = 50,
            DirectoriesInGroup = 60
        };

        using var ms = new MemoryStream();

        // Act
        using (var writer = new BinaryWriter(ms, System.Text.Encoding.Default, leaveOpen: true))
        {
            writer.Write(input);
        }

        ms.Position = 0;

        // Assert
        using var reader = new BinaryReader(ms);
        Assert.Multiple(() =>
        {
            Assert.That(reader.ReadUInt32(), Is.EqualTo(input.BlockUsageBitmapBlockAddress));
            Assert.That(reader.ReadUInt32(), Is.EqualTo(input.InodeUsageBitmapBlockAddress));
            Assert.That(reader.ReadUInt32(), Is.EqualTo(input.InodeTableStartingBlockAddress));
            Assert.That(reader.ReadUInt16(), Is.EqualTo(input.UnallocatedBlocksInGroup));
            Assert.That(reader.ReadUInt16(), Is.EqualTo(input.UnallocatedInodesInGroup));
            Assert.That(reader.ReadUInt16(), Is.EqualTo(input.DirectoriesInGroup));

            var padding = reader.ReadBytes(14);
            Assert.That(padding.Length, Is.EqualTo(14));
            Assert.That(padding.All(b => b == 0), Is.True);
        });
    }
}