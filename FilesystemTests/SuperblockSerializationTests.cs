namespace FilesystemTests;

using Filesystem.Exceptions;
using Filesystem.Models;
using Filesystem.Serializers;
using NUnit.Framework;
using System;
using System.IO;
using System.Text;

public class SuperblockSerializationTests
{
    // Superblock field offsets (in bytes from the start of the superblock structure)
    private const int SignatureOffset = 56;            // s_magic (EXT2_SUPER_MAGIC)
    private const int FileSystemStateOffset = 58;    // s_state
    private const int ErrorHandlingOffset = 60;      // s_errors
    private const int CreatorOSOffset = 72;            // s_creator_os

    private MemoryStream _stream;
    private BinaryWriter _writer;
    private BinaryReader _reader;
    
    private Superblock CreateValidSuperblock() => new()
    {
        InodeCount = 1000,
        BlockCount = 5000,
        SuperuserReservedBlockCount = 100,
        UnallocatedBlockCount = 200,
        UnallocatedInodeCount = 300,
        StartingBlockNumber = 1,
        BlockSize = 4096,
        FragmentSize = 4096,
        BlocksPerGroup = 100,
        FragmentsPerGroup = 100,
        InodesPerGroup = 50,
        LastMountTime = DateTimeOffset.UtcNow,
        LastWrittenTime = DateTimeOffset.UtcNow,
        MountsSinceLastConsistencyCheck = 5,
        MountsBeforeNextConsistencyCheck = 20,
        FileSystemState = FileSystemState.Clean,
        ErrorHandling = ErrorHandling.Panic,
        MinorVersion = 1,
        LastCheckTime = DateTimeOffset.UtcNow,
        CheckInterval = TimeSpan.FromDays(1),
        OperatingSystemId = OperatingSystemID.Linux,
        MajorVersion = 1,
        ReservedUserId = 0,
        ReservedGroupId = 0,
    };

    [SetUp]
    public void Setup()
    {
        _stream = new MemoryStream();
        _writer = new BinaryWriter(_stream, Encoding.Default, true);
        _reader = new BinaryReader(_stream, Encoding.Default, true);
    }

    [TearDown]
    public void TearDown()
    {
        _writer.Dispose();
        _reader.Dispose();
        _stream.Dispose();
    }

    [Test]
    public void Read_ShouldThrow_WhenSignatureInvalid()
    {
        var superblock = CreateValidSuperblock();
        _writer.Write(superblock);

        _stream.Position = SignatureOffset;
        _stream.WriteByte(0x00); // Corrupt the signature
        _stream.WriteByte(0x00); // Corrupt the signature
        _stream.Position = 0;

        var ex = Assert.Throws<SuperblockFormatException>(() => _reader.ReadSuperblock());
        Assert.That(ex!.Message, Does.Contain("Invalid ext2 signature"));
    }

    [TestCase((ushort)0)]
    [TestCase((ushort)3)]
    public void Read_ShouldThrow_WhenFileSystemStateInvalid(ushort state)
    {
        var superblock = CreateValidSuperblock();

        _writer.Write(superblock);

        _stream.Position = FileSystemStateOffset;
        _stream.Write(BitConverter.GetBytes(state), 0, 2); // Set invalid file system state
        _stream.Position = 0;

        var ex = Assert.Throws<SuperblockFormatException>(() => _reader.ReadSuperblock());
        Assert.That(ex!.Message, Does.Contain("Invalid file system state"));
    }

    [TestCase((ushort)0)]
    [TestCase((ushort)4)]
    public void Read_ShouldThrow_WhenErrorHandlingInvalid(ushort value)
    {
        var superblock = CreateValidSuperblock();

        _writer.Write(superblock);

        _stream.Position = ErrorHandlingOffset;
        _stream.Write(BitConverter.GetBytes(value), 0, 2); // Set invalid error handling value
        _stream.Position = 0;

        var ex = Assert.Throws<SuperblockFormatException>(() => _reader.ReadSuperblock());
        Assert.That(ex!.Message, Does.Contain("Invalid error handling"));
    }

    [Test]
    public void Read_ShouldThrow_WhenOperatingSystemIdInvalid()
    {
        var superblock = CreateValidSuperblock();

        _writer.Write(superblock);

        _stream.Position = CreatorOSOffset;
        _stream.Write(BitConverter.GetBytes(5u), 0, 4); // Set invalid OS ID (5 is not defined in OperatingSystemID enum)
        _stream.Position = 0;

        var ex = Assert.Throws<SuperblockFormatException>(() => _reader.ReadSuperblock());
        Assert.That(ex!.Message, Does.Contain("Invalid operating system ID"));
    }

    [Test]
    public void Read_ShouldReturnSuperblock_WhenStreamIsValid()
    {
        var original = CreateValidSuperblock();

        _writer.Write(original);
        _stream.Position = 0;

        var result = _reader.ReadSuperblock();

        Assert.Multiple(() =>
        {
            Assert.That(result.InodeCount, Is.EqualTo(original.InodeCount));
            Assert.That(result.BlockCount, Is.EqualTo(original.BlockCount));
            Assert.That(result.SuperuserReservedBlockCount, Is.EqualTo(original.SuperuserReservedBlockCount));
            Assert.That(result.UnallocatedBlockCount, Is.EqualTo(original.UnallocatedBlockCount));
            Assert.That(result.UnallocatedInodeCount, Is.EqualTo(original.UnallocatedInodeCount));
            Assert.That(result.StartingBlockNumber, Is.EqualTo(original.StartingBlockNumber));
            Assert.That(result.BlockSize, Is.EqualTo(original.BlockSize));
            Assert.That(result.FragmentSize, Is.EqualTo(original.FragmentSize));
            Assert.That(result.BlocksPerGroup, Is.EqualTo(original.BlocksPerGroup));
            Assert.That(result.FragmentsPerGroup, Is.EqualTo(original.FragmentsPerGroup));
            Assert.That(result.InodesPerGroup, Is.EqualTo(original.InodesPerGroup));
            Assert.That(result.LastMountTime, Is.InRange(original.LastMountTime - TimeSpan.FromSeconds(1), original.LastMountTime + TimeSpan.FromSeconds(1)));
            Assert.That(result.LastWrittenTime, Is.InRange(original.LastWrittenTime - TimeSpan.FromSeconds(1), original.LastWrittenTime + TimeSpan.FromSeconds(1)));
            Assert.That(result.MountsSinceLastConsistencyCheck, Is.EqualTo(original.MountsSinceLastConsistencyCheck));
            Assert.That(result.MountsBeforeNextConsistencyCheck, Is.EqualTo(original.MountsBeforeNextConsistencyCheck));
            Assert.That(result.FileSystemState, Is.EqualTo(original.FileSystemState));
            Assert.That(result.ErrorHandling, Is.EqualTo(original.ErrorHandling));
            Assert.That(result.MinorVersion, Is.EqualTo(original.MinorVersion));
            Assert.That(result.LastCheckTime, Is.InRange(original.LastCheckTime - TimeSpan.FromSeconds(1), original.LastCheckTime + TimeSpan.FromSeconds(1)));
            Assert.That(result.CheckInterval, Is.EqualTo(original.CheckInterval));
            Assert.That(result.OperatingSystemId, Is.EqualTo(original.OperatingSystemId));
            Assert.That(result.MajorVersion, Is.EqualTo(original.MajorVersion));
            Assert.That(result.ReservedUserId, Is.EqualTo(original.ReservedUserId));
            Assert.That(result.ReservedGroupId, Is.EqualTo(original.ReservedGroupId));
        });
    }
}