/* 	$OpenBSD$ */
/*
 * Regress test for sshbuf.h buffer API
 *
 * Placed in the public domain
 */

#include <sys/types.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "test_helper.h"
#include "err.h"
#include "sshbuf.h"

void sshbuf_getput_basic_tests(void);

void
sshbuf_getput_basic_tests(void)
{
	struct sshbuf *p1, *p2;
	u_char *d, d2[32], x[] = {
		0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x00, 0x99
	};
	u_int64_t v64;
	u_int32_t v32;
	u_int16_t v16;
	u_char v8;
	size_t s;
	char *s2;
	int r;

	TEST_START("PEEK_U64");
	ASSERT_U64_EQ(PEEK_U64(x), 0x1122334455667788ULL);
	TEST_DONE();

	TEST_START("PEEK_U32");
	ASSERT_U32_EQ(PEEK_U32(x), 0x11223344);
	TEST_DONE();

	TEST_START("PEEK_U16");
	ASSERT_U16_EQ(PEEK_U16(x), 0x1122);
	TEST_DONE();

	TEST_START("POKE_U64");
	bzero(d2, sizeof(d2));
	POKE_U64(d2, 0x1122334455667788ULL);
	ASSERT_MEM_EQ(d2, x, 8);
	TEST_DONE();
	
	TEST_START("POKE_U32");
	bzero(d2, sizeof(d2));
	POKE_U32(d2, 0x11223344);
	ASSERT_MEM_EQ(d2, x, 4);
	TEST_DONE();
	
	TEST_START("POKE_U16");
	bzero(d2, sizeof(d2));
	POKE_U16(d2, 0x1122);
	ASSERT_MEM_EQ(d2, x, 2);
	TEST_DONE();

	TEST_START("sshbuf_put");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_put(p1, x, 5), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 5);
	d = sshbuf_ptr(p1);
	ASSERT_PTR_NE(d, NULL);
	ASSERT_U8_EQ(d[0], 0x11);
	ASSERT_U8_EQ(d[1], 0x22);
	ASSERT_U8_EQ(d[2], 0x33);
	ASSERT_U8_EQ(d[3], 0x44);
	ASSERT_U8_EQ(d[4], 0x55);
	TEST_DONE();

	TEST_START("sshbuf_get");
	ASSERT_INT_EQ(sshbuf_get(p1, d2, 4), 0);
	ASSERT_MEM_EQ(d2, x, 4);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 1);
	ASSERT_U8_EQ(*(sshbuf_ptr(p1)), 0x55);
	TEST_DONE();

	TEST_START("sshbuf_get truncated");
	r = sshbuf_get(p1, d2, 4);
	ASSERT_INT_EQ(r, SSH_ERR_MESSAGE_INCOMPLETE);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 1);
	ASSERT_U8_EQ(*(sshbuf_ptr(p1)), 0x55);
	TEST_DONE();

	TEST_START("sshbuf_put truncated");
	ASSERT_INT_EQ(sshbuf_set_max_size(p1, 4), 0);
	r = sshbuf_put(p1, x, 5);
	ASSERT_INT_EQ(r, SSH_ERR_NO_BUFFER_SPACE);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 1);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_get_u64");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_put(p1, x, 10), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 10);
	ASSERT_INT_EQ(sshbuf_get_u64(p1, &v64), 0);
	ASSERT_U64_EQ(v64, 0x1122334455667788ULL);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 2);
	TEST_DONE();

	TEST_START("sshbuf_get_u64 truncated");
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 2);
	r = sshbuf_get_u64(p1, &v64);
	ASSERT_INT_EQ(r, SSH_ERR_MESSAGE_INCOMPLETE);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 2);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_get_u32");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_put(p1, x, 10), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 10);
	ASSERT_INT_EQ(sshbuf_get_u32(p1, &v32), 0);
	ASSERT_U32_EQ(v32, 0x11223344);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 6);
	ASSERT_INT_EQ(sshbuf_get_u32(p1, &v32), 0);
	ASSERT_U32_EQ(v32, 0x55667788);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 2);
	TEST_DONE();

	TEST_START("sshbuf_get_u32 truncated");
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 2);
	r = sshbuf_get_u32(p1, &v32);
	ASSERT_INT_EQ(r, SSH_ERR_MESSAGE_INCOMPLETE);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 2);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_get_u16");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_put(p1, x, 9), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 9);
	ASSERT_INT_EQ(sshbuf_get_u16(p1, &v16), 0);
	ASSERT_U16_EQ(v16, 0x1122);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 7);
	ASSERT_INT_EQ(sshbuf_get_u16(p1, &v16), 0);
	ASSERT_U16_EQ(v16, 0x3344);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 5);
	ASSERT_INT_EQ(sshbuf_get_u16(p1, &v16), 0);
	ASSERT_U16_EQ(v16, 0x5566);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 3);
	ASSERT_INT_EQ(sshbuf_get_u16(p1, &v16), 0);
	ASSERT_U16_EQ(v16, 0x7788);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 1);
	TEST_DONE();

	TEST_START("sshbuf_get_u16 truncated");
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 1);
	r = sshbuf_get_u16(p1, &v16);
	ASSERT_INT_EQ(r, SSH_ERR_MESSAGE_INCOMPLETE);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 1);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_get_u8");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_put(p1, x, 2), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 2);
	ASSERT_INT_EQ(sshbuf_get_u8(p1, &v8), 0);
	ASSERT_U8_EQ(v8, 0x11);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 1);
	ASSERT_INT_EQ(sshbuf_get_u8(p1, &v8), 0);
	ASSERT_U8_EQ(v8, 0x22);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 0);
	TEST_DONE();

	TEST_START("sshbuf_get_u8 truncated");
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 0);
	r = sshbuf_get_u8(p1, &v8);
	ASSERT_INT_EQ(r, SSH_ERR_MESSAGE_INCOMPLETE);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 0);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_put_u64");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_put_u64(p1, 0x1122334455667788ULL), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 8);
	ASSERT_MEM_EQ(sshbuf_ptr(p1), x, 8);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_put_u64 exact");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_set_max_size(p1, 8), 0);
	ASSERT_INT_EQ(sshbuf_put_u64(p1, 0x1122334455667788ULL), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 8);
	ASSERT_MEM_EQ(sshbuf_ptr(p1), x, 8);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_put_u64 limited");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_set_max_size(p1, 7), 0);
	r = sshbuf_put_u64(p1, 0x1122334455667788ULL);
	ASSERT_INT_EQ(r, SSH_ERR_NO_BUFFER_SPACE);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 0);
	sshbuf_free(p1);
	TEST_DONE();
	
	TEST_START("sshbuf_put_u32");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_put_u32(p1, 0x11223344), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 4);
	ASSERT_MEM_EQ(sshbuf_ptr(p1), x, 4);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_put_u32 exact");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_set_max_size(p1, 4), 0);
	ASSERT_INT_EQ(sshbuf_put_u32(p1, 0x11223344), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 4);
	ASSERT_MEM_EQ(sshbuf_ptr(p1), x, 4);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_put_u32 limited");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_set_max_size(p1, 3), 0);
	r = sshbuf_put_u32(p1, 0x11223344);
	ASSERT_INT_EQ(r, SSH_ERR_NO_BUFFER_SPACE);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 0);
	sshbuf_free(p1);
	TEST_DONE();
	
	TEST_START("sshbuf_put_u16");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_put_u16(p1, 0x1122), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 2);
	ASSERT_MEM_EQ(sshbuf_ptr(p1), x, 2);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_put_u16");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_set_max_size(p1, 2), 0);
	ASSERT_INT_EQ(sshbuf_put_u16(p1, 0x1122), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 2);
	ASSERT_MEM_EQ(sshbuf_ptr(p1), x, 2);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_put_u16 limited");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_set_max_size(p1, 1), 0);
	r = sshbuf_put_u16(p1, 0x1122);
	ASSERT_INT_EQ(r, SSH_ERR_NO_BUFFER_SPACE);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 0);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_get_string");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_put_u32(p1, sizeof(x)), 0);
	ASSERT_INT_EQ(sshbuf_put(p1, x, sizeof(x)), 0);
	ASSERT_INT_EQ(sshbuf_put_u32(p1, sizeof(x)), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), sizeof(x) + 4 + 4);
	ASSERT_INT_EQ(sshbuf_get_string(p1, &d, &s), 0);
	ASSERT_SIZE_T_EQ(s, sizeof(x));
	ASSERT_MEM_EQ(d, x, sizeof(x));
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 4);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_get_string exact");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_set_max_size(p1, sizeof(x) + 4), 0);
	ASSERT_INT_EQ(sshbuf_put_u32(p1, sizeof(x)), 0);
	ASSERT_INT_EQ(sshbuf_put(p1, x, sizeof(x)), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), sizeof(x) + 4);
	ASSERT_INT_EQ(sshbuf_get_string(p1, &d, &s), 0);
	ASSERT_SIZE_T_EQ(s, sizeof(x));
	ASSERT_MEM_EQ(d, x, sizeof(x));
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 0);
	free(d);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_get_string truncated");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_put_u32(p1, sizeof(x)), 0);
	ASSERT_INT_EQ(sshbuf_put(p1, x, sizeof(x)), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), sizeof(x) + 4);
	ASSERT_INT_EQ(sshbuf_consume_end(p1, 1), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), sizeof(x) + 3);
	r = sshbuf_get_string(p1, &d, &s);
	ASSERT_INT_EQ(r, SSH_ERR_MESSAGE_INCOMPLETE);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), sizeof(x) + 3);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_get_string giant");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_put_u32(p1, 0xffffffff), 0);
	ASSERT_INT_EQ(sshbuf_put(p1, x, sizeof(x)), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), sizeof(x) + 4);
	r = sshbuf_get_string(p1, &d, &s);
	ASSERT_INT_EQ(r, SSH_ERR_MESSAGE_INCOMPLETE);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), sizeof(x) + 4);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_get_cstring embedded \\0");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_put_u32(p1, sizeof(x)), 0);
	ASSERT_INT_EQ(sshbuf_put(p1, x, sizeof(x)), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), sizeof(x) + 4);
	r = sshbuf_get_cstring(p1, &s2, NULL);
	ASSERT_INT_EQ(r, SSH_ERR_INVALID_FORMAT);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_get_cstring trailing \\0");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_put_u32(p1, sizeof(x) - 1), 0);
	ASSERT_INT_EQ(sshbuf_put(p1, x, sizeof(x) - 1), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), sizeof(x) + 4 - 1);
	ASSERT_INT_EQ(sshbuf_get_cstring(p1, &s2, &s), 0);
	ASSERT_SIZE_T_EQ(s, sizeof(x) - 1);
	ASSERT_MEM_EQ(s2, x, s);
	free(s2);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_put_string");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_put_string(p1, x, sizeof(x)), 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), sizeof(x) + 4);
	ASSERT_U32_EQ(PEEK_U32(sshbuf_ptr(p1)), sizeof(x));
	ASSERT_MEM_EQ(sshbuf_ptr(p1) + 4, x, sizeof(x));
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_put_string limited");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	ASSERT_INT_EQ(sshbuf_set_max_size(p1, sizeof(x) + 4 - 1), 0);
	r = sshbuf_put_string(p1, x, sizeof(x));
	ASSERT_INT_EQ(r, SSH_ERR_NO_BUFFER_SPACE);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 0);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_put_string giant");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	r = sshbuf_put_string(p1, (void *)0x01, 0xfffffffc);
	ASSERT_INT_EQ(r, SSH_ERR_NO_BUFFER_SPACE);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 0);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_putf");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	r = sshbuf_putf(p1, "%s %d %x", "hello", 23, 0x5f);
	ASSERT_INT_EQ(r, 0);
	ASSERT_SIZE_T_EQ(sshbuf_len(p1), 11);
	ASSERT_MEM_EQ(sshbuf_ptr(p1), "hello 23 5f", 11);
	sshbuf_free(p1);
	TEST_DONE();

	TEST_START("sshbuf_putb");
	p1 = sshbuf_new();
	ASSERT_PTR_NE(p1, NULL);
	p2 = sshbuf_new();
	ASSERT_PTR_NE(p2, NULL);
	ASSERT_INT_EQ(sshbuf_put(p1, "blahblahblah", 12), 0);
	ASSERT_INT_EQ(sshbuf_putb(p2, p1), 0);
	sshbuf_free(p1);
	ASSERT_SIZE_T_EQ(sshbuf_len(p2), 12);
	ASSERT_MEM_EQ(sshbuf_ptr(p2), "blahblahblah", 12);
	sshbuf_free(p2);
	TEST_DONE();
}