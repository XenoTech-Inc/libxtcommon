#ifndef BCRYPT_H
#define BCRYPT_H

#include "blowfish.h"

#define BCRYPT_VERSION '2'
#define BCRYPT_MAXSALT 16	/* Precomputation is just so nice */
#define BCRYPT_BLOCKS 6		/* Ciphertext blocks */
#define BCRYPT_MINROUNDS 16	/* we have log2(rounds) in salt */

#define _PASSWORD_LEN   128             /* max length, not counting NUL */
#define _SALT_LEN       32              /* max length */

/* bcrypt functions*/
void bcrypt_gensalt(uint8_t log_rounds, uint8_t *seed, char *gsalt);
void bcrypt(const char *key, const char *salt, char *encrypted);
void encode_salt(char *, uint8_t *, uint16_t, uint8_t);
uint32_t bcrypt_get_rounds(const char *);

#endif
