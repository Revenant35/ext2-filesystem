namespace FilesystemTests;

using Filesystem;
using Filesystem.Models;
using System.Text;

[TestFixture]
public class FileSystemTests
{
    [Test]
    public void ResolvePath_Should_FindAndReadFile_WhenLocatedInRootDirectory()
    {
        // Arrange
        string fileName = "hello.txt";
        var fileContent = Encoding.UTF8.GetBytes("This is indirect block file content.");
        var (fs, expectedInode, expectedContent) = TestImageBuilder.CreateFileUnderRoot(fileName, fileContent);

        // Act
        var inode = fs.ResolvePath($"/{fileName}");
        var content = fs.ReadFile(inode);

        // Assert
        Assert.Multiple(() =>
        {
            Assert.That(inode.Type, Is.EqualTo(InodeType.File));
            Assert.That(inode.SizeBytes, Is.EqualTo((uint)expectedContent.Length));
            Assert.That(content, Is.EqualTo(expectedContent));
        });
    }
}