#include <xt/error.h>
#include <xt/crypto.h>
#include <xt/os.h>
#include <xt/string.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void report(uint32_t *data, size_t len)
{
	for (size_t i = 0; i < len; i += 2)
		xtprintf(
			"Block: %zu: %08I32x %08I32x.\n",
			i / 2, data[i], data[i + 1]
		);
}

static void blowfish_block09(void)
{
	xtConsoleFillLine("-");
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
	xtConsoleFillLine("-");
	puts("- BLOWFISH ENCRYPT/DECRYPT TEST");
	struct xtBlowfish ctx;
	char key[] = "abcdefghijklmnopqrstuvwxyz";
	uint32_t data[] = {0x424c4f57l, 0x46495348l};

	/* first match test */
	xtBlowfishInit(&ctx, (uint8_t*)key, strlen(key));
	xtBlowfishEncrypt(&ctx, data, 1);
	{
		uint32_t match[2] = {0x324ed0fe, 0xf413a203};
		xtprintf("\nShould read as: %I32x %I32x.\n", match[0], match[1]);
		report(data, 2);
		assert(data[0] == match[0]);
		assert(data[1] == match[1]);
	}
	/* second match test */
	xtBlowfishDecrypt(&ctx, data, 1);
	{
		uint32_t match[2] = {0x424c4f57, 0x46495348};
		xtprintf("\nShould read as: %I32x %I32x.\n", match[0], match[1]);
		report(data, 2);
		assert(data[0] == match[0]);
		assert(data[1] == match[1]);
	}
}

static void serpent_init(void)
{
	xtConsoleFillLine("-");
	puts("-- SERPENT INIT TEST");
	struct xtSerpent serpent;
	char data[256];
	int error = 0;
	for (unsigned i = 0; i < sizeof data; ++i)
		data[i] = rand();
	error = xtSerpentInit(&serpent, data, 512);
	assert(error == XT_EOVERFLOW);
	error = xtSerpentInit(&serpent, data, 256);
	assert(!error);
}

static void serpent_encrypt_decrypt()
{
	xtConsoleFillLine("-");
	puts("-- ENCRYPT DECRYPT TEST");
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
	assert(!error);
	printf("Encrypt and decrypt \"%s\"\n", str);
	strcpy(buf, str);
	xtSerpentEncrypt(&serpent, buf2, buf, sizeof buf);
	xtSerpentDecrypt(&serpent, buf, buf2, sizeof buf);
	assert(!strcmp(str, buf));
}

int main(void)
{
	srand(time(NULL));
	xtConsoleFillLine("-");
	puts("-- CRYPTO TEST");
	serpent_init();
	serpent_encrypt_decrypt();
	blowfish_block09();
	blowfish_encrypt_decrypt();
	xtConsoleFillLine("-");
	puts("All tests have been completed!");
	return EXIT_SUCCESS;
}
