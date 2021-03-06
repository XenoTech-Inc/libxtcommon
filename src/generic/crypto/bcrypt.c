/* Copyright 2014-2018 XenoTech. See LICENSE for legal details. */

/*	$OpenBSD: bcrypt.c,v 1.24 2008/04/02 19:54:05 millert Exp $	*/

/*
 * Copyright 1997 Niels Provos <provos@physnet.uni-hamburg.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Niels Provos.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* This password hashing algorithm was designed by David Mazieres
 * <dm@lcs.mit.edu> and works as follows:
 *
 * 1. state := InitState ()
 * 2. state := ExpandKey (state, salt, password) 3.
 * REPEAT rounds:
 *	state := ExpandKey (state, 0, salt)
 *      state := ExpandKey(state, 0, password)
 * 4. ctext := "OrpheanBeholderScryDoubt"
 * 5. REPEAT 64:
 * 	ctext := Encrypt_ECB (state, ctext);
 * 6. RETURN Concatenate (salt, ctext);
 *
 */

// XT headers
#include "blowfish.h"
#include <xt/error.h>
#include <xt/string.h>

// STD headers
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#define BCRYPT_ERROR ":"

/*
 * This implementation is adaptable to current computing power.
 * You can have up to 2^31 rounds which should be enough for some
 * time to come.
 */


/**
 * Print \a seed in base64 human-friendly format.
 * @param salt - The base64 destination for \a seed
 * @param saltlen - Destination buffer size. Must be >= XT_BCRYPT_SALT_LENGTH
 * @param seed - The computed salt
 * @param seedlen - The computed salt length in bytes
 * @param logRounds - The log2 number of rounds used to compute the salt
 */
static void encode_salt(char *salt, size_t saltlen, const uint8_t *seed, uint16_t seedlen, uint8_t logRounds)
{
	salt[0] = '$';
	salt[1] = XT_BCRYPT_VERSION;
	salt[2] = 'a';
	salt[3] = '$';

	snprintf(salt + 4, 5, "%2.2u$", logRounds);

	xtBase64Encode(salt + 7, saltlen - 7, seed, seedlen);
}

int xtBcryptGenSalt(unsigned log_rounds, const uint8_t *seed, size_t seedlen, char *gsalt, size_t saltlen)
{
	if (log_rounds < 4)
		log_rounds = 4;
	else if (log_rounds > 31)
		log_rounds = 31;

	if (saltlen < XT_BCRYPT_SALT_LENGTH)
		return XT_EMSGSIZE;

	encode_salt(
		gsalt, saltlen,
		seed, seedlen, log_rounds
	);

	return 0;
}

/* We handle $Vers$log2(NumRounds)$salt+passwd$
   i.e. $2$04$iwouldntknowwhattosayetKdJ6iFtacBqJdKe6aW7ou */
int xtBcrypt(const char *key, const char *salt, char *encrypted, size_t hashlen)
{
	struct xtBlowfish state;
	uint32_t rounds, i, k;
	uint8_t key_len, salt_len, logr, minor;
	uint8_t ciphertext[4 * XT_BCRYPT_BLOCKS+1] = "OrpheanBeholderScryDoubt";
	uint8_t csalt[XT_BCRYPT_MAXSALT];
	uint32_t cdata[XT_BCRYPT_BLOCKS];
	int n, ret = XT_EUNKNOWN;

	if (hashlen < XT_BCRYPT_KEY_LENGTH) {
		ret = XT_EMSGSIZE;
		goto fail;
	}

	/* Discard "$" identifier */
	++salt;

	if (*salt > XT_BCRYPT_VERSION)
		goto bad;

	/* Check for minor versions */
	if (salt[1] != '$')
		switch (salt[1]) {
		case 'a':
			/* 'ab' should not yield the same as 'abab' */
			minor = salt[1];
			salt++;
			break;
		default:
			goto bad;
		}
	else
		minor = 0;

	/* Discard version + "$" identifier */
	salt += 2;

	if (salt[2] != '$')
		/* Out of sync with passwd entry */
		goto bad;

	/* Computer power doesn't increase linear, 2^x should be fine */
	n = atoi(salt);
	if (n > 31 || n < 0)
		goto bad;

	logr = (uint8_t)n;
	if ((rounds = (uint32_t)1 << logr) < XT_BCRYPT_MIN_ROUNDS)
		goto bad;

	/* Discard num rounds + "$" identifier */
	salt += 3;

	if (strlen(salt) * 3 / 4 < XT_BCRYPT_MAXSALT)
		goto bad;

	/* We dont want the base64 salt but the raw data */
	salt_len = XT_BCRYPT_MAXSALT;
	key_len = strlen(key) + (minor >= 'a' ? 1 : 0);
	ret = xtBase64Decode(csalt, salt_len, salt, strlen(salt) - 1);
	if (ret)
		goto fail;

	/* Setting up S-Boxes and Subkeys */
	_xtBlowfishInitState(&state);
	_xtBlowfishExpandState(&state, csalt, salt_len, (uint8_t*)key, key_len);
	for (k = 0; k < rounds; k++) {
		_xtBlowfishExpand0State(&state, (uint8_t*)key, key_len);
		_xtBlowfishExpand0State(&state, csalt, salt_len);
	}

	/* This can be precomputed later */
	for (unsigned i = 0, j = 0; i < XT_BCRYPT_BLOCKS; i++)
		cdata[i] = _xtBlowfishGrab(ciphertext, 4 * XT_BCRYPT_BLOCKS, &j);

	/* Now do the encryption */
	for (unsigned k = 0; k < 64; k++)
		xtBlowfishEncrypt(&state, cdata, XT_BCRYPT_BLOCKS / 2);

	for (unsigned i = 0; i < XT_BCRYPT_BLOCKS; i++) {
		ciphertext[4 * i + 3] = cdata[i] & 0xff;
		cdata[i] >>= 8;
		ciphertext[4 * i + 2] = cdata[i] & 0xff;
		cdata[i] >>= 8;
		ciphertext[4 * i + 1] = cdata[i] & 0xff;
		cdata[i] >>= 8;
		ciphertext[4 * i + 0] = cdata[i] & 0xff;
	}

	i = 0;
	encrypted[i++] = '$';
	encrypted[i++] = XT_BCRYPT_VERSION;
	if (minor)
		encrypted[i++] = minor;
	encrypted[i++] = '$';

	snprintf(encrypted + i, 5, "%2.2u$", logr);

	ret = xtBase64Encode(
		encrypted + i + 3, hashlen - i - 3, csalt, XT_BCRYPT_MAXSALT
	);
	if (ret)
		goto fail;

	size_t hashsz = strlen(encrypted);
	ret = xtBase64Encode(
		encrypted + hashsz, hashlen - hashsz,
		ciphertext, 4 * XT_BCRYPT_BLOCKS - 1
	);
	if (ret)
		goto fail;

	memset(&state, 0, sizeof state);
	memset(ciphertext, 0, sizeof ciphertext);
	memset(csalt, 0, sizeof csalt);
	memset(cdata, 0, sizeof cdata);

	ret = 0;
fail:
	return ret;
bad:
	strcpy(encrypted, BCRYPT_ERROR);
	return XT_EINVAL;
}

uint32_t xtBcryptGetRounds(const char *hash)
{
	/* skip past the leading "$" */
	if (!hash || *(hash++) != '$') return 0;

	/* skip past version */
	if (!*hash++) return 0;
	if (*hash && *hash != '$') ++hash;
	if (*hash++ != '$') return 0;

	return atoi(hash);
}
