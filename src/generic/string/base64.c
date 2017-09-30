// XT headers
#include <xt/string.h>
#include <xt/error.h>

static const uint8_t base64[] =
"./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

static const uint8_t indexBase64[256] = {
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255,   0,   1,
	 54,  55,  56,  57,  58,  59,  60,  61,
	 62,  63, 255, 255, 255, 255, 255, 255,
	255,   2,   3,   4,   5,   6,   7,   8,
	  9,  10,  11,  12,  13,  14,  15,  16,
	 17,  18,  19,  20,  21,  22,  23,  24,
	 25,  26,  27, 255, 255, 255, 255, 255,
	255,  28,  29,  30,  31,  32,  33,  34,
	 35,  36,  37,  38,  39,  40,  41,  42,
	 43,  44,  45,  46,  47,  48,  49,  50,
	 51,  52,  53, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255,
};

#define CHAR64(c)  (indexBase64[(c)])

// FIXME proper error handling
int xtBase64Decode(void *buf, size_t buflen, const void *data, size_t datalen)
{
	uint8_t *buffer = buf;
	uint8_t c1, c2, c3, c4, *bp = buffer;
	const uint8_t *p = data;

	while (bp < buffer + buflen) {
		if (!datalen)
			return 0;
		c1 = CHAR64(*p);
		c2 = CHAR64(*(p + 1));

		/* Invalid data */
		if (c1 == 255 || c2 == 255)
			return 0;

		*bp++ = (c1 << 2) | ((c2 & 0x30) >> 4);
		if (bp >= buffer + buflen)
			return 0;

		c3 = CHAR64(*(p + 2));
		if (c3 == 255)
			return 0;

		*bp++ = ((c2 & 0x0f) << 4) | ((c3 & 0x3c) >> 2);
		if (bp >= buffer + buflen)
			return 0;

		c4 = CHAR64(*(p + 3));
		if (c4 == 255)
			return 0;

		*bp++ = ((c3 & 0x03) << 6) | c4;
		p += 4;
		if (datalen > 4)
			datalen -= 4;
		else
			datalen = 0;
	}
	return 0;
}

#define PUT(x) \
	do { \
		if (!buflen--) \
			goto fail; \
		*bp++ = x; \
	} while(0)

int xtBase64Encode(void *buf, size_t buflen, const void *data, size_t datalen)
{
	uint8_t *buffer = buf;
	const uint8_t *src = data, *p = src;
	uint8_t c1, c2, *bp = buffer;
	size_t n = buflen;

	while (p < src + datalen) {
		c1 = *p++;
		PUT(base64[c1 >> 2]);
		c1 = (c1 & 0x03) << 4;

		if (p >= src + datalen) {
			PUT(base64[c1]);
			break;
		}

		c2 = *p++;
		c1 |= (c2 >> 4) & 0x0f;
		PUT(base64[c1]);
		c1 = (c2 & 0x0f) << 2;

		if (p >= src + datalen) {
			PUT(base64[c1]);
			break;
		}

		c2 = *p++;
		c1 |= (c2 >> 6) & 0x03;
		PUT(base64[c1]);
		PUT(base64[c2 & 0x3f]);
	}

	*bp = '\0';
	return 0;
fail:
	if (n)
		bp[-1] = '\0';

	return XT_EMSGSIZE;
}
