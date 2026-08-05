#include "types.h"
#include "demo.h"
#include "line.h"
#include "terminal.h"
#include "task.h"
#include "pit.h"
#include "pmm.h"

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
            terminal_write("clear  echo <text>  help  mem  tasks\n");
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
                case TASK_READY:   terminal_write("ready"); break;
                case TASK_BLOCKED: terminal_write("blocked"); break;
                case TASK_DEAD:    terminal_write("dead"); break;
                }
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
