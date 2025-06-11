// See https://aka.ms/new-console-template for more information

using Filesystem;
using Filesystem.Services;

using var fs = new FileStream("ext2.img", FileMode.Open, FileAccess.ReadWrite, FileShare.None);
if (!fs.CanRead || !fs.CanWrite)
{
    Console.WriteLine("Cannot read or write to the file.");
    return;
}

var superblockService = new SuperblockService(fs);
var blockGroupDescriptorService = new BlockGroupDescriptorService(fs, superblockService);
using var disk = new Disk(fs, superblockService, blockGroupDescriptorService);
var fileSystem = new FileSystem(disk);

var directories = fileSystem.ListRootDirectory().ToList();

Console.WriteLine("Root Directory Entries:");
foreach (var entry in directories)
{
    Console.WriteLine($"- {entry}");
}