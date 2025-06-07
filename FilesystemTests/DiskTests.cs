namespace FilesystemTests;

using Filesystem;
using Filesystem.Models;
using Filesystem.Serializers;
using System.Text;

[TestFixture]
public class DiskTests
{
    [Test]
    public void Load_Should_LoadSuperblock()
    {
        // Arrange
        using var ms = new MemoryStream();
        var formatter = new DiskFormatter(ms);
        formatter.Format(Superblock.Default);
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
    
    [Test]
    public void ReadInode_Should_ReturnExpectedInode_WhenStreamIsValid()
    {
        // Arrange
        var inodeIndex = 1u;
        var expectedInode = Inode.Default;

        using var ms = new MemoryStream();
        var formatter = new DiskFormatter(ms);
        formatter.Format(Superblock.Default);

        var disk = new Disk(ms);
        var inodeOffset = disk.GetInodeOffset(inodeIndex);
    
        ms.Position = inodeOffset!;
        using (var writer = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true))
        {
            writer.Write(expectedInode);
        }

        ms.Position = 0;

        // Act
        var result = disk.ReadInode(inodeIndex);

        // Assert
        Assert.Multiple(() =>
        {
            Assert.That(result.SizeBytes, Is.EqualTo(expectedInode.SizeBytes));
        });
    }
    [Test]
    public void ReadBlockBitmap_Should_ReturnCorrectBitmap_WhenValid()
    {
        // Arrange
        var blockSize = Superblock.Default.BlockSize;
        var blockBitmapAddress = 5u;
        var bitmapData = new byte[blockSize];
        bitmapData[0] = 0b_00001111;

        using var ms = new MemoryStream();
        var formatter = new DiskFormatter(ms);
        formatter.Format(Superblock.Default);

        ms.Position = blockBitmapAddress * blockSize;
        ms.Write(bitmapData, 0, bitmapData.Length);
        ms.Position = 0;

        var disk = new Disk(ms);
        var descriptor = new BlockGroupDescriptor
        {
            BlockUsageBitmapBlockAddress = blockBitmapAddress
        };

        // Act
        var bitmap = disk.ReadBlockBitmap(descriptor);

        // Assert
        Assert.Multiple(() =>
        {
            Assert.That(bitmap.Test(0), Is.True);
            Assert.That(bitmap.Test(1), Is.True);
            Assert.That(bitmap.Test(2), Is.True);
            Assert.That(bitmap.Test(3), Is.True);
            Assert.That(bitmap.Test(4), Is.False);
        });
    }
}