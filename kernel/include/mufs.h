#ifndef KERNEL_MUFS_H
#define KERNEL_MUFS_H

#include "types.h"
#include "vfs.h"

#define MUFS_MAGIC          0x4D554653
#define MUFS_VERSION        0x0001
#define MUFS_EXT_OFFSET     0x200
#define MUFS_HEAD_SIZE      56
#define MUFS_BODY_SIZE      104

#define MUFST_DMP  0x01
#define MUFST_SMP  0x02
#define MUFST_VMP  0x03

#define MUFS_FLAG_READONLY        0x0001
#define MUFS_FLAG_HIDDEN          0x0002
#define MUFS_FLAG_NO_UNMOUNT      0x0004
#define MUFS_FLAG_SYSTEM_CRITICAL 0x0008

typedef struct __attribute__((packed))
{
    u32 mu_magic;
    u16 mu_version;
    u16 mu_flags;
    u8  mu_name[48];
    u8  mu_type;
    u8  mu_perm;
    u8  mu_auto_mount;
    u8  mu_var_tag[16];
    u8  mu_var_target[16];
    u32 mu_checksum;
    u8  mu_reserved[65];
} mufs_ext_t;

typedef struct mufs_mount
{
    char name[48];
    u8   type;
    u16  flags;
    u8   auto_mount;
    vfs_sb_type *sb;
    struct mufs_mount *next;
} mufs_mount_t;

typedef struct mufs_var
{
    char name[16];
    char target[256];
    struct mufs_var *next;
} mufs_var_t;

void mufs_init(void);

int  mufs_mount(vfs_sb_type *sb, const char *name, u8 type, u8 auto_mount);
int  mufs_set_var(const char *name, const char *path);
int  mufs_open(const char *raw_path, u32 flags);

void mufs_list_mounts(void);
void mufs_list_vars(void);

mufs_mount_t *mufs_get_mounts(void);
mufs_var_t   *mufs_get_vars(void);

#endif
