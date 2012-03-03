#include <stdarg.h>
#include <errno.h>

#include "error.h"
#include "shell.h"

static void    err_doit(int, const char *, va_list);

void err_sys(const char *fmt, ...)
{
	va_list        ap;

    va_start(ap, fmt);
    err_doit(1, fmt, ap);
    va_end(ap);
    exit(1);
}

void err_ret(const char *fmt, ...)
{
	va_list        ap;

    va_start(ap, fmt);
    err_doit(1, fmt, ap);
    va_end(ap);
    return;
}


static void err_doit(int errnoflag, const char *fmt, va_list ap)
{
    int     errno_save;
    char    buf[BUFMAX];

    errno_save = errno;        /* value caller might want printed */
    vsprintf(buf, fmt, ap);
    if (errnoflag)
        sprintf(buf+strlen(buf), ": %s", strerror(errno_save));
    strcat(buf, "\n");
    fflush(stdout);        /* in case stdout and stderr are the same */
    fputs(buf, stderr);
    fflush(stderr);        /* SunOS 4.1.* doesn't grok NULL argument */
    return;
}
