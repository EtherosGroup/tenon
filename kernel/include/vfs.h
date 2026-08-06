#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

#include "types.h"

#define VFS_TYPE_FILE 1
#define VFS_TYPE_DIR 2

#define VFS_O_RDONLY 1
#define VFS_O_WRONLY 2
#define VFS_O_RDWR 3
#define VFS_O_CREAT 4

#define VFS_NAME_MAX 48
#define VFS_PATH_MAX 256

typedef struct vfs_inode vfs_inode_type;
typedef struct vfs_dentry vfs_dentry_type;
typedef struct vfs_file vfs_file_type;
typedef struct vfs_sb vfs_sb_type;

typedef struct
{
    int (*read)(vfs_inode_type *inode, void *buf, u64 offset, u64 size);
    int (*write)(vfs_inode_type *inode, const void *buf, u64 offset, u64 size);
    vfs_inode_type *(*lookup)(vfs_inode_type *dir, const char *name);
    int (*create)(vfs_inode_type *dir, const char *name, u32 type);
    int (*unlink)(vfs_inode_type *dir, const char *name);
} vfs_inode_ops_t;

struct vfs_inode
{
    u64 ino;
    u32 type;
    u64 size;
    vfs_sb_type *sb;
    vfs_inode_ops_t *ops;
    void *fs_data;
};

struct vfs_dentry
{
    char name[VFS_NAME_MAX];
    vfs_inode_type *inode;
    vfs_dentry_type *parent;
    vfs_dentry_type *child;
    vfs_dentry_type *next;
};

struct vfs_file
{
    vfs_inode_type *inode;
    u64 offset;
    u32 flags;
};

struct vfs_sb
{
    vfs_dentry_type *root;
    void *fs_data;
};

#define VFS_MAX_FD 64

extern vfs_file_type *vfs_fd_table[VFS_MAX_FD];

void vfs_init(void);

int vfs_open(const char *path, u32 flags);
int vfs_close(int fd);
int vfs_read(int fd, void *buf, u64 size);
int vfs_write(int fd, const void *buf, u64 size);
int vfs_mkdir(const char *path);
int vfs_readdir(int fd, char *name_out, u64 maxlen);

int vfs_mount_reg(const char *name, vfs_sb_type *sb);
vfs_sb_type *vfs_mount_lookup(const char *name);

#endif
