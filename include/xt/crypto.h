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

#ifdef __cplusplus
}
#endif

#endif
