/**
 * @brief Contains cryptographic algorithms and structures with various algorithms.
 * Some cryptographic algorithms require acknowledgement (e.g. serpent). See the
 * implementation code in src/generic/crypto for copyright info.
 *
 * @file crypto.h
 * @author Folkert van Verseveld
 * @date 2017
 * @copyright XT-License.
 */

#ifndef _XT_CRYPTO_H
#define _XT_CRYPTO_H

#ifdef __cplusplus
extern "C" {
#endif

// STD headers
#include <stddef.h>
#include <stdint.h>

/**
 * Serpent cipher.
 */
struct xtSerpent {
	uint32_t l_key[140];
};

/* Schneier specifies a maximum key length of 56 bytes.
 * This ensures that every key bit affects every cipher
 * bit.  However, the subkeys can hold up to 72 bytes.
 * Warning: For normal blowfish encryption only 56 bytes
 * of the key affect all cipherbits.
 */

#define XT_BLOWFISH_N	16			/* Number of Subkeys */
#define XT_BLOWFISH_MAXKEYLEN ((XT_BLOWFISH_N-2)*4)	/* 448 bits */
#define XT_BLOWFISH_MAXUTILIZED ((XT_BLOWFISH_N+2)*4)	/* 576 bits */

/**
 * Blowfish cipher.
 */
struct xtBlowfish {
	/** S-Boxes */
	uint32_t S[4][256];
	/** Subkeys */
	uint32_t P[XT_BLOWFISH_N + 2];
};

#define XT_BCRYPT_VERSION '2'
#define XT_BCRYPT_MAXSALT 16
/** Ciphertext blocks */
#define XT_BCRYPT_BLOCKS 6
/** Minimum number of rounds for computing salt */
#define XT_BCRYPT_MINROUNDS 16

/**
 * Initializes a serpent cipher with the specified key. Note that \a keySize
 * must not exceed 256 bytes.
 * @param ctx - The crypto cipher to initialize.
 * @param key - The encryption/decryption key
 * @param keySize - Key size in bytes (should be at least 16 and less than 256).
 */
int xtSerpentInit(struct xtSerpent *ctx, const void *key, unsigned keySize);
/**
 * Encrypt a block of data. \a dataSize must be a multiple of 16.
 * @param ctx - The encryp cipher to use.
 * @param dest - Location for the encrypted data.
 * @param data - The block of data to be encrypted.
 * @param dataSize - The number of bytes to encrypt.
 */
void xtSerpentEncrypt(struct xtSerpent *ctx, void *restrict dest, const void *restrict data, size_t dataSize);
void xtSerpentDecrypt(struct xtSerpent *ctx, void *restrict dest, const void *restrict data, size_t dataSize);

/**
 * Initializes a blowfish cipher with the specified key. Note that \a keySize
 * must not exceed XT_BLOWFISH_MAXKEYLEN.
 * @param ctx - The blowfish cipher to initialize.
 * @param key - The encryption/decryption key
 * @param keySize - Key size in bytes (should be at least 8 and less than 56).
 */
int xtBlowfishInit(struct xtBlowfish *ctx, const uint8_t *key, uint16_t keySize);
/**
 * Encrypt a block of data in place. \a dataSize must be a multiple of 8.
 * @param ctx - The encrypt cipher to use.
 * @param data - The block of data to be encrypted.
 * @param dataSize - The number of bytes to encrypt.
 */
void xtBlowfishEncrypt(struct xtBlowfish *ctx, uint32_t *data, unsigned dataSize);
/**
 * Decrypt a block of data in place. \a dataSize must be a multiple of 8.
 * @param ctx - The decrypt cipher to use.
 * @param data - The block of data to be decrypted.
 * @param dataSize - The number of bytes to decrypt.
 */
void xtBlowfishDecrypt(struct xtBlowfish *ctx, uint32_t *data, unsigned dataSize);
/**
 * Encrypt a block of data in place using electronic codebook mode. \a dataSize
 * must be a multiple of 8.
 * @param ctx - The encrypt cipher to use.
 * @param data - The block of data to be encrypted.
 * @param dataSize - The number of bytes to encrypt.
 */
void xtBlowfishEncryptECB(struct xtBlowfish *ctx, uint8_t *data, uint32_t dataSize);
/**
 * Decrypt a block of data in place using electronic codebook mode. \a dataSize
 * must be a multiple of 8.
 * @param ctx - The decrypt cipher to use.
 * @param data - The block of data to be decrypted.
 * @param dataSize - The number of bytes to decrypt.
 */
void xtBlowfishDecryptECB(struct xtBlowfish *ctx, uint8_t *data, uint32_t dataSize);
/**
 * Encrypt a block of data in place using cipher block chaining. \a dataSize
 * must be a multiple of 8.
 * @param ctx - The encrypt cipher to use.
 * @param iva - The initialization vector to use.
 * @param data - The block of data to be encrypted.
 * @param dataSize - The number of bytes to encrypt.
 */
void xtBlowfishEncryptCBC(struct xtBlowfish *ctx, uint8_t *iva, uint8_t *data, uint32_t dataSize);
/**
 * Decrypt a block of data in place using cipher block chaining. \a dataSize
 * must be a multiple of 8.
 * @param ctx - The decrypt cipher to use.
 * @param iva - The initialization vector to use.
 * @param data - The block of data to be decrypted.
 * @param dataSize - The number of bytes to decrypt.
 */
void xtBlowfishDecryptCBC(struct xtBlowfish *ctx, uint8_t *iva, uint8_t *data, uint32_t dataSize);

/* xtBcrypt functions*/
void xtBcryptGenSalt(uint8_t log_rounds, uint8_t *seed, char *gsalt);
void xtBcrypt(const char *key, const char *salt, char *encrypted);
void xtEncodeSalt(char *, uint8_t *, uint16_t, uint8_t);
uint32_t xtBcryptGetRounds(const char *);

#ifdef __cplusplus
}
#endif

#endif
