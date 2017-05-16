#ifndef BLOWFISH_H
#define BLOWFISH_H

#include <xt/crypto.h>

/* Internal use only */
void _xtBlowfishEncipher(struct xtBlowfish *, uint32_t *, uint32_t *);
void _xtBlowfishDecipher(struct xtBlowfish *, uint32_t *, uint32_t *);
void _xtBlowfishInitState(struct xtBlowfish *);
void _xtBlowfishExpand0State(struct xtBlowfish *, const uint8_t *, uint16_t);
void _xtBlowfishExpandState(struct xtBlowfish *, const uint8_t *, uint16_t, const uint8_t *, uint16_t);

/*
 * Converts uint8_t to uint32_t
 * Don't move this: it is also used by bcrypt
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


#endif
