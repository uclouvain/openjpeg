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

#include "msgqueue_manager.h"

static void fail(const char *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

static void expect_empty(msgqueue_param_t *msgqueue)
{
    if (msgqueue->first != NULL || msgqueue->last != NULL) {
        fail("truncated VBAS unexpectedly queued messages");
    }
}

static void expect_single_zero_length_message(msgqueue_param_t *msgqueue)
{
    message_param_t *msg = msgqueue->first;

    if (msg == NULL || msg != msgqueue->last || msg->next != NULL) {
        fail("valid JPIP message was not preserved");
    }

    if (msg->class_id != 0 || msg->in_class_id != 0 || msg->bin_offset != 0 ||
            msg->length != 0 || msg->last_byte != OPJ_FALSE) {
        fail("parsed JPIP message fields changed unexpectedly");
    }
}

static void test_rejects_truncated_vbas_without_queuing_messages(void)
{
    static const Byte_t stream[] = { 0x40, 0x00, 0x00, 0x00, 0x80 };
    msgqueue_param_t *msgqueue = gene_msgqueue(OPJ_TRUE, NULL);

    parse_JPIPstream((Byte_t *)stream, sizeof(stream), 0, msgqueue);
    expect_empty(msgqueue);

    delete_msgqueue(&msgqueue);
}

static void test_preserves_existing_messages_on_truncated_append(void)
{
    static const Byte_t valid_stream[] = { 0x40, 0x00, 0x00, 0x00 };
    static const Byte_t truncated_stream[] = { 0x80 };
    msgqueue_param_t *msgqueue = gene_msgqueue(OPJ_TRUE, NULL);

    parse_JPIPstream((Byte_t *)valid_stream, sizeof(valid_stream), 0, msgqueue);
    expect_single_zero_length_message(msgqueue);

    parse_JPIPstream((Byte_t *)truncated_stream, sizeof(truncated_stream),
                     sizeof(valid_stream), msgqueue);
    expect_single_zero_length_message(msgqueue);

    delete_msgqueue(&msgqueue);
}

int main(void)
{
    test_rejects_truncated_vbas_without_queuing_messages();
    test_preserves_existing_messages_on_truncated_append();

    return 0;
}
