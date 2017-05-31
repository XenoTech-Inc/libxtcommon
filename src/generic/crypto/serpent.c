/*
Edited by Medicijnman 20150310
See below for original author and license.
*/
// Copyright in this code is held by Dr B.R. Gladman but free direct or
// derivative use is permitted subject to acknowledgement of its origin
// and subject to any constraints placed on the use of the algorithm by
// its designers (if such constraints may exist, this will be indicated
// below).
//
// Dr. B. R. Gladman                               . 25th January 2000.
// Copyright:   Dr B. R Gladman (gladman@seven77.demon.co.uk)
//              and Sam Simpson (s.simpson@mia.co.uk)
//              17th December 1998
//
// We hereby give permission for information in this file to be
// used freely subject only to acknowledgement of its origin
#include <xt/crypto.h>
#include <xt/error.h>

// FIXME use something better to make it also work normally on windows
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include <xt/endian.h>

void _xtSerpentEncryptBlock(struct xtSerpent *ctx, void *restrict dest, const void *restrict src);
void _xtSerpentDecryptBlock(struct xtSerpent *ctx, void *restrict dest, const void *restrict src);

#define rotr(x,n) (((x)>>((int)((n)&0x1f)))|((x)<<((int)((32-((n)&0x1f))))))
#define rotl(x,n) (((x)<<((int)((n)&0x1f)))|((x)>>((int)((32-((n)&0x1f))))))
#define sb0(a,b,c,d,e,f,g,h) {t1=a^d;t2=a&d;t3=c^t1;t6=b&t1;t4=b^t3;t10=~t3;h=t2^t4;t7=a^t6;t14=~t7;t8=c|t7;t11=t3^t7;g=t4^t8;t12=h&t11;f=t10^t12;e=t12^t14;}
#define ib0(a,b,c,d,e,f,g,h) {t1=~a;t2=a^b;t3=t1|t2;t4=d^t3;t7=d&t2;t5=c^t4;t8=t1^t7;g=t2^t5;t11=a&t4;t9=g&t8;t14=t5^t8;f=t4^t9;t12=t5|f;h=t11^t12;e=h^t14;}
#define sb1(a,b,c,d,e,f,g,h) {t1=~a;t2=b^t1;t3=a|t2;t4=d|t2;t5=c^t3;g=d^t5;t7=b^t4;t8=t2^g;t9= t5&t7;h=t8^t9;t11=t5^t7;f=h^t11;t13=t8&t11;e=t5^t13;}
#define ib1(a,b,c,d,e,f,g,h) {t1=a^d;t2=a&b;t3=b^c;t4=a^t3;t5=b|d;t7=c|t1;h=t4^t5;t8=b^t7;t11=~t2;t9=t4&t8;f=t1^t9;t13=t9^t11;t12=h&f;g=t12^t13;t15=a&d;t16=c^t13;e=t15^t16;}
#define sb2(a,b,c,d,e,f,g,h) {t1=~a;t2=b^d;t3=c&t1;t13=d|t1;e=t2^t3;t5=c^t1;t6=c^e;t7=b&t6;t10=e|t5;h=t5^t7;t9=d|t7;t11=t9&t10;t14=t2^h;g=a^t11;t15=g^t13;f=t14^t15;}
#define ib2(a,b,c,d,e,f,g,h) {t1=b^d;t2=~t1;t3=a^c;t4=c^t1;t7=a|t2;t5=b&t4;t8=d^t7;t11=~t4;e=t3^t5;t9=t3|t8;t14=d&t11;h=t1^t9;t12=e|h;f=t11^t12;t15=t3^t12;g=t14^t15;}
#define sb3(a,b,c,d,e,f,g,h) {t1=a^c;t2=d^t1;t3=a&t2;t4=d^t3;t5=b&t4;g=t2^t5;t7=a|g;t8=b|d;t11=a|d;t9=t4&t7;f=t8^t9;t12=b^t11;t13=g^t9;t15=t3^t8;h=t12^t13;t16=c&t15;e=t12^t16;}
#define ib3(a,b,c,d,e,f,g,h) {t1=b^c;t2=b|c;t3=a^c;t7=a^d;t4=t2^t3;t5=d|t4;t9=t2^t7;e=t1^t5;t8=t1|t5;t11=a&t4;g=t8^t9;t12=e|t9;f=t11^t12;t14=a&g;t15=t2^t14;t16=e&t15;h=t4^t16;}
#define sb4(a,b,c,d,e,f,g,h) {t1=a^d;t2=d&t1;t3=c^t2;t4=b|t3;h=t1^t4;t6=~b;t7=t1|t6;e=t3^t7;t9=a&e;t10=t1^t6;t11=t4&t10;g=t9^t11;t13=a^t3;t14=t10&g;f= t13^t14;}
#define ib4(a,b,c,d,e,f,g,h) {t1=c^d;t2=c|d;t3=b^t2;t4=a&t3;f=t1^t4;t6=a^d;t7=b|d;t8=t6&t7;h=t3^t8;t10=~a;t11=c^h;t12=t10|t11;e=t3^t12;t14=c|t4;t15=t7^t14;t16=h|t10;g=t15^t16;}
#define sb5(a,b,c,d,e,f,g,h) {t1=~a;t2=a^b;t3=a^d;t4=c^t1;t5=t2|t3;e=t4^t5;t7=d&e;t8=t2^e;t10=t1|e;f=t7^t8;t11=t2|t7;t12=t3^t10;t14=b^t7;g=t11^t12;t15=f&t12;h=t14^t15;}
#define ib5(a,b,c,d,e,f,g,h) {t1=~c;t2=b&t1;t3=d^t2;t4=a&t3;t5=b^t1;h=t4^t5;t7=b|h;t8=a&t7;f=t3^t8;t10=a|d;t11=t1^t7;e=t10^t11;t13=a^c;t14=b&t10;t15=t4|t13;g=t14^t15;}
#define sb6(a,b,c,d,e,f,g,h) {t1=~a;t2=a^d;t3=b^t2;t4=t1|t2;t5=c^t4;f=b^t5;t13=~t5;t7=t2|f;t8=d^t7;t9=t5&t8;g=t3^t9;t11=t5^t8;e=g^t11;t14=t3&t11;h=t13^t14;}
#define ib6(a,b,c,d,e,f,g,h) {t1=~a;t2=a^b;t3=c^t2;t4=c|t1;t5=d^t4;t13=d&t1;f=t3^t5;t7=t3&t5;t8=t2^t7;t9=b|t8;h=t5^t9;t11=b|h;e=t8^t11;t14=t3^t11;g=t13^t14;}
#define sb7(a,b,c,d,e,f,g,h) {t1=~c;t2=b^c;t3=b|t1;t4=d^t3;t5=a&t4;t7=a^d;h=t2^t5;t8=b^t5;t9=t2|t8;t11=d&t3;f=t7^t9;t12=t5^f;t15=t1|t4;t13=h&t12;g=t11^t13;t16=t12^g;e=t15^t16;}
#define ib7(a,b,c,d,e,f,g,h) {t1=a&b;t2=a|b;t3=c|t1;t4=d&t2;h=t3^t4;t6=~d;t7=b^t4;t8=h^t6;t11=c^t7;t9=t7|t8;f=a^t9;t12=d|f;e=t11^t12;t14=a&h;t15=t3^f;t16=e^t14;g=t15^t16;}
#define ai(r,a,b,c,d) { a ^= ctx->l_key[4 * r +  8]; b ^= ctx->l_key[4 * r +  9]; c ^= ctx->l_key[4 * r + 10]; d ^= ctx->l_key[4 * r + 11]; }
#define k_set(r,a,b,c,d) { a = ctx->l_key[4 * r +  8]; b = ctx->l_key[4 * r +  9]; c = ctx->l_key[4 * r + 10]; d = ctx->l_key[4 * r + 11]; }
#define k_get(r,a,b,c,d) { ctx->l_key[4 * r +  8] = a; ctx->l_key[4 * r +  9] = b; ctx->l_key[4 * r + 10] = c; ctx->l_key[4 * r + 11] = d; }
#define rot(a,b,c,d) { a = rotl(a, 13); c = rotl(c, 3); d ^= c ^ (a << 3); b ^= a ^ c; d = rotl(d, 7); b = rotl(b, 1); a ^= b ^ d; c ^= d ^ (b << 7); a = rotl(a, 5); c = rotl(c, 22); }
#define irot(a,b,c,d) { c = rotr(c, 22); a = rotr(a, 5); c ^= d ^ (b << 7); a ^= b ^ d; d = rotr(d, 7); b = rotr(b, 1); d ^= c ^ (a << 3); b ^= a ^ c; c = rotr(c, 3); a = rotr(a, 13); }
#define aa(x,y) k_set(x,a,b,c,d);sb##y(a,b,c,d,e,f,g,h);k_get(x,e,f,g,h);
#define ab(x) aa(x,3)aa(x+1,2)aa(x+2,1)aa(x+3,0)aa(x+4,7)aa(x+5,6)aa(x+6,5)aa(x+7,4)
#define AA 0x9e3779b9
#define ac(a,b) a=xthtole32(*((const uint32_t*)(in_blk+b)));
#define ad(x,y) ai((x),a,b,c,d);sb##y(a,b,c,d,e,f,g,h);rot(e,f,g,h);
#define ae(x,y) ai((x),e,f,g,h);sb##y(e,f,g,h,a,b,c,d);rot(a,b,c,d);
#define af(x) ad(x,0)ae(x+1,1)ad(x+2,2)ae(x+3,3)ad(x+4,4)ae(x+5,5)ad(x+6,6)ae(x+7,7)
#define ag(a,b) *((uint32_t*)(out_blk+b))=xtle32toh(a);
#define ah(x,y) irot(e,f,g,h);ib##y(e,f,g,h,a,b,c,d);ai((x),a,b,c,d);
#define aj(x,y) irot(a,b,c,d);ib##y(a,b,c,d,e,f,g,h);ai((x),e,f,g,h);
#define ak(x) aj(x,7)ah(x-1,6)aj(x-2,5)ah(x-3,4)aj(x-4,3)ah(x-5,2)aj(x-6,1)ah(x-7,0)

