#include <xt/error.h>
#include <xt/crypto.h>
#include <xt/os.h>
#include <xt/string.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils.h"

static struct stats stats;

static void report(uint32_t *data, size_t len)
{
	for (size_t i = 0; i < len; i += 2)
		xtfprintf(stderr,
			"Block: %zu: %08I32x %08I32x.\n",
			i / 2, data[i], data[i + 1]
		);
}

static void blowfish_block09(void)
{
	puts("- BLOWFISH BLOCK TEST");
	struct xtBlowfish ctx;
	char key[] = "AAAAA";
	uint32_t data[10];

	for (unsigned i = 0; i < 10; ++i)
		data[i] = i;

	xtBlowfishInit(&ctx, (uint8_t*)key, 5);
	xtBlowfishEncrypt(&ctx, data, 5);
	xtBlowfishDecrypt(&ctx, data, 1);
	xtBlowfishDecrypt(&ctx, data + 2, 4);
	puts("Should read as 0 - 9.");
	report(data, 10);
}

static void blowfish_encrypt_decrypt(void)
{
	struct xtBlowfish ctx;
	char key[] = "abcdefghijklmnopqrstuvwxyz";
	uint32_t data[] = {0x424c4f57l, 0x46495348l};

	/* first match test */
	xtBlowfishInit(&ctx, (uint8_t*)key, strlen(key));
	xtBlowfishEncrypt(&ctx, data, 1);
	{
		uint32_t match[2] = {0x324ed0fe, 0xf413a203};
		if (data[0] == match[0] && data[1] == match[1])
			PASS("xtBlowfishEncrypt()");
		else {
			FAIL("xtBlowfishEncrypt()");
			xtfprintf(stderr, "Should read as: %I32x %I32x.\n", match[0], match[1]);
			report(data, 2);
		}
	}
	/* second match test */
	xtBlowfishDecrypt(&ctx, data, 1);
	{
		uint32_t match[2] = {0x424c4f57, 0x46495348};
		if (data[0] == match[0] && data[1] == match[1])
			PASS("xtBlowfishDecrypt()");
		else {
			FAIL("xtBlowfishDecrypt()");
			xtfprintf(stderr, "Should read as: %I32x %I32x.\n", match[0], match[1]);
			report(data, 2);
		}
	}
}

static void serpent_init(void)
{
	struct xtSerpent serpent;
	char data[256];
	int error = 0;
	for (unsigned i = 0; i < sizeof data; ++i)
		data[i] = rand();
	error = xtSerpentInit(&serpent, data, 512);
	if (error != XT_EOVERFLOW)
		FAIL("xtSerpentInit() - overflow");
	else
		PASS("xtSerpentInit() - overflow");
	error = xtSerpentInit(&serpent, data, 256);
	if (error)
		FAIL("xtSerpentInit() - default");
	else
		PASS("xtSerpentInit() - default");
}

static void serpent_encrypt_decrypt()
{
	struct xtSerpent serpent;
	const char *str = "You are tearing me apart Lisa!";
	char buf[80], buf2[80];
	char data[256];
	int error = 0;
	/* make valgrind happy */
	memset(buf, 0, sizeof buf);
	memset(buf2, 0, sizeof buf2);
	for (unsigned i = 0; i < sizeof data; ++i)
		data[i] = rand();
	error = xtSerpentInit(&serpent, data, 256);
	if (error)
		FAIL("xtSerpentInit() - encrypt/decrypt");
	else
		PASS("xtSerpentInit() - encrypt/decrypt");
	strcpy(buf, str);
	xtSerpentEncrypt(&serpent, buf2, buf, sizeof buf);
	xtSerpentDecrypt(&serpent, buf, buf2, sizeof buf);
	if (strcmp(str, buf))
		FAIL("xtSerpent - encrypt/decrypt");
	else
		PASS("xtSerpent - encrypt/decrypt");
}

#define LOGROUNDS 10

static int compare_salt(const char *passwd, const char *hash)
{
	char bcrypted[XT_BCRYPT_KEY_LENGTH];
	xtBcrypt(passwd, hash, bcrypted, sizeof bcrypted);
	/*
	 * This does not check the whole string making it vulnerable to a timing attack,
	 * but we don't care about that during test purposes.
	 */
	return strcmp(bcrypted, hash);
}

static void bcrypt_salt(void)
{
	puts("-- BCRYPT SALT TEST");
	const char *passwd = "WhoahD1nnur";
	char salt[XT_BCRYPT_SALT_LENGTH], hash[XT_BCRYPT_KEY_LENGTH];
	uint8_t seed[XT_BCRYPT_MAXSALT];
	for (unsigned i = 0; i < sizeof seed; ++i)
		seed[i] = rand();
	xtBcryptGenSalt(LOGROUNDS, seed, sizeof seed, salt, sizeof salt);
	printf("salt: %s\n", salt);
	xtBcrypt(passwd, salt, hash, sizeof hash);
	printf("hash: %s\n", hash);
	if (!compare_salt("WhoahDinnur", hash))
		FAIL("xtBcrypt() - wrong password");
	else
		PASS("xtBcrypt() - wrong password");
	if (compare_salt("WhoahD1nnur", hash))
		FAIL("xtBcrypt() - good password");
	else
		PASS("xtBcrypt() - good password");
}

int main(void)
{
	stats_init(&stats, "crypto");
	srand(time(NULL));
	puts("-- CRYPTO TEST");
	serpent_init();
	serpent_encrypt_decrypt();
	blowfish_block09();
	blowfish_encrypt_decrypt();
	bcrypt_salt();
	stats_info(&stats);
	return stats_status(&stats);
}
