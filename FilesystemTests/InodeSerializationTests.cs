using Filesystem.Models;

namespace FilesystemTests;

using Filesystem.Enums;
using Filesystem.Serialization.Serializers;

[TestFixture]
public class InodeSerializationTests
{
    [Test]
    public void InodeSerializer_Should_ReadAndWriteCorrectData_WithNonDefaultValues()
    {
        // Arrange
        var originalInode = new Inode
        {
            Type = InodeType.Directory,
            Permissions = InodePermissions.UserRead |
                          InodePermissions.UserWrite |
                          InodePermissions.UserExecute |
                          InodePermissions.GroupRead |
                          InodePermissions.GroupExecute |
                          InodePermissions.OtherRead |
                          InodePermissions.OtherExecute,
            UserID = 1001,
            SizeBytes = 4096,
            LastAccessed = DateTimeOffset.UtcNow.AddHours(-1)
                .ToUniversalTime(),
            CreatedAt = DateTimeOffset.UtcNow.AddDays(-1)
                .ToUniversalTime(),
            LastModified = DateTimeOffset.UtcNow.AddMinutes(-30)
                .ToUniversalTime(),
            DeletedAt = null,
            GroupID = 2002,
            HardLinkCount = 1,
            DiskSectorCount = 8,
            Flags = InodeFlags.ImmutableFile | InodeFlags.AppendOnly,
            BlockAddresses = Enumerable.Range(1, 12)
                .Select(i => (uint)i * 100)
                .ToList(),
            GenerationNumber = 0x12345678,
            FileACLBlock = 1000,
            DirectoryACLBlock = 2000,
            FragmentBlockAddress = 3000,
            OperatingSystemSpecificValues =
            [
                ..Enumerable.Range(0,
                        4)
                    .Select(i => (byte)(i + 0x40)),
                ..Enumerable.Range(0,
                        12)
                    .Select(i => (byte)(i + 0x40))
            ],
            SinglyIndirectBlockAddress = null,
            DoublyIndirectBlockAddress = null,
            TriplyIndirectBlockAddress = null
        };

        using var ms = new MemoryStream();

        // Act
        using (var writer = new BinaryWriter(ms, System.Text.Encoding.Default, leaveOpen: true))
        {
            writer.Write(originalInode);
        }

        ms.Position = 0;

        // Assert
        using var reader = new BinaryReader(ms);
        var deserializedInode = reader.ReadInode();

        Assert.Multiple(() =>
        {
            Assert.That(deserializedInode.Type, Is.EqualTo(originalInode.Type));
            Assert.That(deserializedInode.Permissions, Is.EqualTo(originalInode.Permissions));
            Assert.That(deserializedInode.UserID, Is.EqualTo(originalInode.UserID));
            Assert.That(deserializedInode.SizeBytes, Is.EqualTo(originalInode.SizeBytes));
            Assert.That(deserializedInode.LastAccessed?.ToUnixTimeSeconds(), Is.EqualTo(originalInode.LastAccessed?.ToUnixTimeSeconds()));
            Assert.That(deserializedInode.CreatedAt.ToUnixTimeSeconds(), Is.EqualTo(originalInode.CreatedAt.ToUnixTimeSeconds()));
            Assert.That(deserializedInode.LastModified?.ToUnixTimeSeconds(), Is.EqualTo(originalInode.LastModified?.ToUnixTimeSeconds()));
            Assert.That(deserializedInode.DeletedAt?.ToUnixTimeSeconds(), Is.EqualTo(originalInode.DeletedAt?.ToUnixTimeSeconds()));
            Assert.That(deserializedInode.GroupID, Is.EqualTo(originalInode.GroupID));
            Assert.That(deserializedInode.HardLinkCount, Is.EqualTo(originalInode.HardLinkCount));
            Assert.That(deserializedInode.DiskSectorCount, Is.EqualTo(originalInode.DiskSectorCount));
            Assert.That(deserializedInode.Flags, Is.EqualTo(originalInode.Flags));
            Assert.That(deserializedInode.BlockAddresses, Is.EqualTo(originalInode.BlockAddresses));
            Assert.That(deserializedInode.GenerationNumber, Is.EqualTo(originalInode.GenerationNumber));
            Assert.That(deserializedInode.FileACLBlock, Is.EqualTo(originalInode.FileACLBlock));
            Assert.That(deserializedInode.DirectoryACLBlock, Is.EqualTo(originalInode.DirectoryACLBlock));
            Assert.That(deserializedInode.FragmentBlockAddress, Is.EqualTo(originalInode.FragmentBlockAddress));
            Assert.That(deserializedInode.OperatingSystemSpecificValues, Is.EqualTo(originalInode.OperatingSystemSpecificValues));
        });
    }
}