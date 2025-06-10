namespace Filesystem;

public class FileSystem
{
    private readonly Disk _disk;

    public FileSystem(Disk disk)
    {
        _disk = disk;
    }
    
    public IEnumerable<string> ListRootDirectory() => 
        _disk.ReadRootDirectoryEntries()
            .Select(e => e.Name);

}