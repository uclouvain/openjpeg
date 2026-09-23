/*
 * Regression tests for the JPIP query parser.
 *
 * The query string comes straight from getenv("QUERY_STRING") on the FastCGI
 * side and is therefore fully attacker controlled.  get_fieldparam() used to
 * copy the field name and value into fixed stack buffers with lengths derived
 * from the '=' and '&' delimiters, so an oversized field overflowed the
 * stack.  These tests cover the oversized cases plus a few well-formed ones.
 */

#include "query_parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Big enough to hold the largest oversized field used below. */
#define QBUFSZ 512

static char qbuf[QBUFSZ];
static int failures = 0;

#define CHECK(cond, msg)                                        \
    do {                                                        \
        if (!(cond)) {                                          \
            fprintf(stderr, "FAIL: %s\n", msg);                 \
            failures++;                                         \
        } else {                                                \
            printf("ok: %s\n", msg);                            \
        }                                                       \
    } while (0)

/* Build "<name>=<val>" in qbuf and return it.  Caller frees nothing. */
static const char *make_query(const char *name, size_t namelen,
                              const char *val, size_t vallen)
{
    size_t n = namelen + 1 + vallen;
    if (n + 1 > QBUFSZ) {
        fprintf(stderr, "test buffer too small\n");
        exit(1);
    }
    memcpy(qbuf, name, namelen);
    qbuf[namelen] = '=';
    memcpy(qbuf + namelen + 1, val, vallen);
    qbuf[n] = '\0';
    return qbuf;
}

int main(void)
{
    query_param_t *q;
    char big[256];
    size_t i;

    for (i = 0; i < sizeof(big); i++) {
        big[i] = 'A';
    }

    /* Value longer than MAX_LENOFFIELDVAL (128): this used to write 137
     * bytes into fieldval[128] on the stack. */
    q = parse_query(make_query("target", 6, big, 137));
    CHECK(q != NULL, "oversized field value does not crash");
    CHECK(q == NULL || q->target == NULL, "oversized field value is dropped");
    if (q) {
        delete_query(&q);
    }

    /* Name longer than MAX_LENOFFIELDNAME (10): this used to write 20 bytes
     * into fieldname[10] on the stack. */
    q = parse_query(make_query(big, 20, "1", 1));
    CHECK(q != NULL, "oversized field name does not crash");
    if (q) {
        delete_query(&q);
    }

    /* '&' before '=': the old code computed a negative length and relied on
     * an assert() that NDEBUG removes. */
    q = parse_query("x&y=1");
    CHECK(q != NULL, "ampersand before equals does not crash");
    if (q) {
        delete_query(&q);
    }

    /* A well-formed query must still parse exactly as before. */
    q = parse_query("target=image.jp2&fsiz=170,170&layers=3&rsiz=64,64");
    CHECK(q != NULL, "well-formed query parses");
    if (q) {
        CHECK(q->target != NULL && strcmp(q->target, "image.jp2") == 0,
              "target is extracted");
        CHECK(q->fx == 170 && q->fy == 170, "frame size is extracted");
        CHECK(q->rw == 64 && q->rh == 64, "roi size is extracted");
        delete_query(&q);
    }

    /* Field names are case-insensitive. */
    q = parse_query("TARGET=abc");
    CHECK(q != NULL && q->target != NULL && strcmp(q->target, "abc") == 0,
          "field name is case-insensitive");
    if (q) {
        delete_query(&q);
    }

    /* A value of exactly MAX_LENOFFIELDVAL - 1 bytes must still be accepted. */
    q = parse_query(make_query("target", 6, big, 127));
    CHECK(q != NULL && q->target != NULL && strlen(q->target) == 127,
          "127-byte field value is accepted");
    if (q) {
        delete_query(&q);
    }

    if (failures) {
        fprintf(stderr, "%d test(s) failed\n", failures);
        return 1;
    }
    printf("all JPIP query parser tests passed\n");
    return 0;
}