int xtSerpentInit(struct xtSerpent *ctx, const void *src, unsigned key_len)
{
	const uint8_t *in_key = src;
	uint32_t  i,lk,a,b,c,d,e,f,g,h;
	uint32_t  t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16;
	if (key_len > 256) return XT_EOVERFLOW;
	i = 0; lk = (key_len + 31) / 32;
	while (i < lk) {
		ctx->l_key[i] = xthtole32(*((const uint32_t *)(in_key + 4 * i))); i++;
	}
	if (key_len < 256) {
		while(i < 8)
			ctx->l_key[i++] = 0;
		i = key_len / 32; lk = 1 << key_len % 32;
		ctx->l_key[i] = (ctx->l_key[i] & (lk - 1)) | lk;
	}
	t1 = ctx->l_key[2] ^ ctx->l_key[4] ^ ctx->l_key[6] ^ AA;
	t2 = ctx->l_key[3] ^ ctx->l_key[5] ^ ctx->l_key[7] ^ AA;
	for (i = 0; i < 132; i += 2) {
		ctx->l_key[i + 8] = rotr(i ^ ctx->l_key[i] ^ t2, 21);
		t1 ^= ctx->l_key[i + 2] ^ ctx->l_key[i + 8];
		ctx->l_key[i + 9] = rotr((i + 1) ^ ctx->l_key[i + 1] ^ t1, 21);
		t2 ^= ctx->l_key[i + 3] ^ ctx->l_key[i + 9];
	}
	ab(0)ab(8)ab(16)ab(24)
	return 0;
}

