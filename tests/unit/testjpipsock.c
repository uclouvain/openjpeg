/*
 * Regression test for receive_line() (src/lib/openjpip/sock_manager.c):
 * it must never write past the caller's buffer, and a line that does not
 * fit must be reported as an error.
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "openjpeg.h"
#include "sock_manager.h"

#define CAP 16
#define GUARD 0x5A

typedef struct {
    char buf[CAP];
    char guard[64];
} guarded_t;

static int check(const char *line, size_t linelen, OPJ_SIZE_T expected_len)
{
    int sv[2];
    guarded_t g;
    OPJ_SIZE_T ret;
    size_t i;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0) {
        perror("socketpair");
        return 1;
    }
    memset(&g, GUARD, sizeof(g));
    if (write(sv[1], line, linelen) != (ssize_t)linelen) {
        perror("write");
        return 1;
    }
    close(sv[1]);   /* EOF after the data, so a read past the line cannot hang */

    ret = receive_line(sv[0], g.buf, sizeof(g.buf));
    close(sv[0]);

    for (i = 0; i < sizeof(g.guard); i++) {
        if (g.guard[i] != GUARD) {
            fprintf(stderr, "linelen %zu: wrote past buffer (guard byte %zu)\n",
                    linelen, i);
            return 1;
        }
    }
    if (ret != expected_len) {
        fprintf(stderr, "linelen %zu: returned %lu, expected %lu\n", linelen,
                (unsigned long)ret, (unsigned long)expected_len);
        return 1;
    }
    if (expected_len == 0 && g.buf[0] != '\0') {
        fprintf(stderr, "linelen %zu: buffer not cleared on error\n", linelen);
        return 1;
    }
    if (expected_len != 0 && (strlen(g.buf) != expected_len || g.buf[expected_len] != '\0')) {
        fprintf(stderr, "linelen %zu: bad string\n", linelen);
        return 1;
    }
    return 0;
}

int main(void)
{
    char big[1024];
    int failed = 0;

    memset(big, 'A', sizeof(big));

    failed |= check("abc\n", 4, 3);
    /* longest line that fits: CAP - 1 characters plus the NUL */
    big[CAP - 1] = '\n';
    failed |= check(big, CAP, CAP - 1);
    /* one character too many */
    big[CAP - 1] = 'A';
    big[CAP] = '\n';
    failed |= check(big, CAP + 1, 0);
    big[CAP] = 'A';
    /* the case from the issue: far longer than the buffer */
    big[400] = '\n';
    failed |= check(big, 401, 0);
    failed |= check(big, 400, 0);   /* no newline, then EOF */

    return failed ? 1 : 0;
}
