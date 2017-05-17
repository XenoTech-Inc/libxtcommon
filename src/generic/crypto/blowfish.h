#ifndef _XT_BLOWFISH_H
#define _XT_BLOWFISH_H

#include <xt/crypto.h>

/** Minimum number of rounds for computing salt */
#define XT_BCRYPT_MIN_ROUNDS 16

/* Internal use only */
void _xtBlowfishEncipher(struct xtBlowfish *, uint32_t *, uint32_t *);
void _xtBlowfishDecipher(struct xtBlowfish *, uint32_t *, uint32_t *);
void _xtBlowfishInitState(struct xtBlowfish *);
void _xtBlowfishExpand0State(struct xtBlowfish *, const uint8_t *, uint16_t);
void _xtBlowfishExpandState(struct xtBlowfish *, const uint8_t *, uint16_t, const uint8_t *, uint16_t);

/*
 * Converts uint8_t to uint32_t
 * Don't move this: it is also used by xtBcrypt
 */
static inline uint32_t _xtBlowfishGrab(const uint8_t *data, unsigned n, unsigned *pos)
{
	unsigned j = *pos;
	uint32_t word = 0;

	for (uint8_t i = 0; i < 4; ++i) {
		word = (word << 8) | data[j];
		j = (j + 1) % n;
	}

	*pos = j;
	return word;
}

/**
 * Print \a csalt in base64 human-friendly format.
 * @param salt - The base64 destination for \a csalt
 * @param csalt - The computed salt
 * @param clen - The computed salt length in bytes
 * @param logRounds - The log2 number of rounds used to compute the salt
 */
void _xtEncodeSalt(char *salt, uint8_t *csalt, uint16_t clen, uint8_t logRounds);

#endif