void _xtSerpentEncryptBlock(struct xtSerpent *ctx, void *restrict dest, const void *restrict src)
{
	const uint8_t *in_blk = src;
	uint8_t *out_blk = dest;
	uint32_t  a,b,c,d,e,f,g,h;
	uint32_t  t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16;
	ac(a,0)ac(b,4)ac(c,8)ac(d,12)
	af(0)af(8)af(16)
	ad(24,0)ae(25,1)ad(26,2)ae(27,3)ad(28,4)ae(29,5)ad(30,6)
	ai(31,e,f,g,h); sb7(e,f,g,h,a,b,c,d); ai(32,a,b,c,d);
	ag(a,0)ag(b,4)ag(c,8)ag(d,12)
}

void _xtSerpentDecryptBlock(struct xtSerpent *ctx, void *restrict dest, const void *restrict src)
{
	const uint8_t *in_blk = src;
	uint8_t *out_blk = dest;
	uint32_t  a,b,c,d,e,f,g,h;
	uint32_t  t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16;
	ac(a,0)ac(b,4)ac(c,8)ac(d,12)
	ai(32,a,b,c,d); ib7(a,b,c,d,e,f,g,h); ai(31,e,f,g,h);
	ah(30,6)aj(29,5)ah(28,4)aj(27,3)ah(26,2)aj(25,1)ah(24,0)
	ak(23)ak(15)ak(7)
	ag(a,0)ag(b,4)ag(c,8)ag(d,12)
}

void xtSerpentEncrypt(struct xtSerpent *ctx, void *restrict dest, const void *restrict data, size_t dataSize)
{
	const uint8_t *in = data;
	uint8_t *out = dest;
	for (size_t i = 0; i < dataSize; i += 16)
		_xtSerpentEncryptBlock(ctx, &out[i], &in[i]);
}

void xtSerpentDecrypt(struct xtSerpent *ctx, void *restrict dest, const void *restrict data, size_t dataSize)
{
	const uint8_t *in = data;
	uint8_t *out = dest;
	for (size_t i = 0; i < dataSize; i += 16)
		_xtSerpentDecryptBlock(ctx, &out[i], &in[i]);
}
