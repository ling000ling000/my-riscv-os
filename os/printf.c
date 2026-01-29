#include "os.h"

void uart_puts(const char *s)
{
    while (*s) {
        sbi_console_putchar(*s++);
    }
}

static char msg_c1[] = "C1\n";
static char msg_panic[] = "PANIC\n";

int printf(const char* fmt, ...)
{
    (void)fmt;
    uart_puts(msg_c1);
    return 0;
}

void panic(const char *fmt, ...)
{
    (void)fmt;
    uart_puts(msg_panic);
    while (1) {}
}