using Filesystem.Models;
using Filesystem.Serializers;

namespace FilesystemTests;

[TestFixture]
public class InodeSerializationTests
{
    [Test]
    public void InodeSerializer_Should_ReadAndWriteCorrectData()
    {
        // Arrange
        var inode = new Inode();
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
            Assert.That(result.LastAccessed?.ToUnixTimeSeconds(), Is.EqualTo(inode.LastAccessed?.ToUnixTimeSeconds()));
            Assert.That(result.CreatedAt.ToUnixTimeSeconds(), Is.EqualTo(inode.CreatedAt.ToUnixTimeSeconds()));
            Assert.That(result.LastModified?.ToUnixTimeSeconds(), Is.EqualTo(inode.LastModified?.ToUnixTimeSeconds()));
            Assert.That(result.DeletedAt?.ToUnixTimeSeconds(), Is.EqualTo(inode.DeletedAt?.ToUnixTimeSeconds()));
            Assert.That(result.GroupID, Is.EqualTo(inode.GroupID));
            Assert.That(result.HardLinkCount, Is.EqualTo(inode.HardLinkCount));
            Assert.That(result.DiskSectorCount, Is.EqualTo(inode.DiskSectorCount));
            Assert.That(result.Flags, Is.EqualTo(inode.Flags));
            Assert.That(result.BlockPointers, Is.EquivalentTo(inode.BlockPointers));
            Assert.That(result.GenerationNumber, Is.EqualTo(inode.GenerationNumber));
            Assert.That(result.FragmentBlockAddress, Is.EqualTo(inode.FragmentBlockAddress));
        });
    }
}