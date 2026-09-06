/*
 * Copyright (c) 2026, OpenJPEG contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "query_parser.h"

static void expect_default_query(query_param_t *query)
{
    if (query == NULL) {
        fprintf(stderr, "parse_query returned NULL\n");
        exit(1);
    }

    if (query->target != NULL || query->box_type[0][0] != '\0') {
        fprintf(stderr, "oversized field unexpectedly populated query state\n");
        delete_query(&query);
        exit(1);
    }

    delete_query(&query);
}

static void test_rejects_oversized_field_name(void)
{
    char query_string[256];

    memset(query_string, 'A', sizeof(query_string));
    query_string[200] = '=';
    query_string[201] = '1';
    query_string[202] = '\0';

    expect_default_query(parse_query(query_string));
}

static void test_rejects_oversized_field_value(void)
{
    char query_string[256];
    char field_value[200];

    memset(field_value, 'A', 180);
    field_value[180] = '\0';
    snprintf(query_string, sizeof(query_string), "target=%s", field_value);

    expect_default_query(parse_query(query_string));
}

static void test_rejects_oversized_metareq_box_property(void)
{
    char query_string[128];
    char box_property[64];

    memset(box_property, 'B', 40);
    box_property[40] = '\0';
    snprintf(query_string, sizeof(query_string), "metareq=[%s]", box_property);

    expect_default_query(parse_query(query_string));
}

static void test_rejects_oversized_metareq_box_property_list_entry(void)
{
    char query_string[128];
    char box_property[64];

    memset(box_property, 'B', 40);
    box_property[40] = '\0';
    snprintf(query_string, sizeof(query_string), "metareq=[%s;ftyp]", box_property);

    expect_default_query(parse_query(query_string));
}

static void test_rejects_too_many_metareq_box_properties(void)
{
    const char prefix[] = "metareq=[";
    char query_string[128];
    size_t pos;
    int i;

    memcpy(query_string, prefix, sizeof(prefix) - 1);
    pos = sizeof(prefix) - 1;

    for (i = 0; i < 40; i++) {
        query_string[pos++] = 'a';
        query_string[pos++] = ';';
    }
    query_string[pos - 1] = ']';
    query_string[pos] = '\0';

    expect_default_query(parse_query(query_string));
}

int main(void)
{
    test_rejects_oversized_field_name();
    test_rejects_oversized_field_value();
    test_rejects_oversized_metareq_box_property();
    test_rejects_oversized_metareq_box_property_list_entry();
    test_rejects_too_many_metareq_box_properties();

    return 0;
}
