#include "ramfs.h"
#include "kheap.h"
#include "serial.h"

typedef struct
{
    u64 capacity;
    u64 data_size;
    u8 *data;
    int entry_count;
} ramfs_inode_t;

typedef struct
{
    char name[48];
    u64 ino;
} ramfs_dirent_t;

#define RAMFS_MAX_INODES 256

static vfs_inode_type *inode_table[RAMFS_MAX_INODES];
static u64 next_ino = 1;

static ramfs_inode_t *ramfs_get(vfs_inode_type *vi)
{
    return (ramfs_inode_t *)vi->fs_data;
}

static vfs_inode_type *ramfs_alloc_inode(u32 type)
{
    if (next_ino >= RAMFS_MAX_INODES) return null;

    vfs_inode_type *vi = (vfs_inode_type *)kmalloc(sizeof(vfs_inode_type));
    if (!vi) return null;

    ramfs_inode_t *ri = (ramfs_inode_t *)kmalloc(sizeof(ramfs_inode_t));
    if (!ri) { kfree(vi); return null; }

    ri->capacity = 0;
    ri->data_size = 0;
    ri->data = null;
    ri->entry_count = 0;

    vi->ino = next_ino++;
    vi->type = type;
    vi->size = 0;
    vi->sb = null;
    vi->ops = null;
    vi->fs_data = ri;

    inode_table[vi->ino] = vi;
    return vi;
}

// ram fs操作
static int ramfs_read(vfs_inode_type *inode, void *buf, u64 offset, u64 size)
{
    ramfs_inode_t *ri = ramfs_get(inode);

    if (inode->type == VFS_TYPE_DIR)
    {
        int idx = (int)offset;
        if (idx < 0 || idx >= ri->entry_count) return 0;

        ramfs_dirent_t *entries = (ramfs_dirent_t *)ri->data;
        char *name = entries[idx].name;

        int i = 0;
        while (name[i] && i < (int)size - 1) { ((char *)buf)[i] = name[i]; i++; }
        ((char *)buf)[i] = '\0';
        return i;
    }

    if (offset >= ri->data_size) return 0;
    u64 avail = ri->data_size - offset;
    u64 n = size < avail ? size : avail;
    for (u64 i = 0; i < n; i++) ((u8 *)buf)[i] = ri->data[offset + i];
    return (int)n;
}

static int ramfs_write(vfs_inode_type *inode, const void *buf, u64 offset, u64 size)
{
    ramfs_inode_t *ri = ramfs_get(inode);
    u64 needed = offset + size;

    if (needed > ri->capacity)
    {
        u64 new_cap = ri->capacity ? ri->capacity : 64;
        while (new_cap < needed) new_cap *= 2;
        u8 *new_data = (u8 *)krealloc(ri->data, new_cap);
        if (!new_data) return -1;
        for (u64 i = ri->capacity; i < new_cap; i++) new_data[i] = 0;
        ri->data = new_data;
        ri->capacity = new_cap;
    }

    for (u64 i = 0; i < size; i++) ri->data[offset + i] = ((const u8 *)buf)[i];
    if (needed > ri->data_size) ri->data_size = needed;
    inode->size = ri->data_size;
    return (int)size;
}

/* ── directory operations ───────────────────────── */

static vfs_inode_type *ramfs_lookup(vfs_inode_type *dir, const char *name)
{
    ramfs_inode_t *ri = ramfs_get(dir);
    ramfs_dirent_t *entries = (ramfs_dirent_t *)ri->data;

    for (int i = 0; i < ri->entry_count; i++)
    {
        int j = 0;
        while (entries[i].name[j] && name[j] && entries[i].name[j] == name[j]) j++;
        if (!entries[i].name[j] && !name[j]) return inode_table[entries[i].ino];
    }
    return null;
}

static int ramfs_create(vfs_inode_type *dir, const char *name, u32 type)
{
    ramfs_inode_t *ri = ramfs_get(dir);

    if (ramfs_lookup(dir, name)) return -1;

    vfs_inode_type *child = ramfs_alloc_inode(type);
    if (!child) return -1;

    child->sb = dir->sb;
    child->ops = dir->ops;

    // 追加目录
    int idx = ri->entry_count;
    u64 new_size = (idx + 1) * sizeof(ramfs_dirent_t);
    u8 *new_data = (u8 *)krealloc(ri->data, new_size);
    if (!new_data)
    {
        kfree(ramfs_get(child));
        kfree(child); return -1; 
    }
    ri->data = new_data;
    ri->capacity = new_size;
    ri->data_size = new_size;

    ramfs_dirent_t *entries = (ramfs_dirent_t *)ri->data;
    int j = 0;
    while (name[j] && j < 47) { entries[idx].name[j] = name[j]; j++; }
    entries[idx].name[j] = '\0';
    entries[idx].ino  = child->ino;
    ri->entry_count++;

    return 0;
}

static int ramfs_unlink(vfs_inode_type *dir, const char *name)
{
    ramfs_inode_t *ri = ramfs_get(dir);
    ramfs_dirent_t *entries = (ramfs_dirent_t *)ri->data;
    int target = -1;

    for (int i = 0; i < ri->entry_count; i++)
    {
        int j = 0;
        while (entries[i].name[j] && name[j] && entries[i].name[j] == name[j]) j++;
        if (!entries[i].name[j] && !name[j]) { target = i; break; }
    }
    if (target < 0) return -1;

    u64 ino = entries[target].ino;
    vfs_inode_type *vi = inode_table[ino];
    if (vi)
    {
        ramfs_inode_t *ri_child = ramfs_get(vi);
        kfree(ri_child->data);
        kfree(ri_child);
        kfree(vi);
        inode_table[ino] = null;
    }

    // 移动剩余条目
    for (int i = target; i < ri->entry_count - 1; i++)
    {
        entries[i] = entries[i + 1];
    }
    ri->entry_count--;

    return 0;
}

static const vfs_inode_ops_t ramfs_ops = {
    .read = ramfs_read,
    .write = ramfs_write,
    .lookup = ramfs_lookup,
    .create = ramfs_create,
    .unlink = ramfs_unlink,
};

// 超级块

vfs_sb_type *ramfs_create_sb(void)
{
    vfs_sb_type *sb = (vfs_sb_type *)kmalloc(sizeof(vfs_sb_type));
    if (!sb) return null;

    vfs_inode_type *root_inode = ramfs_alloc_inode(VFS_TYPE_DIR);
    if (!root_inode) { kfree(sb); return null; }
    root_inode->sb = sb;
    root_inode->ops = (vfs_inode_ops_t *)&ramfs_ops;

    vfs_dentry_type *root_dentry = (vfs_dentry_type *)kmalloc(sizeof(vfs_dentry_type));
    if (!root_dentry) { kfree(root_inode); kfree(sb); return null; }

    int i = 0;
    while ("/" [i] && i < VFS_NAME_MAX - 1) { root_dentry->name[i] = "/" [i]; i++; }
    root_dentry->name[i] = '\0';
    root_dentry->inode = root_inode;
    root_dentry->parent = null;
    root_dentry->child = null;
    root_dentry->next = null;

    sb->root = root_dentry;
    sb->fs_data = null;

    return sb;
}
