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
#include "blowfish.h"

// STD headers
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

// Otherwise GCC spits out some warnings about snprintf in here
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"

/*
 * This implementation is adaptable to current computing power.
 * You can have up to 2^31 rounds which should be enough for some
 * time to come.
 */

static void _xtBase64Encode(uint8_t *, uint8_t *, uint16_t);
static void _xtBase64Decode(uint8_t *, uint16_t, uint8_t *);

static const char *error = ":";

static const uint8_t Base64Code[] =
"./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

static const uint8_t index_64[128] = {
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 0, 1, 54, 55,
	56, 57, 58, 59, 60, 61, 62, 63, 255, 255,
	255, 255, 255, 255, 255, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
	255, 255, 255, 255, 255, 255, 28, 29, 30,
	31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
	51, 52, 53, 255, 255, 255, 255, 255
};
#define CHAR64(c)  ( (c) > 127 ? 255 : index_64[(c)])

static void _xtBase64Decode(uint8_t *buffer, uint16_t len, uint8_t *data)
{
	uint8_t c1, c2, c3, c4, *bp = buffer, *p = data;
	while (bp < buffer + len) {
		c1 = CHAR64(*p);
		c2 = CHAR64(*(p + 1));

		/* Invalid data */
		if (c1 == 255 || c2 == 255)
			break;

		*bp++ = (c1 << 2) | ((c2 & 0x30) >> 4);
		if (bp >= buffer + len)
			break;

		c3 = CHAR64(*(p + 2));
		if (c3 == 255)
			break;

		*bp++ = ((c2 & 0x0f) << 4) | ((c3 & 0x3c) >> 2);
		if (bp >= buffer + len)
			break;

		c4 = CHAR64(*(p + 3));
		if (c4 == 255)
			break;
		*bp++ = ((c3 & 0x03) << 6) | c4;

		p += 4;
	}
}

void _xtEncodeSalt(char *salt, uint8_t *csalt, uint16_t clen, uint8_t logRounds)
{
	salt[0] = '$';
	salt[1] = XT_BCRYPT_VERSION;
	salt[2] = 'a';
	salt[3] = '$';

	snprintf(salt + 4, 4, "%2.2u$", logRounds);

	_xtBase64Encode((uint8_t*)salt + 7, csalt, clen);
}

void xtBcryptGenSalt(uint8_t log_rounds, uint8_t *seed, char *gsalt)
{
	if (log_rounds < 4)
		log_rounds = 4;
	else if (log_rounds > 31)
		log_rounds = 31;

	_xtEncodeSalt(gsalt, seed, XT_BCRYPT_MAXSALT, log_rounds);
}

/* We handle $Vers$log2(NumRounds)$salt+passwd$
   i.e. $2$04$iwouldntknowwhattosayetKdJ6iFtacBqJdKe6aW7ou */
void xtBcrypt(const char *key, const char *salt, char *encrypted)
{
	struct xtBlowfish state;
	uint32_t rounds, i, k;
	uint8_t key_len, salt_len, logr, minor;
	uint8_t ciphertext[4 * XT_BCRYPT_BLOCKS+1] = "OrpheanBeholderScryDoubt";
	uint8_t csalt[XT_BCRYPT_MAXSALT];
	uint32_t cdata[XT_BCRYPT_BLOCKS];
	int n;

	/* Discard "$" identifier */
	++salt;

	if (*salt > XT_BCRYPT_VERSION) {
		/* How do I handle errors ? Return ':' */
		strcpy(encrypted, error);
		return;
	}

	/* Check for minor versions */
	if (salt[1] != '$')
		switch (salt[1]) {
			case 'a':
				/* 'ab' should not yield the same as 'abab' */
				minor = salt[1];
				salt++;
				break;
			default:
				strcpy(encrypted, error);
				return;
		}
	else
		minor = 0;

	/* Discard version + "$" identifier */
	salt += 2;

	if (salt[2] != '$') {
		/* Out of sync with passwd entry */
		strcpy(encrypted, error);
		return;
	}

	/* Computer power doesn't increase linear, 2^x should be fine */
	n = atoi(salt);
	if (n > 31 || n < 0) {
		strcpy(encrypted, error);
		return;
	}
	logr = (uint8_t)n;
	if ((rounds = (uint32_t)1 << logr) < XT_BCRYPT_MIN_ROUNDS) {
		strcpy(encrypted, error);
		return;
	}

	/* Discard num rounds + "$" identifier */
	salt += 3;

	if (strlen(salt) * 3 / 4 < XT_BCRYPT_MAXSALT) {
		strcpy(encrypted, error);
		return;
	}

	/* We dont want the base64 salt but the raw data */
	_xtBase64Decode(csalt, XT_BCRYPT_MAXSALT, (uint8_t*)salt);
	salt_len = XT_BCRYPT_MAXSALT;
	key_len = strlen(key) + (minor >= 'a' ? 1 : 0);


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

	snprintf(encrypted + i, 4, "%2.2u$", logr);

	_xtBase64Encode((uint8_t*)encrypted + i + 3, csalt, XT_BCRYPT_MAXSALT);
	_xtBase64Encode((uint8_t*)encrypted + strlen(encrypted), ciphertext, 4 * XT_BCRYPT_BLOCKS - 1);
	memset(&state, 0, sizeof state);
	memset(ciphertext, 0, sizeof ciphertext);
	memset(csalt, 0, sizeof csalt);
	memset(cdata, 0, sizeof cdata);
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

static void _xtBase64Encode(uint8_t *buffer, uint8_t *data, uint16_t len)
{
	uint8_t c1, c2, *bp = buffer, *p = data;
	while (p < data + len) {
		c1 = *p++;
		*bp++ = Base64Code[(c1 >> 2)];
		c1 = (c1 & 0x03) << 4;
		if (p >= data + len) {
			*bp++ = Base64Code[c1];
			break;
		}
		c2 = *p++;
		c1 |= (c2 >> 4) & 0x0f;
		*bp++ = Base64Code[c1];
		c1 = (c2 & 0x0f) << 2;
		if (p >= data + len) {
			*bp++ = Base64Code[c1];
			break;
		}
		c2 = *p++;
		c1 |= (c2 >> 6) & 0x03;
		*bp++ = Base64Code[c1];
		*bp++ = Base64Code[c2 & 0x3f];
	}
	*bp = '\0';
}

#pragma GCC diagnostic pop
