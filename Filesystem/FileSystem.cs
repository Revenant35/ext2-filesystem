namespace Filesystem;

public class FileSystem : IDisposable, IAsyncDisposable
{
    private readonly Disk _disk;

    public FileSystem(Disk disk)
    {
        _disk = disk;
    }
    
    public IEnumerable<string> ListRootDirectory() => 
        _disk.ReadRootDirectoryEntries()
            .Select(e => e.Name);



    #region IDisposable & IAsyncDisposable

    public void Dispose()
    {
        GC.SuppressFinalize(this);
        _disk.Dispose();
    }

    public async ValueTask DisposeAsync()
    {
        GC.SuppressFinalize(this);
        await _disk.DisposeAsync();
    }

    #endregion
}