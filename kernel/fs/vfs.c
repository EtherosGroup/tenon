#include "vfs.h"
#include "serial.h"
#include "kheap.h"

vfs_file_type *vfs_fd_table[VFS_MAX_FD];

// 挂载注册表
#define VFS_MAX_MOUNTS 16 // 最大挂载数

typedef struct
{
    char name[VFS_NAME_MAX];
    vfs_sb_type *sb;
} vfs_mount_entry_t;

static vfs_mount_entry_t mounts[VFS_MAX_MOUNTS];
static int mount_count;

int vfs_mount_reg(const char *name, vfs_sb_type *sb)
{
    if (mount_count >= VFS_MAX_MOUNTS) return -1;

    int i = 0;
    while (name[i] && i < VFS_NAME_MAX - 1)
    {
        mounts[mount_count].name[i] = name[i];
        i++;
    }
    mounts[mount_count].name[i] = '\0';
    mounts[mount_count].sb = sb;
    mount_count++;
    return 0;
}

vfs_sb_type *vfs_mount_lookup(const char *name)
{
    for (int i = 0; i < mount_count; i++)
    {
        int j = 0;
        while (mounts[i].name[j] && name[j] && mounts[i].name[j] == name[j]) j++;
        if (!mounts[i].name[j] && (!name[j] || name[j] == '/')) return mounts[i].sb;
    }
    return null;
}

// 分配一个空闲的文件描述符，成功返回 fd 编号，失败返回 -1
static int vfs_fd_alloc(vfs_file_type *file)
{
    for (int i = 0; i < VFS_MAX_FD; i++)
    {
        if (!vfs_fd_table[i])
        {
            vfs_fd_table[i] = file;
            return i;
        }
    }
    return -1;
}

/* ── path helpers ───────────────────────────────── */
static const char *vfs_skip_slash(const char *path)
{
    while (*path == '/') path++;
    return path;
}

/* ── dentry walking ─────────────────────────────── */
static vfs_inode_type *vfs_walk(vfs_inode_type *dir, const char *path)
{
    if (!path || !*path) return dir;
    if (dir->type != VFS_TYPE_DIR) return null;

    const char *p = vfs_skip_slash(path);
    if (!*p) return dir;

    /* extract next component */
    char comp[VFS_NAME_MAX];
    int ci = 0;
    while (*p && *p != '/' && ci < VFS_NAME_MAX - 1) comp[ci++] = *p++;
    comp[ci] = '\0';

    if (ci == 0) return dir;

    vfs_inode_type *next = dir->ops->lookup(dir, comp);
    if (!next) return null;

    /* recurse */
    if (*p == '/') return vfs_walk(next, p);
    return next;
}

/* ── root inode ─────────────────────────────────── */
static int vfs_root_readdir(vfs_inode_type *inode, void *buf, u64 offset, u64 size)
{
    (void)inode;
    if ((int)offset >= mount_count) return 0;
    char *name = mounts[(int)offset].name;
    int i = 0;
    while (name[i] && i < (int)size - 1) { ((char *)buf)[i] = name[i]; i++; }
    ((char *)buf)[i] = '\0';
    return i;
}

static const vfs_inode_ops_t vfs_root_ops = {
    .read = vfs_root_readdir,
};

static vfs_inode_type vfs_root_inode;

/* ── full path resolve ──────────────────────────── */
static vfs_inode_type *vfs_resolve(const char *path)
{
    if (!path || !*path) return null;

    const char *p = vfs_skip_slash(path);
    if (!*p) return &vfs_root_inode;

    /* extract mount name */
    char mount[VFS_NAME_MAX];
    int mi = 0;
    while (*p && *p != '/' && mi < VFS_NAME_MAX - 1) mount[mi++] = *p++;
    mount[mi] = '\0';

    vfs_sb_type *sb = vfs_mount_lookup(mount);
    if (!sb) return null;

    vfs_inode_type *dir = sb->root->inode;
    if (!dir) return null;

    /* walk remaining path */
    if (*p == '/')
    {
        return vfs_walk(dir, p);
    }
    return dir;
}

/* ── public API ─────────────────────────────────── */

void vfs_init(void)
{
    for (int i = 0; i < VFS_MAX_FD; i++) vfs_fd_table[i] = null;
    mount_count = 0;

    vfs_root_inode.ino  = 0;
    vfs_root_inode.type = VFS_TYPE_DIR;
    vfs_root_inode.size = 0;
    vfs_root_inode.sb   = null;
    vfs_root_inode.ops  = (vfs_inode_ops_t *)&vfs_root_ops;
    vfs_root_inode.fs_data = null;
}

