#include "types.h"
#include "demo.h"
#include "keyboard.h"
#include "terminal.h"
#include "task.h"
#include "pit.h"

static void shell_task(void *arg)
{
    (void)arg;
    for (;;)
    {
        char c = keyboard_readchar();
        char buf[2] = {c, '\0'};
        terminal_write(buf);
    }
}

static void timer(void *arg)
{
    (void)arg;
    u64 time = 0;
    for (;;)
    {
        terminal_write("[Timer] ");

        char buf[21];
        int i;
        u64 n = time;
        if (n == 0)
        {
            buf[0] = '0';
            i = 1;
        }
        else
        {
            i = 0;
            while (n)
            {
                buf[i++] = '0' + (n % 10);
                n /= 10;
            }
        }
        for (int j = 0; j < i / 2; j++)
        {
            char tmp = buf[j];
            buf[j] = buf[i - 1 - j];
            buf[i - 1 - j] = tmp;
        }
        buf[i] = '\0';
        terminal_write(buf);

        terminal_write("s\n");
        time++;
        task_sleep_ms(1000);
    }
}

void demo(void)
{
    terminal_clear();
    terminal_write("Tenon v0.0.1\n");
    task_create(shell_task, null, "terminal");
    task_create(timer, null, "timer");
}