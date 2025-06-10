// See https://aka.ms/new-console-template for more information

using Filesystem;

using var fs = new FileStream("ext2.img", FileMode.Open, FileAccess.ReadWrite, FileShare.None);
if (!fs.CanRead || !fs.CanWrite)
{
    Console.WriteLine("Cannot read or write to the file.");
    return;
}

using var disk = new Disk(fs);
var fileSystem = new FileSystem(disk);

var directories = fileSystem.ListRootDirectory().ToList();

Console.WriteLine("Root Directory Entries:");
foreach (var entry in directories)
{
    Console.WriteLine($"- {entry}");
}