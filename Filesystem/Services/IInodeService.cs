namespace Filesystem.Services;

using Models;

public interface IInodeService
{
    public Inode ReadInode(uint inodeIndex);
    public Inode ReadRootInode();
    public void WriteInode(Inode inode, uint index);
}