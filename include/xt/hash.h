/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

/**
 * @brief Contains functions to hash data with various algorithms.
 *
 * It should be noted that these functions are only supported on little-endian platforms.
 * @file hash.h
 * @author Tom Everaarts
 * @date 2017
 * @copyright LGPL v3.0.
 */

#ifndef _XT_HASH_H
#define _XT_HASH_H

#ifdef __cplusplus
extern "C" {
#endif

// XT headers
#include <xt/_base.h>

// STD headers
#include <stddef.h>
#include <stdint.h>

#define XT_HASH_MD5_HASH_SIZE (128 / 8)
#define XT_HASH_SHA256_HASH_SIZE (256 / 8)
#define XT_HASH_SHA512_HASH_SIZE (512 / 8)
#define XT_HASH_LARGEST_HASH_SIZE XT_HASH_SHA512_HASH_SIZE

/**
 * Compute Cyclic Redundancy Check code from specified data.
 * @param checksum - The initial checksum.
 * @param data - Source data.
 * @param datalen - Source data length.
 * @return The computed code.
 */
uint32_t xtHashCRC32(uint32_t checksum, const void *data, size_t datalen);
/**
 * The MD5 context. It is not to be used externally.
 */
struct _xtMD5Context {
	uint32_t lo;
	uint32_t hi;
	uint32_t a, b, c, d;
	uint8_t buffer[64];
	uint32_t block[16];
};
/**
 * The SHA256 context. It is not to be used externally.
 */
struct _xtSHA256Context {
	uint64_t length;
	uint32_t state[8];
	uint32_t curlen;
	uint8_t buf[64];
};
/**
 * The SHA512 context. It is not to be used externally.
 */
struct _xtSHA512Context {
	uint64_t length;
	uint64_t state[8];
	uint32_t curlen;
	uint8_t buf[128];
};
/**
 * Contains the supported message digest algorithms.
 */
enum xtHashAlgorithm {
	/** MD5 digester */
	XT_HASH_MD5,
	/** SHA256 digester */
	XT_HASH_SHA256,
	/** SHA512 digester */
	XT_HASH_SHA512
};
/**
 * Contains all functionality for various hashing algorithms.
 * You are free to read all data from this struct BUT do not modify anything.
 */
struct xtHash {
	union MessageDigesters {
		struct _xtMD5Context md5;
		struct _xtSHA256Context sha256;
		struct _xtSHA512Context sha512;
	} digesters;
	/**
	 * The digest algorithm that was chosen.
	 */
	enum xtHashAlgorithm algorithm;
	/**
	 * The raw hash bytes. These are updated after each call to xtHashDigest().
	 */
	uint8_t hash[XT_HASH_LARGEST_HASH_SIZE];
	/**
	 * Tells you the length of the raw hash in bytes.
	 */
	unsigned hashSizeInBytes;
};
/**
 * Completes the hash computation by performing final operations such as padding.
 * You can access the resulting hash by reading xtHash.hash.
 */
void xtHashDigest(struct xtHash *ctx);
/**
 * Initiates the chosen digest, making it ready to be used.
 * @param algorithm - The algorithm to use.
 */
void xtHashInit(struct xtHash *ctx, enum xtHashAlgorithm algorithm);
/**
 * Resets the digest for further use.
 */
void xtHashReset(struct xtHash *ctx);
/**
 * Updates the state of the digest.
 */
void xtHashUpdate(struct xtHash *restrict ctx, const void *restrict buf, size_t buflen);

#ifdef __cplusplus
}
#endif

#endif
