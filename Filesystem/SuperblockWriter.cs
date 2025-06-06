namespace Filesystem;

using Models;
using System.Text;

// https://wiki.osdev.org/Ext2#Superblock
public class SuperblockWriter(Stream stream, Encoding encoding, bool leaveOpen = false)
{
    public void Write(Superblock superblock)
    {
        using var writer = new BinaryWriter(stream, encoding, leaveOpen);

        writer.Write(superblock.InodeCount);
        writer.Write(superblock.BlockCount);
        writer.Write(superblock.SuperuserReservedBlockCount);
        writer.Write(superblock.UnallocatedBlockCount);
        writer.Write(superblock.UnallocatedInodeCount);
        writer.Write(superblock.StartingBlockNumber);
        writer.Write(int.Log2(superblock.BlockSize) - 10);
        writer.Write(int.Log2(superblock.FragmentSize) - 10);
        writer.Write(superblock.BlocksPerGroup);
        writer.Write(superblock.FragmentsPerGroup);
        writer.Write(superblock.InodesPerGroup);
        writer.Write(Convert.ToUInt32(superblock.LastMountTime.ToUnixTimeSeconds()));
        writer.Write(Convert.ToUInt32(superblock.LastWrittenTime.ToUnixTimeSeconds()));
        writer.Write(superblock.MountsSinceLastConsistencyCheck);
        writer.Write(superblock.MountsBeforeNextConsistencyCheck);
        writer.Write(Superblock.Ext2Signature);
        writer.Write((ushort)superblock.FileSystemState);
        writer.Write((ushort)superblock.ErrorHandling);
        writer.Write(superblock.MinorVersion);
        writer.Write(Convert.ToUInt32(superblock.LastCheckTime.ToUnixTimeSeconds()));
        writer.Write(Convert.ToUInt32(superblock.CheckInterval.TotalSeconds));
        writer.Write((uint)superblock.OperatingSystemId);
        writer.Write(superblock.MajorVersion);
        writer.Write(superblock.ReservedUserId);
        writer.Write(superblock.ReservedGroupId);
    }
}