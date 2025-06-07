namespace FilesystemTests;

using Filesystem;
using Filesystem.Models;
using Filesystem.Serializers;

[TestFixture]
public class DiskTests
{
    [Test]
    public void Load_Should_LoadSuperblock()
    {
        // Arrange
        using var ms = new MemoryStream();
        ms.Position = Disk.SuperblockOffset;
        
        using (var writer = new BinaryWriter(ms, System.Text.Encoding.Default, leaveOpen: true))
        {
            writer.Write(Superblock.Default);
        }
        
        ms.Position = 0;

        // Act
        var disk = new Disk(ms);

        // Assert
        Assert.Multiple(() =>
        {
            Assert.That(disk.InodeCount, Is.EqualTo(Superblock.Default.InodeCount));
            Assert.That(disk.BlockCount, Is.EqualTo(Superblock.Default.BlockCount));
            Assert.That(disk.BlockSize, Is.EqualTo(Superblock.Default.BlockSize));
            Assert.That(disk.FragmentSize, Is.EqualTo(Superblock.Default.FragmentSize));
            Assert.That(disk.BlocksPerGroup, Is.EqualTo(Superblock.Default.BlocksPerGroup));
            Assert.That(disk.InodesPerGroup, Is.EqualTo(Superblock.Default.InodesPerGroup));
        });
    }
}