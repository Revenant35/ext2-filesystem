using Filesystem;
using Filesystem.Services;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Filesystem.Mapping;

namespace FilesystemConsole;

public class Program
{
    public async static Task Main(string[] args)
    {
        var builder = Host.CreateApplicationBuilder(args);
        ConfigureServices(builder.Services);

        using var host = builder.Build();

        try
        {
            var fileSystem = host.Services.GetRequiredService<FileSystem>();
            var directories = fileSystem.ListRootDirectory().ToList();

            Console.WriteLine("Root Directory Entries:");
            foreach (var entry in directories)
            {
                Console.WriteLine($"- {entry}");
            }
        }
        catch (FileNotFoundException fnfEx)
        {
            await Console.Error.WriteLineAsync($"Error: {fnfEx.Message}");
            await Console.Error.WriteLineAsync("Please ensure the disk image file exists and the path is correct.");
        }
        catch (Exception ex)
        {
            await Console.Error.WriteLineAsync($"An unexpected error occurred: {ex.Message}");
            await Console.Error.WriteLineAsync(ex.StackTrace);
        }
    }

    private static void ConfigureServices(IServiceCollection services)
    {
        services.AddSingleton<Stream>(_ => GetDiskImageStream());

        services.AddAutoMapper(typeof(SuperblockProfile));
        services.AddAutoMapper(typeof(InodeProfile));

        services.AddSingleton<ISuperblockService, SuperblockService>();
        services.AddSingleton<IBlockGroupDescriptorService, BlockGroupDescriptorService>();
        services.AddSingleton<IInodeService, InodeService>();
        services.AddSingleton<Disk>();
        services.AddSingleton<FileSystem>();
    }
    
    private static FileStream GetDiskImageStream()
    {
        const string imagePath = "ext2.img";
        if (!File.Exists(imagePath))
        {
            throw new FileNotFoundException($"Disk image file not found at '{imagePath}'.", imagePath);
        }
        return new FileStream(imagePath, FileMode.Open, FileAccess.ReadWrite, FileShare.None);
    }
}