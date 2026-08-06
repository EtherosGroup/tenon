#include "mufs.h"
#include "kheap.h"
#include "serial.h"

static mufs_mount_t *mount_head;
static mufs_var_t *var_head;

void mufs_init(void)
{
    mount_head = null;
    var_head = null;
}

int mufs_mount(vfs_sb_type *sb, const char *name, u8 type, u8 auto_mount)
{
    mufs_mount_t *m = (mufs_mount_t *)kmalloc(sizeof(mufs_mount_t));
    if (!m) return -1;

    int i = 0;
    while (name[i] && i < 47) { m->name[i] = name[i]; i++; }
    m->name[i] = '\0';
    m->type = type;
    m->flags = 0;
    m->auto_mount = auto_mount;
    m->sb = sb;
    m->next = mount_head;
    mount_head = m;

    vfs_mount_reg(name, sb);

    serial_print("[MUFS] mount /");
    serial_print(name);
    serial_print(" type=");
    serial_print_hex(type);
    serial_println("");

    return 0;
}

int mufs_set_var(const char *name, const char *target)
{
    mufs_var_t *v = (mufs_var_t *)kmalloc(sizeof(mufs_var_t));
    if (!v) return -1;

    int i = 0;
    while (name[i] && i < 15) { v->name[i] = name[i]; i++; }
    v->name[i] = '\0';

    i = 0;
    while (target[i] && i < 255) { v->target[i] = target[i]; i++; }
    v->target[i] = '\0';

    v->next = var_head;
    var_head = v;
    return 0;
}

int mufs_open(const char *raw_path, u32 flags)
{
    char resolved[VFS_PATH_MAX];
    int ri = 0;

    // 变量解析 %var%
    const char *p = raw_path;
    while (*p && ri < VFS_PATH_MAX - 1)
    {
        if (*p == '%')
        {
            p++;
            char var_name[16];
            int vi = 0;
            while (*p && *p != '%' && vi < 15) var_name[vi++] = *p++;
            var_name[vi] = '\0';
            if (*p == '%') p++;

            // 查找变量
            bool found = false;
            for (mufs_var_t *v = var_head; v; v = v->next)
            {
                int j = 0;
                while (v->name[j] && var_name[j] && v->name[j] == var_name[j]) j++;
                if (!v->name[j] && !var_name[j])
                {
                    for (int k = 0; v->target[k] && ri < VFS_PATH_MAX - 1; k++)
                    {
                        resolved[ri++] = v->target[k];
                    }
                    found = true;
                    break;
                }
            }
            if (!found) return -1;
        }
        else
        {
            resolved[ri++] = *p++;
        }
    }
    resolved[ri] = '\0';

    return vfs_open(resolved, flags);
}

void mufs_list_mounts(void)
{
    serial_println("[MUFS] Mount points:");
    for (mufs_mount_t *m = mount_head; m; m = m->next)
    {
        serial_print("  /");
        serial_print(m->name);
        serial_print(" type=");
        serial_print_hex(m->type);
        serial_println("");
    }
}

void mufs_list_vars(void)
{
    serial_println("[MUFS] Variables:");
    for (mufs_var_t *v = var_head; v; v = v->next)
    {
        serial_print("  %");
        serial_print(v->name);
        serial_print("% -> ");
        serial_println(v->target);
    }
}

mufs_mount_t *mufs_get_mounts(void) { return mount_head; }
mufs_var_t *mufs_get_vars(void) { return var_head; }
