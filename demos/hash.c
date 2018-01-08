#include <xt/hash.h>
#include <xt/os.h>
#include <xt/string.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

static struct stats stats;
/**
 * Used to store the hash as string.
 */
static char asciiBuffer[XT_HASH_LARGEST_HASH_SIZE];

static void crc32_test(const char *input, uint32_t expectedResult)
{
	if (xtHashCRC32(0, input, strlen(input)) == expectedResult)
		PASS("xtHash() - CRC32");
	else
		FAIL("xtHash() - CRC32");
}

static void md5_test(const char *input, const char *expectedResult)
{
	struct xtHash ctx;
	xtHashInit(&ctx, XT_HASH_MD5);
	xtHashUpdate(&ctx, input, strlen(input));
	xtHashDigest(&ctx);
	xtFormatHex(asciiBuffer, sizeof asciiBuffer, ctx.hash, ctx.hashSizeInBytes, 0, false);
	if (strcmp(expectedResult, asciiBuffer) == 0)
		PASS("xtHash() - MD5");
	else
		FAIL("xtHash() - MD5");
}

static void sha256_test(const char *input, const char *expectedResult)
{
	struct xtHash ctx;
	xtHashInit(&ctx, XT_HASH_SHA256);
	xtHashUpdate(&ctx, input, strlen(input));
	xtHashDigest(&ctx);
	xtFormatHex(asciiBuffer, sizeof asciiBuffer, ctx.hash, ctx.hashSizeInBytes, 0, false);
	if (strcmp(expectedResult, asciiBuffer) == 0)
		PASS("xtHash() - SHA256");
	else
		FAIL("xtHash() - SHA256");
}

static void sha512_test(const char *input, const char *expectedResult)
{
	struct xtHash ctx;
	xtHashInit(&ctx, XT_HASH_SHA512);
	xtHashUpdate(&ctx, input, strlen(input));
	xtHashDigest(&ctx);
	xtFormatHex(asciiBuffer, sizeof asciiBuffer, ctx.hash, ctx.hashSizeInBytes, 0, false);
	if (strcmp(expectedResult, asciiBuffer) == 0)
		PASS("xtHash() - SHA512");
	else
		FAIL("xtHash() - SHA512");
}

static void hash_test(void)
{
	const char *input = "The pope uses dope!";
	crc32_test(input, 0xc9c4bd93);
	md5_test(input, "677fa16580aa8e5f717b360030992cfc");
	sha256_test(input, "e3dd550bd2b60a07d8822183eb2941f1e354b71111f679c60f8eeb660cc80995");
	sha512_test(input, "37da4a8b798b9be47e20dee330cee72c08c35758b69bd763529ccd724939e92dd820121537cab790b95b797b135758b27450dc671e72bf2b1fbec63f7d359efc");
}

int main(void)
{
	stats_init(&stats, "hash");
	xtConsoleFillLine("-");
	puts("-- HASH TEST");
	hash_test();
	stats_info(&stats);
	return stats_status(&stats);
}
