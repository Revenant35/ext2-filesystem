using Filesystem.Models;
using Filesystem.Serializers;

namespace FilesystemTests;

[TestFixture]
public class InodeSerializationTests
{
    [Test]
    public void WriteInode_Should_WriteCorrectBinaryData_When_ValidInodeProvided()
    {
        // Arrange
        var now = DateTimeOffset.UtcNow.ToUnixTimeSeconds();
        var inode = new Inode
        {
            Type = InodeType.Directory,
            Permissions = InodePermissions.UserRead | InodePermissions.GroupRead,
            UserID = 1234,
            SizeBytes = 56789,
            LastAccessed = DateTimeOffset.FromUnixTimeSeconds(now),
            CreatedAt = DateTimeOffset.FromUnixTimeSeconds(now + 1),
            LastModified = DateTimeOffset.FromUnixTimeSeconds(now + 2),
            DeletedAt = DateTimeOffset.FromUnixTimeSeconds(now + 3),
            GroupID = 4321,
            HardLinkCount = 5,
            DiskSectorCount = 250,
            Flags = InodeFlags.ImmutableFile | InodeFlags.FileCompression,
            BlockPointers = Enumerable.Range(1, 12).Select(i => (uint)i).ToArray(),
            SinglyIndirectBlockPointer = 1000,
            DoublyIndirectBlockPointer = 2000,
            TriplyIndirectBlockPointer = 3000,
            GenerationNumber = 123456789,
            FragmentBlockAddress = 42,
            OperatingSystemSpecificValues = [],
        };

        using var ms = new MemoryStream();

        // Act
        using (var writer = new BinaryWriter(ms, System.Text.Encoding.Default, leaveOpen: true))
        {
            writer.Write(inode);
        }

        ms.Position = 0;

        // Assert
        using var reader = new BinaryReader(ms);
        var result = reader.ReadInode();

        Assert.Multiple(() =>
        {
            Assert.That(result.Type, Is.EqualTo(inode.Type));
            Assert.That(result.Permissions, Is.EqualTo(inode.Permissions));
            Assert.That(result.UserID, Is.EqualTo(inode.UserID));
            Assert.That(result.SizeBytes, Is.EqualTo(inode.SizeBytes));
            Assert.That(result.LastAccessed.ToUnixTimeSeconds(), Is.EqualTo(inode.LastAccessed.ToUnixTimeSeconds()));
            Assert.That(result.CreatedAt.ToUnixTimeSeconds(), Is.EqualTo(inode.CreatedAt.ToUnixTimeSeconds()));
            Assert.That(result.LastModified.ToUnixTimeSeconds(), Is.EqualTo(inode.LastModified.ToUnixTimeSeconds()));
            Assert.That(result.DeletedAt.ToUnixTimeSeconds(), Is.EqualTo(inode.DeletedAt.ToUnixTimeSeconds()));
            Assert.That(result.GroupID, Is.EqualTo(inode.GroupID));
            Assert.That(result.HardLinkCount, Is.EqualTo(inode.HardLinkCount));
            Assert.That(result.DiskSectorCount, Is.EqualTo(inode.DiskSectorCount));
            Assert.That(result.Flags, Is.EqualTo(inode.Flags));
            Assert.That(result.BlockPointers, Is.EquivalentTo(inode.BlockPointers));
            Assert.That(result.SinglyIndirectBlockPointer, Is.EqualTo(inode.SinglyIndirectBlockPointer));
            Assert.That(result.DoublyIndirectBlockPointer, Is.EqualTo(inode.DoublyIndirectBlockPointer));
            Assert.That(result.TriplyIndirectBlockPointer, Is.EqualTo(inode.TriplyIndirectBlockPointer));
            Assert.That(result.GenerationNumber, Is.EqualTo(inode.GenerationNumber));
            Assert.That(result.FragmentBlockAddress, Is.EqualTo(inode.FragmentBlockAddress));
        });
    }

    [Test]
    public void ReadInode_Should_ReturnCorrectValues_When_ValidBinaryDataProvided()
    {
        // Arrange
        var type = InodeType.RegularFile;
        var permissions = InodePermissions.UserRead | InodePermissions.UserWrite | InodePermissions.GroupRead;
        var flags = InodeFlags.SecureDeletion | InodeFlags.AppendOnly;
        var now = (uint)DateTimeOffset.UtcNow.ToUnixTimeSeconds();

        using var ms = new MemoryStream();
        using (var writer = new BinaryWriter(ms, System.Text.Encoding.Default, leaveOpen: true))
        {
            writer.Write((ushort)((ushort)type | (ushort)permissions));
            writer.Write((ushort)1001);
            writer.Write((uint)123456);
            writer.Write(now);
            writer.Write(now + 1);
            writer.Write(now + 2);
            writer.Write(now + 3);
            writer.Write((ushort)2002);
            writer.Write((ushort)3);
            writer.Write((uint)100);
            writer.Write((uint)flags);
            writer.Write((uint)0);
            for (var i = 0; i < 12; i++) writer.Write((uint)(i + 100));
            writer.Write((uint)1111);
            writer.Write((uint)2222);
            writer.Write((uint)3333);
            writer.Write((uint)0);
            writer.Write((uint)0);
            writer.Write((uint)4444);
            writer.Write((uint)55);
            writer.Write(new byte[12]);
        }

        ms.Position = 0;

        // Act
        using var reader = new BinaryReader(ms);
        var inode = reader.ReadInode();

        // Assert
        Assert.Multiple(() =>
        {
            Assert.That(inode.Type, Is.EqualTo(type));
            Assert.That(inode.Permissions, Is.EqualTo(permissions));
            Assert.That(inode.UserID, Is.EqualTo(1001));
            Assert.That(inode.SizeBytes, Is.EqualTo(123456));
            Assert.That(inode.LastAccessed.ToUnixTimeSeconds(), Is.EqualTo(now));
            Assert.That(inode.CreatedAt.ToUnixTimeSeconds(), Is.EqualTo(now + 1));
            Assert.That(inode.LastModified.ToUnixTimeSeconds(), Is.EqualTo(now + 2));
            Assert.That(inode.DeletedAt.ToUnixTimeSeconds(), Is.EqualTo(now + 3));
            Assert.That(inode.GroupID, Is.EqualTo(2002));
            Assert.That(inode.HardLinkCount, Is.EqualTo(3));
            Assert.That(inode.DiskSectorCount, Is.EqualTo(100));
            Assert.That(inode.Flags, Is.EqualTo(flags));
            Assert.That(inode.BlockPointers, Is.EqualTo(Enumerable.Range(100, 12).Select(i => (uint)i).ToArray()));
            Assert.That(inode.SinglyIndirectBlockPointer, Is.EqualTo(1111));
            Assert.That(inode.DoublyIndirectBlockPointer, Is.EqualTo(2222));
            Assert.That(inode.TriplyIndirectBlockPointer, Is.EqualTo(3333));
            Assert.That(inode.GenerationNumber, Is.EqualTo(4444));
            Assert.That(inode.FragmentBlockAddress, Is.EqualTo(55));
        });
    }
}