// XT headers
#include <xt/hash.h>

// STD headers
#include <string.h> // memset

/**
 * The MD5 and SHAXXX functions are all taken from github.com/WaterJuice/CryptLib and changed to fit in here.
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	MD5 START
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  F, G, H, I
//
//  The basic MD5 functions. F and G are optimized compared to their RFC 1321 definitions for architectures that lack
//  an AND-NOT instruction, just like in Colin Plumb's implementation.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MD5_F( x, y, z )	( (z) ^ ((x) & ((y) ^ (z))) )
#define MD5_G( x, y, z )	( (y) ^ ((z) & ((x) ^ (y))) )
#define MD5_H( x, y, z )	( (x) ^ (y) ^ (z) )
#define MD5_I( x, y, z )	( (y) ^ ((x) | ~(z)) )


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  MD5_STEP
//
//  The MD5 transformation for all four rounds.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MD5_STEP( f, a, b, c, d, x, t, s )                          \
	(a) += f((b), (c), (d)) + (x) + (t);                        \
	(a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s))));  \
	(a) += (b);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  MD5_SET, MD5_GET
//
//  MD5_SET reads 4 input bytes in little-endian byte order and stores them in a properly aligned word in host byte order.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MD5_SET(n)	(*(uint32_t*) &ptr[(n) * 4])
#define MD5_GET(n)	MD5_SET(n)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  md5_transform
//
//  This processes one or more 64-byte data blocks, but does NOT update the bit counters. There are no alignment
//  requirements.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void *md5_transform(struct _xtMD5Context *restrict ctx, const uint8_t *restrict data, uintmax_t size)
{
	uint8_t *ptr;
	uint32_t a, b, c, d;
	uint32_t saved_a, saved_b, saved_c, saved_d;

	ptr = (uint8_t*)data;

	a = ctx->a;
	b = ctx->b;
	c = ctx->c;
	d = ctx->d;

	do {
		saved_a = a;
		saved_b = b;
		saved_c = c;
		saved_d = d;

		// Round 1
		MD5_STEP( MD5_F, a, b, c, d, MD5_SET(0),  0xd76aa478, 7 )
		MD5_STEP( MD5_F, d, a, b, c, MD5_SET(1),  0xe8c7b756, 12 )
		MD5_STEP( MD5_F, c, d, a, b, MD5_SET(2),  0x242070db, 17 )
		MD5_STEP( MD5_F, b, c, d, a, MD5_SET(3),  0xc1bdceee, 22 )
		MD5_STEP( MD5_F, a, b, c, d, MD5_SET(4),  0xf57c0faf, 7 )
		MD5_STEP( MD5_F, d, a, b, c, MD5_SET(5),  0x4787c62a, 12 )
		MD5_STEP( MD5_F, c, d, a, b, MD5_SET(6),  0xa8304613, 17 )
		MD5_STEP( MD5_F, b, c, d, a, MD5_SET(7),  0xfd469501, 22 )
		MD5_STEP( MD5_F, a, b, c, d, MD5_SET(8 ),  0x698098d8, 7 )
		MD5_STEP( MD5_F, d, a, b, c, MD5_SET(9 ),  0x8b44f7af, 12 )
		MD5_STEP( MD5_F, c, d, a, b, MD5_SET(10 ), 0xffff5bb1, 17 )
		MD5_STEP( MD5_F, b, c, d, a, MD5_SET(11 ), 0x895cd7be, 22 )
		MD5_STEP( MD5_F, a, b, c, d, MD5_SET(12 ), 0x6b901122, 7 )
		MD5_STEP( MD5_F, d, a, b, c, MD5_SET(13 ), 0xfd987193, 12 )
		MD5_STEP( MD5_F, c, d, a, b, MD5_SET(14 ), 0xa679438e, 17 )
		MD5_STEP( MD5_F, b, c, d, a, MD5_SET(15 ), 0x49b40821, 22 )

		// Round 2
		MD5_STEP( MD5_G, a, b, c, d, MD5_GET(1),  0xf61e2562, 5 )
		MD5_STEP( MD5_G, d, a, b, c, MD5_GET(6),  0xc040b340, 9 )
		MD5_STEP( MD5_G, c, d, a, b, MD5_GET(11), 0x265e5a51, 14 )
		MD5_STEP( MD5_G, b, c, d, a, MD5_GET(0),  0xe9b6c7aa, 20 )
		MD5_STEP( MD5_G, a, b, c, d, MD5_GET(5),  0xd62f105d, 5 )
		MD5_STEP( MD5_G, d, a, b, c, MD5_GET(10), 0x02441453, 9 )
		MD5_STEP( MD5_G, c, d, a, b, MD5_GET(15), 0xd8a1e681, 14 )
		MD5_STEP( MD5_G, b, c, d, a, MD5_GET(4),  0xe7d3fbc8, 20 )
		MD5_STEP( MD5_G, a, b, c, d, MD5_GET(9),  0x21e1cde6, 5 )
		MD5_STEP( MD5_G, d, a, b, c, MD5_GET(14), 0xc33707d6, 9 )
		MD5_STEP( MD5_G, c, d, a, b, MD5_GET(3),  0xf4d50d87, 14 )
		MD5_STEP( MD5_G, b, c, d, a, MD5_GET(8),  0x455a14ed, 20 )
		MD5_STEP( MD5_G, a, b, c, d, MD5_GET(13), 0xa9e3e905, 5 )
		MD5_STEP( MD5_G, d, a, b, c, MD5_GET(2),  0xfcefa3f8, 9 )
		MD5_STEP( MD5_G, c, d, a, b, MD5_GET(7),  0x676f02d9, 14 )
		MD5_STEP( MD5_G, b, c, d, a, MD5_GET(12), 0x8d2a4c8a, 20 )

		// Round 3
		MD5_STEP( MD5_H, a, b, c, d, MD5_GET(5),  0xfffa3942, 4 )
		MD5_STEP( MD5_H, d, a, b, c, MD5_GET(8),  0x8771f681, 11 )
		MD5_STEP( MD5_H, c, d, a, b, MD5_GET(11), 0x6d9d6122, 16 )
		MD5_STEP( MD5_H, b, c, d, a, MD5_GET(14), 0xfde5380c, 23 )
		MD5_STEP( MD5_H, a, b, c, d, MD5_GET(1),  0xa4beea44, 4 )
		MD5_STEP( MD5_H, d, a, b, c, MD5_GET(4),  0x4bdecfa9, 11 )
		MD5_STEP( MD5_H, c, d, a, b, MD5_GET(7),  0xf6bb4b60, 16 )
		MD5_STEP( MD5_H, b, c, d, a, MD5_GET(10), 0xbebfbc70, 23 )
		MD5_STEP( MD5_H, a, b, c, d, MD5_GET(13), 0x289b7ec6, 4 )
		MD5_STEP( MD5_H, d, a, b, c, MD5_GET(0),  0xeaa127fa, 11 )
		MD5_STEP( MD5_H, c, d, a, b, MD5_GET(3),  0xd4ef3085, 16 )
		MD5_STEP( MD5_H, b, c, d, a, MD5_GET(6),  0x04881d05, 23 )
		MD5_STEP( MD5_H, a, b, c, d, MD5_GET(9),  0xd9d4d039, 4 )
		MD5_STEP( MD5_H, d, a, b, c, MD5_GET(12), 0xe6db99e5, 11 )
		MD5_STEP( MD5_H, c, d, a, b, MD5_GET(15), 0x1fa27cf8, 16 )
		MD5_STEP( MD5_H, b, c, d, a, MD5_GET(2),  0xc4ac5665, 23 )

		// Round 4
		MD5_STEP( MD5_I, a, b, c, d, MD5_GET(0),  0xf4292244, 6 )
		MD5_STEP( MD5_I, d, a, b, c, MD5_GET(7),  0x432aff97, 10 )
		MD5_STEP( MD5_I, c, d, a, b, MD5_GET(14), 0xab9423a7, 15 )
		MD5_STEP( MD5_I, b, c, d, a, MD5_GET(5),  0xfc93a039, 21 )
		MD5_STEP( MD5_I, a, b, c, d, MD5_GET(12), 0x655b59c3, 6 )
		MD5_STEP( MD5_I, d, a, b, c, MD5_GET(3),  0x8f0ccc92, 10 )
		MD5_STEP( MD5_I, c, d, a, b, MD5_GET(10), 0xffeff47d, 15 )
		MD5_STEP( MD5_I, b, c, d, a, MD5_GET(1),  0x85845dd1, 21 )
		MD5_STEP( MD5_I, a, b, c, d, MD5_GET(8),  0x6fa87e4f, 6 )
		MD5_STEP( MD5_I, d, a, b, c, MD5_GET(15), 0xfe2ce6e0, 10 )
		MD5_STEP( MD5_I, c, d, a, b, MD5_GET(6),  0xa3014314, 15 )
		MD5_STEP( MD5_I, b, c, d, a, MD5_GET(13), 0x4e0811a1, 21 )
		MD5_STEP( MD5_I, a, b, c, d, MD5_GET(4),  0xf7537e82, 6 )
		MD5_STEP( MD5_I, d, a, b, c, MD5_GET(11), 0xbd3af235, 10 )
		MD5_STEP( MD5_I, c, d, a, b, MD5_GET(2),  0x2ad7d2bb, 15 )
		MD5_STEP( MD5_I, b, c, d, a, MD5_GET(9),  0xeb86d391, 21 )

		a += saved_a;
		b += saved_b;
		c += saved_c;
		d += saved_d;

		ptr += 64;
	} while (size -= 64);

	ctx->a = a;
	ctx->b = b;
	ctx->c = c;
	ctx->d = d;

	return ptr;
}

static void md5_digest(struct _xtMD5Context *restrict ctx, uint8_t *restrict digest)
{
	uint32_t used;
	uint32_t free;

	used = ctx->lo & 0x3f;
	ctx->buffer[used++] = 0x80;
	free = 64 - used;

	if (free < 8) {
		memset (&ctx->buffer[used], 0, free);
		md5_transform(ctx, ctx->buffer, 64);
		used = 0;
		free = 64;
	}

	memset(&ctx->buffer[used], 0, free - 8);

	ctx->lo <<= 3;
	ctx->buffer[56] = (uint8_t) (ctx->lo);
	ctx->buffer[57] = (uint8_t) (ctx->lo >> 8);
	ctx->buffer[58] = (uint8_t) (ctx->lo >> 16);
	ctx->buffer[59] = (uint8_t) (ctx->lo >> 24);
	ctx->buffer[60] = (uint8_t) (ctx->hi);
	ctx->buffer[61] = (uint8_t) (ctx->hi >> 8);
	ctx->buffer[62] = (uint8_t) (ctx->hi >> 16);
	ctx->buffer[63] = (uint8_t) (ctx->hi >> 24);

	md5_transform(ctx, ctx->buffer, 64);

	digest[0]  = (uint8_t) (ctx->a);
	digest[1]  = (uint8_t) (ctx->a >> 8);
	digest[2]  = (uint8_t) (ctx->a >> 16);
	digest[3]  = (uint8_t) (ctx->a >> 24);
	digest[4]  = (uint8_t) (ctx->b);
	digest[5]  = (uint8_t) (ctx->b >> 8);
	digest[6]  = (uint8_t) (ctx->b >> 16);
	digest[7]  = (uint8_t) (ctx->b >> 24);
	digest[8]  = (uint8_t) (ctx->c);
	digest[9]  = (uint8_t) (ctx->c >> 8);
	digest[10] = (uint8_t) (ctx->c >> 16);
	digest[11] = (uint8_t) (ctx->c >> 24);
	digest[12] = (uint8_t) (ctx->d);
	digest[13] = (uint8_t) (ctx->d >> 8);
	digest[14] = (uint8_t) (ctx->d >> 16);
	digest[15] = (uint8_t) (ctx->d >> 24);
}

static void md5_init(struct _xtMD5Context *ctx)
{
	ctx->a = 0x67452301;
	ctx->b = 0xefcdab89;
	ctx->c = 0x98badcfe;
	ctx->d = 0x10325476;

	ctx->lo = 0;
	ctx->hi = 0;
}

static void md5_update(struct _xtMD5Context *restrict ctx, const uint8_t *restrict buf, size_t buflen)
{
	uint32_t saved_lo;
	uint32_t used;
	uint32_t free;
	saved_lo = ctx->lo;
	if ((ctx->lo = (saved_lo + buflen) & 0x1fffffff) < saved_lo) {
		ctx->hi++;
	}
	ctx->hi += (uint32_t)( buflen >> 29 );

	used = saved_lo & 0x3f;

	if (used) {
		free = 64 - used;

		if (buflen < free) {
			memcpy(&ctx->buffer[used], buf, buflen);
			return;
		}
		memcpy(&ctx->buffer[used], buf, free);
		buf = (uint8_t*) buf + free;
		buflen -= free;
		md5_transform(ctx, ctx->buffer, 64);
	}
	if (buflen >= 64) {
		buf = md5_transform(ctx, buf, buflen & ~(uint64_t) 0x3f);
		buflen &= 0x3f;
	}
	memcpy(ctx->buffer, buf, buflen);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	MD5 END
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SHA256 START
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SHA256_ror(value, bits) (((value) >> (bits)) | ((value) << (32 - (bits))))

#define SHA256_MIN(x, y) ( ((x)<(y))?(x):(y) )

#define SHA256_STORE32H(x, y)												\
	{ (y)[0] = (uint8_t)(((x)>>24)&255); (y)[1] = (uint8_t)(((x)>>16)&255);	\
	(y)[2] = (uint8_t)(((x)>>8)&255); (y)[3] = (uint8_t)((x)&255); }

#define SHA256_LOAD32H(x, y)				\
	{ x = ((uint32_t)((y)[0] & 255)<<24) | 	\
	((uint32_t)((y)[1] & 255)<<16) | 		\
	((uint32_t)((y)[2] & 255)<<8)  | 		\
	((uint32_t)((y)[3] & 255)); }

#define SHA256_STORE64H(x, y)													\
	{ (y)[0] = (uint8_t)(((x)>>56)&255); (y)[1] = (uint8_t)(((x)>>48)&255);		\
	(y)[2] = (uint8_t)(((x)>>40)&255); (y)[3] = (uint8_t)(((x)>>32)&255);		\
	(y)[4] = (uint8_t)(((x)>>24)&255); (y)[5] = (uint8_t)(((x)>>16)&255);		\
	(y)[6] = (uint8_t)(((x)>>8)&255); (y)[7] = (uint8_t)((x)&255); }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  CONSTANTS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// The K array
static const uint32_t SHA256_K[64] = {
	0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 0x3956c25bUL,
	0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL, 0xd807aa98UL, 0x12835b01UL,
	0x243185beUL, 0x550c7dc3UL, 0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL,
	0xc19bf174UL, 0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
	0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL, 0x983e5152UL,
	0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 0xc6e00bf3UL, 0xd5a79147UL,
	0x06ca6351UL, 0x14292967UL, 0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL,
	0x53380d13UL, 0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
	0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL, 0xd192e819UL,
	0xd6990624UL, 0xf40e3585UL, 0x106aa070UL, 0x19a4c116UL, 0x1e376c08UL,
	0x2748774cUL, 0x34b0bcb5UL, 0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL,
	0x682e6ff3UL, 0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
	0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

#define SHA256_BLOCK_SIZE	64

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  INTERNAL FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Various logical functions
#define SHA256_Ch( x, y, z )	(z ^ (x & (y ^ z)))
#define SHA256_Maj( x, y, z )	(((x | y) & z) | (x & y))
#define SHA256_S( x, n )		SHA256_ror((x),(n))
#define SHA256_R( x, n )		(((x)&0xFFFFFFFFUL)>>(n))
#define SHA256_Sigma0( x )		(SHA256_S(x, 2) ^ SHA256_S(x, 13) ^ SHA256_S(x, 22))
#define SHA256_Sigma1( x )		(SHA256_S(x, 6) ^ SHA256_S(x, 11) ^ SHA256_S(x, 25))
#define SHA256_Gamma0( x )		(SHA256_S(x, 7) ^ SHA256_S(x, 18) ^ SHA256_R(x, 3))
#define SHA256_Gamma1( x )		(SHA256_S(x, 17) ^ SHA256_S(x, 19) ^ SHA256_R(x, 10))

#define SHA256_Round( a, b, c, d, e, f, g, h, i )							\
	t0 = h + SHA256_Sigma1(e) + SHA256_Ch(e, f, g) + SHA256_K[i] + W[i];	\
	t1 = SHA256_Sigma0(a) + SHA256_Maj(a, b, c);							\
	d += t0;																\
	h  = t0 + t1;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  TransformFunction
//
//  Compress 512-bits
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void sha256_transform(struct _xtSHA256Context *restrict ctx, uint8_t *restrict buf)
{
	uint32_t	S[8];
	uint32_t	W[64];
	uint32_t	t0;
	uint32_t	t1;
	uint32_t	t;
	int			i;

	// Copy state into S
	for (i = 0; i < 8; i++) {
		S[i] = ctx->state[i];
	}

	// Copy the state into 512-bits into W[0..15]
	for (i = 0; i < 16; i++) {
		SHA256_LOAD32H(W[i], buf + (4*i));
	}

	// Fill W[16..63]
	for (i = 16; i < 64; i++) {
		W[i] = SHA256_Gamma1(W[i-2]) + W[i-7] + SHA256_Gamma0(W[i-15]) + W[i-16];
	}

	// Compress
	for (i = 0; i < 64; i++) {
		SHA256_Round(S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], i);
		t = S[7];
		S[7] = S[6];
		S[6] = S[5];
		S[5] = S[4];
		S[4] = S[3];
		S[3] = S[2];
		S[2] = S[1];
		S[1] = S[0];
		S[0] = t;
	}

	// Feedback
	for (i = 0; i < 8; i++) {
		ctx->state[i] = ctx->state[i] + S[i];
	}
}

static void sha256_digest(struct _xtSHA256Context *restrict ctx, uint8_t *restrict digest)
{
	int i;

	if (ctx->curlen >= sizeof ctx->buf)
		return;

	// Increase the length of the message
	ctx->length += ctx->curlen * 8;

	// Append the '1' bit
	ctx->buf[ctx->curlen++] = (uint8_t) 0x80;

	// if the length is currently above 56 bytes we append zeros
	// then compress.  Then we can fall back to padding zeros and length
	// encoding like normal.
	if (ctx->curlen > 56) {
		while (ctx->curlen < 64)
			ctx->buf[ctx->curlen++] = (uint8_t) 0;
		sha256_transform(ctx, ctx->buf);
		ctx->curlen = 0;
	}

	// Pad up to 56 bytes of zeroes
	while (ctx->curlen < 56)
		ctx->buf[ctx->curlen++] = (uint8_t) 0;

	// Store length
	SHA256_STORE64H(ctx->length, ctx->buf + 56);
	sha256_transform(ctx, ctx->buf);

	// Copy output
	for (i = 0; i < 8; i++) {
		SHA256_STORE32H(ctx->state[i], digest + (4 * i));
	}
}

static void sha256_init(struct _xtSHA256Context *ctx)
{
	ctx->curlen = 0;
	ctx->length = 0;
	ctx->state[0] = 0x6A09E667UL;
	ctx->state[1] = 0xBB67AE85UL;
	ctx->state[2] = 0x3C6EF372UL;
	ctx->state[3] = 0xA54FF53AUL;
	ctx->state[4] = 0x510E527FUL;
	ctx->state[5] = 0x9B05688CUL;
	ctx->state[6] = 0x1F83D9ABUL;
	ctx->state[7] = 0x5BE0CD19UL;
}

static void sha256_update(struct _xtSHA256Context *restrict ctx, const uint8_t *restrict buf, size_t buflen)
{
	uint32_t n;
	if (ctx->curlen > sizeof ctx->buf)
		return;

	while (buflen > 0) {
		if (ctx->curlen == 0 && buflen >= SHA256_BLOCK_SIZE) {
			sha256_transform(ctx, (uint8_t*) buf);
			ctx->length += SHA256_BLOCK_SIZE * 8;
			buf = (uint8_t*) buf + SHA256_BLOCK_SIZE;
			buflen -= SHA256_BLOCK_SIZE;
		} else {
			n = SHA256_MIN(buflen, (SHA256_BLOCK_SIZE - ctx->curlen));
			memcpy(ctx->buf + ctx->curlen, buf, (size_t) n);
			ctx->curlen += n;
			buf = (uint8_t*) buf + n;
			buflen -= n;
			if (ctx->curlen == SHA256_BLOCK_SIZE) {
				sha256_transform(ctx, ctx->buf);
				ctx->length += 8 * SHA256_BLOCK_SIZE;
				ctx->curlen = 0;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SHA256 END
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SHA512 START
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SHA512_ROR64( value, bits ) (((value) >> (bits)) | ((value) << (64 - (bits))))

#define SHA512_MIN( x, y ) ( ((x)<(y))?(x):(y) )

#define SHA512_LOAD64H( x, y )                                                      \
   { x = (((uint64_t)((y)[0] & 255))<<56)|(((uint64_t)((y)[1] & 255))<<48) | \
         (((uint64_t)((y)[2] & 255))<<40)|(((uint64_t)((y)[3] & 255))<<32) | \
         (((uint64_t)((y)[4] & 255))<<24)|(((uint64_t)((y)[5] & 255))<<16) | \
         (((uint64_t)((y)[6] & 255))<<8)|(((uint64_t)((y)[7] & 255))); }

#define SHA512_STORE64H( x, y )                                                                     \
   { (y)[0] = (uint8_t)(((x)>>56)&255); (y)[1] = (uint8_t)(((x)>>48)&255);     \
     (y)[2] = (uint8_t)(((x)>>40)&255); (y)[3] = (uint8_t)(((x)>>32)&255);     \
     (y)[4] = (uint8_t)(((x)>>24)&255); (y)[5] = (uint8_t)(((x)>>16)&255);     \
     (y)[6] = (uint8_t)(((x)>>8)&255); (y)[7] = (uint8_t)((x)&255); }


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  CONSTANTS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// The K array
static const uint64_t SHA512_K[80] = {
    0x428a2f98d728ae22LLU, 0x7137449123ef65cdLLU, 0xb5c0fbcfec4d3b2fLLU, 0xe9b5dba58189dbbcLLU,
    0x3956c25bf348b538LLU, 0x59f111f1b605d019LLU, 0x923f82a4af194f9bLLU, 0xab1c5ed5da6d8118LLU,
    0xd807aa98a3030242LLU, 0x12835b0145706fbeLLU, 0x243185be4ee4b28cLLU, 0x550c7dc3d5ffb4e2LLU,
    0x72be5d74f27b896fLLU, 0x80deb1fe3b1696b1LLU, 0x9bdc06a725c71235LLU, 0xc19bf174cf692694LLU,
    0xe49b69c19ef14ad2LLU, 0xefbe4786384f25e3LLU, 0x0fc19dc68b8cd5b5LLU, 0x240ca1cc77ac9c65LLU,
    0x2de92c6f592b0275LLU, 0x4a7484aa6ea6e483LLU, 0x5cb0a9dcbd41fbd4LLU, 0x76f988da831153b5LLU,
    0x983e5152ee66dfabLLU, 0xa831c66d2db43210LLU, 0xb00327c898fb213fLLU, 0xbf597fc7beef0ee4LLU,
    0xc6e00bf33da88fc2LLU, 0xd5a79147930aa725LLU, 0x06ca6351e003826fLLU, 0x142929670a0e6e70LLU,
    0x27b70a8546d22ffcLLU, 0x2e1b21385c26c926LLU, 0x4d2c6dfc5ac42aedLLU, 0x53380d139d95b3dfLLU,
    0x650a73548baf63deLLU, 0x766a0abb3c77b2a8LLU, 0x81c2c92e47edaee6LLU, 0x92722c851482353bLLU,
    0xa2bfe8a14cf10364LLU, 0xa81a664bbc423001LLU, 0xc24b8b70d0f89791LLU, 0xc76c51a30654be30LLU,
    0xd192e819d6ef5218LLU, 0xd69906245565a910LLU, 0xf40e35855771202aLLU, 0x106aa07032bbd1b8LLU,
    0x19a4c116b8d2d0c8LLU, 0x1e376c085141ab53LLU, 0x2748774cdf8eeb99LLU, 0x34b0bcb5e19b48a8LLU,
    0x391c0cb3c5c95a63LLU, 0x4ed8aa4ae3418acbLLU, 0x5b9cca4f7763e373LLU, 0x682e6ff3d6b2b8a3LLU,
    0x748f82ee5defb2fcLLU, 0x78a5636f43172f60LLU, 0x84c87814a1f0ab72LLU, 0x8cc702081a6439ecLLU,
    0x90befffa23631e28LLU, 0xa4506cebde82bde9LLU, 0xbef9a3f7b2c67915LLU, 0xc67178f2e372532bLLU,
    0xca273eceea26619cLLU, 0xd186b8c721c0c207LLU, 0xeada7dd6cde0eb1eLLU, 0xf57d4f7fee6ed178LLU,
    0x06f067aa72176fbaLLU, 0x0a637dc5a2c898a6LLU, 0x113f9804bef90daeLLU, 0x1b710b35131c471bLLU,
    0x28db77f523047d84LLU, 0x32caab7b40c72493LLU, 0x3c9ebe0a15c9bebcLLU, 0x431d67c49c100d4cLLU,
    0x4cc5d4becb3e42b6LLU, 0x597f299cfc657e2aLLU, 0x5fcb6fab3ad6faecLLU, 0x6c44198c4a475817LLU
};

#define SHA512_BLOCK_SIZE	128

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  INTERNAL FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Various logical functions
#define SHA512_Ch( x, y, z )     (z ^ (x & (y ^ z)))
#define SHA512_Maj(x, y, z )     (((x | y) & z) | (x & y))
#define SHA512_S( x, n )         SHA512_ROR64( x, n )
#define SHA512_R( x, n )         (((x)&0xFFFFFFFFFFFFFFFFLLU)>>((uint64_t)n))
#define SHA512_Sigma0( x )       (SHA512_S(x, 28) ^ SHA512_S(x, 34) ^ SHA512_S(x, 39))
#define SHA512_Sigma1( x )       (SHA512_S(x, 14) ^ SHA512_S(x, 18) ^ SHA512_S(x, 41))
#define SHA512_Gamma0( x )       (SHA512_S(x, 1) ^ SHA512_S(x, 8) ^ SHA512_R(x, 7))
#define SHA512_Gamma1( x )       (SHA512_S(x, 19) ^ SHA512_S(x, 61) ^ SHA512_R(x, 6))

#define SHA512_Round( a, b, c, d, e, f, g, h, i )       \
     t0 = h + SHA512_Sigma1(e) + SHA512_Ch(e, f, g) + SHA512_K[i] + W[i];   \
     t1 = SHA512_Sigma0(a) + SHA512_Maj(a, b, c);                    \
     d += t0;                                          \
     h  = t0 + t1;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  TransformFunction
//
//  Compress 1024-bits
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void sha512_transform(struct _xtSHA512Context *restrict ctx, uint8_t * restrict buf)
{
	uint64_t	S[8];
	uint64_t	W[80];
	uint64_t	t0;
	uint64_t	t1;
	int			i;

	// Copy state into S
	for (i = 0; i < 8; i++) {
		S[i] = ctx->state[i];
	}

	// Copy the state into 1024-bits into W[0..15]
	for (i = 0; i < 16; i++) {
		SHA512_LOAD64H(W[i], buf + (8 * i));
	}

	// Fill W[16..79]
	for (i = 16; i < 80; i++) {
		W[i] = SHA512_Gamma1(W[i - 2]) + W[i - 7] + SHA512_Gamma0(W[i - 15]) + W[i - 16];
	}

	// Compress
	for (i = 0; i < 80; i += 8) {
		SHA512_Round(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],i+0);
		SHA512_Round(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],i+1);
		SHA512_Round(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],i+2);
		SHA512_Round(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],i+3);
		SHA512_Round(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],i+4);
		SHA512_Round(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],i+5);
		SHA512_Round(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],i+6);
		SHA512_Round(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],i+7);
	}

	// Feedback
	for (i = 0; i < 8; i++) {
		ctx->state[i] = ctx->state[i] + S[i];
	}
}

static void sha512_digest(struct _xtSHA512Context *restrict ctx, uint8_t *restrict digest)
{
	int i;

	if (ctx->curlen >= sizeof ctx->buf)
		return;

	// Increase the length of the message
	ctx->length += ctx->curlen * 8LLU;

	// Append the '1' bit
	ctx->buf[ctx->curlen++] = (uint8_t)0x80;

	// If the length is currently above 112 bytes we append zeros
	// then compress.  Then we can fall back to padding zeros and length
	// encoding like normal.
	if (ctx->curlen > 112) {
		while (ctx->curlen < 128)
			ctx->buf[ctx->curlen++] = (uint8_t)0;
		sha512_transform(ctx, ctx->buf);
		ctx->curlen = 0;
	}

	// Pad up to 120 bytes of zeroes
	// note: that from 112 to 120 is the 64 MSB of the length.  We assume that you won't hash
	// > 2^64 bits of data... :-)
	while (ctx->curlen < 120)
		ctx->buf[ctx->curlen++] = (uint8_t)0;

	// Store length
	SHA512_STORE64H(ctx->length, ctx->buf + 120);
	sha512_transform(ctx, ctx->buf);

	// Copy output
	for (i = 0; i < 8; i++)
		SHA512_STORE64H(ctx->state[i], ((uint8_t*)(digest)) + (8 * i));
}

static void sha512_init(struct _xtSHA512Context *ctx)
{
	ctx->curlen = 0;
	ctx->length = 0;
	ctx->state[0] = 0x6a09e667f3bcc908LLU;
	ctx->state[1] = 0xbb67ae8584caa73bLLU;
	ctx->state[2] = 0x3c6ef372fe94f82bLLU;
	ctx->state[3] = 0xa54ff53a5f1d36f1LLU;
	ctx->state[4] = 0x510e527fade682d1LLU;
	ctx->state[5] = 0x9b05688c2b3e6c1fLLU;
	ctx->state[6] = 0x1f83d9abfb41bd6bLLU;
	ctx->state[7] = 0x5be0cd19137e2179LLU;
}

static void sha512_update(struct _xtSHA512Context *restrict ctx, const uint8_t *restrict buf, size_t buflen)
{
	uint32_t n;

	if (ctx->curlen > sizeof ctx->buf)
		return;

	while (buflen > 0) {
		if (ctx->curlen == 0 && buflen >= SHA512_BLOCK_SIZE) {
			sha512_transform(ctx, (uint8_t*)buf);
			ctx->length += SHA512_BLOCK_SIZE * 8;
			buf = (uint8_t*)buf + SHA512_BLOCK_SIZE;
			buflen -= SHA512_BLOCK_SIZE;
		} else {
			n = SHA512_MIN(buflen, (SHA512_BLOCK_SIZE - ctx->curlen));
			memcpy(ctx->buf + ctx->curlen, buf, (size_t)n);
			ctx->curlen += n;
			buf = (uint8_t*)buf + n;
			buflen -= n;
			if (ctx->curlen == SHA512_BLOCK_SIZE) {
				sha512_transform(ctx, ctx->buf);
				ctx->length += 8 * SHA512_BLOCK_SIZE;
				ctx->curlen = 0;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SHA512 END
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void xtHashDigest(struct xtHash *ctx)
{
	switch (ctx->algorithm) {
	case XT_HASH_MD5 :
		md5_digest(&ctx->digesters.md5, ctx->hash);
		break;
	case XT_HASH_SHA256 :
		sha256_digest(&ctx->digesters.sha256, ctx->hash);
		break;
	case XT_HASH_SHA512 :
		sha512_digest(&ctx->digesters.sha512, ctx->hash);
		break;
	}
}

void xtHashInit(struct xtHash *ctx, enum xtHashAlgorithm algorithm)
{
	ctx->algorithm = algorithm;
	xtHashReset(ctx);
}

void xtHashReset(struct xtHash *ctx)
{
	switch (ctx->algorithm) {
	case XT_HASH_MD5 :
		md5_init(&ctx->digesters.md5);
		ctx->hashSizeInBytes = 16;
		break;
	case XT_HASH_SHA256 :
		sha256_init(&ctx->digesters.sha256);
		ctx->hashSizeInBytes = 32;
		break;
	case XT_HASH_SHA512 :
		sha512_init(&ctx->digesters.sha512);
		ctx->hashSizeInBytes = 64;
		break;
	}
}

void xtHashUpdate(struct xtHash *restrict ctx, const void *restrict buf, size_t buflen)
{
	switch (ctx->algorithm) {
	case XT_HASH_MD5 :
		md5_update(&ctx->digesters.md5, buf, buflen);
		break;
	case XT_HASH_SHA256 :
		sha256_update(&ctx->digesters.sha256, buf, buflen);
		break;
	case XT_HASH_SHA512 :
		sha512_update(&ctx->digesters.sha512, buf, buflen);
		break;
	}
}
