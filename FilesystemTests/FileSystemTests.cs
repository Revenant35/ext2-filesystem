namespace FilesystemTests;

using Filesystem;
using Filesystem.Models;

[TestFixture]
public class FileSystemTests
{
    [Test]
    public void ResolvePath_Should_FindAndReadFile_WhenLocatedInRootDirectory()
    {
        // Arrange
        var fileName = "hello.txt";
        var fileContent = "This is indirect block file content."u8.ToArray();
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

    [Test]
    public void ResolvePath_To_File_In_SubDirectory_ResolvesCorrectly()
    {
        // Arrange
        var subDirName = "testdir";
        var fileNameInSubDir = "nestedfile.txt";
        var fullPath = $"/{subDirName}/{fileNameInSubDir}";
        var fileContent = "Content of the nested file."u8.ToArray();

        const uint subDirInodeNum = 4u;  
        const uint fileInSubDirInodeNum = 5u;
        const uint fileInSubDirDataBlock = 115u;

        (var fs, var actualSubDirInodeNum) = TestImageBuilder.CreateSubDirectoryInRoot(subDirName);
        Assert.That(actualSubDirInodeNum, Is.EqualTo(subDirInodeNum), "Subdirectory inode number mismatch after creation.");

        var disk = fs.Disk;

        if (fs.Disk.Stream is MemoryStream memoryStream) 
            TestImageBuilder.AddFileToDirectory(disk, memoryStream, subDirInodeNum, fileNameInSubDir, fileContent, fileInSubDirInodeNum, fileInSubDirDataBlock);
        else 
            Assert.Fail("Could not retrieve MemoryStream from FileSystem's Disk for test setup.");

        // Act
        var resolvedInode = fs.ResolvePath(fullPath);
        var resolvedFileContent = fs.ReadFile(resolvedInode);

        // Assert
        Assert.Multiple(() =>
        {
            Assert.That(resolvedInode.Type, Is.EqualTo(InodeType.File), "Resolved item is not a file.");
            Assert.That(resolvedInode.SizeBytes, Is.EqualTo((uint)fileContent.Length), "Resolved file size mismatch.");
            Assert.That(resolvedFileContent, Is.EqualTo(fileContent), "Resolved file content mismatch.");
        });
    }
}