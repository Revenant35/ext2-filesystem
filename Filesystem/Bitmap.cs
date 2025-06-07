namespace Filesystem;

using System.Numerics;

public class Bitmap
{
    private readonly byte[] _data;

    public int Capacity { get; }
    public uint UsedBits => (uint)_data.Sum(t => BitOperations.PopCount(t));
    public uint UnusedBits => (uint)(Capacity - UsedBits);

    public Bitmap(byte[] data, int capacity)
    {
        ArgumentOutOfRangeException.ThrowIfNegative(capacity);
        ArgumentOutOfRangeException.ThrowIfGreaterThanOrEqual(capacity, data.Length * 8);

        _data = data;
        Capacity = capacity;
    }

    public bool Test(int index)
    {
        ArgumentOutOfRangeException.ThrowIfNegative(index);
        ArgumentOutOfRangeException.ThrowIfGreaterThanOrEqual(index, Capacity);

        var byteIndex = index / 8;
        var bitOffset = index % 8;
        return (_data[byteIndex] & (1 << bitOffset)) != 0;
    }

    public void Set(int index)
    {
        ArgumentOutOfRangeException.ThrowIfNegative(index);
        ArgumentOutOfRangeException.ThrowIfGreaterThanOrEqual(index, Capacity);

        var byteIndex = index / 8;
        var bitOffset = index % 8;

        _data[byteIndex] |= (byte)(1 << bitOffset);
    }


    public void Reset(int index)
    {
        ArgumentOutOfRangeException.ThrowIfNegative(index);
        ArgumentOutOfRangeException.ThrowIfGreaterThanOrEqual(index, Capacity);

        var byteIndex = index / 8;
        var bitOffset = index % 8;

        _data[byteIndex] &= (byte)~(1 << bitOffset);
    }

    public void Toggle(int index)
    {
        ArgumentOutOfRangeException.ThrowIfNegative(index);
        ArgumentOutOfRangeException.ThrowIfGreaterThanOrEqual(index, Capacity);

        var byteIndex = index / 8;
        var bitOffset = index % 8;

        _data[byteIndex] ^= (byte)(1 << bitOffset);
    }

    public IEnumerable<int> UnsetBits()
    {
        for (var i = 0; i < Capacity; i++)
            if (!Test(i))
                yield return i;
    }

    public IEnumerable<int> SetBits()
    {
        for (var i = 0; i < Capacity; i++)
            if (Test(i))
                yield return i;
    }

    public byte[] ToByteArray() => _data.ToArray();
}