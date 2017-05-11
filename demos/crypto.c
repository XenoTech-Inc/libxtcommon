// os_macros.h must be first in order to make it work
#include <xt/error.h>
#include <xt/crypto.h>
#include <xt/os.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void serpent_init(void)
{
	xtConsoleFillLine("-");
	puts("-- SERPENT INIT TEST");
	struct xtCryptoSerpent serpent;
	char data[256];
	int error = 0;
	for (unsigned i = 0; i < sizeof data; ++i)
		data[i] = rand();
	error = xtCryptoSerpentInit(&serpent, data, 512);
	assert(error == XT_EOVERFLOW);
	error = xtCryptoSerpentInit(&serpent, data, 256);
	assert(!error);
}

static void serpent_encrypt_decrypt()
{
	xtConsoleFillLine("-");
	puts("-- ENCRYPT DECRYPT TEST");
	struct xtCryptoSerpent serpent;
	const char *str = "You are tearing me apart Lisa!";
	char buf[80], buf2[80];
	char data[256];
	int error = 0;
	for (unsigned i = 0; i < sizeof data; ++i)
		data[i] = rand();
	error = xtCryptoSerpentInit(&serpent, data, 256);
	assert(!error);
	printf("Encrypt and decrypt \"%s\"\n", str);
	strcpy(buf, str);
	xtCryptoSerpentEncrypt(&serpent, buf2, buf, sizeof buf);
	xtCryptoSerpentDecrypt(&serpent, buf, buf2, sizeof buf);
	assert(!strcmp(str, buf));
}

int main(void)
{
	srand(time(NULL));
	xtConsoleFillLine("-");
	puts("-- CRYPTO TEST");
	serpent_init();
	serpent_encrypt_decrypt();
	xtConsoleFillLine("-");
	puts("All tests have been completed!");
	return EXIT_SUCCESS;
}