int vfs_open(const char *path, u32 flags)
{
    vfs_inode_type *inode = vfs_resolve(path);

    /* VFS_O_CREAT: file doesn't exist → create it */
    if (!inode && (flags & VFS_O_CREAT))
    {
        /* resolve parent */
        char parent_path[VFS_PATH_MAX];
        char file_name[VFS_NAME_MAX];
        int i = 0, last_slash = -1;

        while (path[i])
        {
            if (path[i] == '/') last_slash = i;
            i++;
        }

        int pi = 0;
        for (int j = 0; j < last_slash && pi < VFS_PATH_MAX - 1; j++) parent_path[pi++] = path[j];
        parent_path[pi] = '\0';

        int fi = 0;
        for (int j = last_slash + 1; path[j] && fi < VFS_NAME_MAX - 1; j++) file_name[fi++] = path[j];
        file_name[fi] = '\0';

        if (fi == 0) return -1;

        vfs_inode_type *parent = vfs_resolve(parent_path);
        if (!parent) return -1;

        parent->ops->create(parent, file_name, VFS_TYPE_FILE);
        inode = vfs_resolve(path);
        if (!inode) return -1;
    }

    if (!inode) return -1;

    vfs_file_type *file = (vfs_file_type *)kmalloc(sizeof(vfs_file_type));
    if (!file) return -1;

    file->inode  = inode;
    file->offset = 0;
    file->flags  = flags;

    int fd = vfs_fd_alloc(file);
    if (fd < 0) { kfree(file); return -1; }
    return fd;
}

int vfs_close(int fd)
{
    if (fd < 0 || fd >= VFS_MAX_FD || !vfs_fd_table[fd]) return -1;
    kfree(vfs_fd_table[fd]);
    vfs_fd_table[fd] = null;
    return 0;
}

int vfs_read(int fd, void *buf, u64 size)
{
    if (fd < 0 || fd >= VFS_MAX_FD || !vfs_fd_table[fd]) return -1;
    vfs_file_type *file = vfs_fd_table[fd];
    vfs_inode_type *inode = file->inode;
    if (!inode->ops->read) return -1;

    int ret = inode->ops->read(inode, buf, file->offset, size);
    if (ret > 0) file->offset += ret;
    return ret;
}

int vfs_write(int fd, const void *buf, u64 size)
{
    if (fd < 0 || fd >= VFS_MAX_FD || !vfs_fd_table[fd]) return -1;
    vfs_file_type *file = vfs_fd_table[fd];
    vfs_inode_type *inode = file->inode;
    if (!inode->ops->write) return -1;

    int ret = inode->ops->write(inode, buf, file->offset, size);
    if (ret > 0) file->offset += ret;
    return ret;
}

int vfs_mkdir(const char *path)
{
    char parent_path[VFS_PATH_MAX];
    char dir_name[VFS_NAME_MAX];
    int i = 0, last_slash = -1;

    while (path[i])
    {
        if (path[i] == '/') last_slash = i;
        i++;
    }

    int pi = 0;
    for (int j = 0; j < last_slash && pi < VFS_PATH_MAX - 1; j++) parent_path[pi++] = path[j];
    parent_path[pi] = '\0';

    int di = 0;
    for (int j = last_slash + 1; path[j] && di < VFS_NAME_MAX - 1; j++) dir_name[di++] = path[j];
    dir_name[di] = '\0';

    if (di == 0) return -1;

    vfs_inode_type *parent = vfs_resolve(parent_path);
    if (!parent) return -1;

    return parent->ops->create(parent, dir_name, VFS_TYPE_DIR);
}

int vfs_readdir(int fd, char *name_out, u64 maxlen)
{
    if (fd < 0 || fd >= VFS_MAX_FD || !vfs_fd_table[fd]) return -1;
    vfs_file_type *file = vfs_fd_table[fd];
    vfs_inode_type *inode = file->inode;

    if (inode->type != VFS_TYPE_DIR) return -1;
    if (!inode->ops->read) return -1;

    /* use read with file->offset as dirent index */
    int ret = inode->ops->read(inode, name_out, file->offset, maxlen);
    if (ret > 0) file->offset++;
    return ret;
}
