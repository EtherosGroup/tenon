#include "types.h"
#include "demo.h"
#include "line.h"
#include "terminal.h"
#include "task.h"
#include "pit.h"
#include "pmm.h"
#include "mufs.h"
#include "ramfs.h"

static bool strcmp(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return *a == *b;
}

static void u64_to_str(u64 n, char *buf)
{
    int i = 0;
    if (n == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    while (n) { buf[i++] = '0' + (n % 10); n /= 10; }
    for (int j = 0; j < i / 2; j++)
    {
        char t = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = t;
    }
    buf[i] = '\0';
}

static int my_strlen(const char *s)
{
    int len = 0;
    while (s[len]) len++;
    return len;
}

static void shell_task(void *arg)
{
    (void)arg;
    char buf[256];
    char num[21];
    for (;;)
    {
        terminal_write("> ");
        terminal_draw_cursor();
        line_read(buf, sizeof(buf));

        char *cmd = buf;
        while (*cmd == ' ') cmd++;

        if (*cmd == '\0') continue;

        char *arg_start = cmd;
        while (*arg_start && *arg_start != ' ') arg_start++;
        if (*arg_start == ' ') { *arg_start = '\0'; arg_start++; }
        while (*arg_start == ' ') arg_start++;

        if (strcmp(cmd, "help"))
        {
            terminal_write("  ls <dir>\n  cat <file>\n  touch <file>\n  mkdir <dir>\n  write <file> <text>\n  mounts\n  vars\n  clear\n  echo\n  mem\n  tasks\n");
        }
        else if (strcmp(cmd, "clear"))
        {
            terminal_clear();
        }
        else if (strcmp(cmd, "echo"))
        {
            terminal_write(arg_start);
            terminal_write("\n");
        }
        else if (strcmp(cmd, "mem"))
        {
            terminal_write("free: ");
            u64_to_str(pmm_free_pages() * 4, num);
            terminal_write(num);
            terminal_write(" KiB / ");
            u64_to_str(pmm_total_pages() * 4, num);
            terminal_write(num);
            terminal_write(" KiB\n");
        }
        else if (strcmp(cmd, "tasks"))
        {
            for (task_type *t = ready_queue; t; t = t->next)
            {
                terminal_write("  [");
                u64_to_str(t->id, num);
                terminal_write(num);
                terminal_write("] ");
                terminal_write(t->name);
                terminal_write(" ");
                switch (t->state)
                {
                case TASK_RUNNING: terminal_write("run"); break;
                case TASK_READY: terminal_write("ready"); break;
                case TASK_BLOCKED: terminal_write("blocked"); break;
                case TASK_DEAD: terminal_write("dead"); break;
                }
                terminal_write("\n");
            }
        }
        else if (strcmp(cmd, "ls"))
        {
            char *path = arg_start;
            if (!*path) path = "/system";
            int fd = mufs_open(path, VFS_O_RDONLY);
            if (fd < 0) { terminal_write("ls: not found\n"); continue; }
            char name[48];
            while (vfs_readdir(fd, name, sizeof(name)) > 0)
            {
                terminal_write("  ");
                terminal_write(name);
                terminal_write("\n");
            }
            vfs_close(fd);
        }
        else if (strcmp(cmd, "cat"))
        {
            int fd = mufs_open(arg_start, VFS_O_RDONLY);
            if (fd < 0) { terminal_write("cat: not found\n"); continue; }
            char rbuf[512];
            int n;
            while ((n = vfs_read(fd, rbuf, sizeof(rbuf) - 1)) > 0)
            {
                rbuf[n] = '\0';
                terminal_write(rbuf);
            }
            terminal_write("\n");
            vfs_close(fd);
        }
        else if (strcmp(cmd, "touch"))
        {
            int fd = mufs_open(arg_start, VFS_O_CREAT | VFS_O_RDWR);
            if (fd < 0)
            {
                terminal_write("touch: failed\n");
                continue;
            }
            vfs_close(fd);
        }
        else if (strcmp(cmd, "mkdir"))
        {
            if (vfs_mkdir(arg_start) != 0)
            {
                terminal_write("mkdir: failed\n");
            }
        }
        else if (strcmp(cmd, "mkfs"))
        {
            if (!*arg_start) { terminal_write("mkfs: need name\n"); continue; }
            vfs_sb_type *sb = ramfs_create_sb();
            if (!sb) { terminal_write("mkfs: failed\n"); continue; }
            mufs_mount(sb, arg_start, MUFST_DMP, 1);
            terminal_write("ramdisk /");
            terminal_write(arg_start);
            terminal_write(" created\n");
        }
        else if (strcmp(cmd, "write"))
        {
            char *path_end = arg_start;
            while (*path_end && *path_end != ' ') path_end++;
            char *content = null;
            if (*path_end == ' ') { *path_end = '\0'; content = path_end + 1; }
            while (content && *content == ' ') content++;
            if (!content || !*content) { terminal_write("write: need text\n"); continue; }
            int fd = mufs_open(arg_start, VFS_O_CREAT | VFS_O_RDWR);
            if (fd < 0) { terminal_write("write: failed\n"); continue; }
            int len = my_strlen(content);
            vfs_write(fd, content, len);
            vfs_close(fd);
        }
        else if (strcmp(cmd, "mounts"))
        {
            terminal_write("Mount points:\n");
            for (mufs_mount_t *m = mufs_get_mounts(); m; m = m->next)
            {
                terminal_write("  /");
                terminal_write(m->name);
                terminal_write("\n");
            }
        }
        else if (strcmp(cmd, "vars"))
        {
            terminal_write("Variables:\n");
            for (mufs_var_t *v = mufs_get_vars(); v; v = v->next)
            {
                terminal_write("  %");
                terminal_write(v->name);
                terminal_write("% -> ");
                terminal_write(v->target);
                terminal_write("\n");
            }
        }
        else
        {
            terminal_write("unknown command: ");
            terminal_write(cmd);
            terminal_write("\n");
        }
    }
}

void demo(void)
{
    terminal_clear();
    terminal_write("Tenon v0.0.1\n");
    task_create(shell_task, null, "terminal");
}
