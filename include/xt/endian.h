/**
 * @brief Contains portable endian conversion functions.
 * 
 * This header defines all byte swapping functions as macros (Obviously with side effects). 
 * All of them are prefixed with "xt", all of the names are lowercased. 
 * This code will work in C89, but then it will require some work by hand.
 * @file endian.h
 * @author Tom Everaarts
 * @date 2016
 * @copyright XT-License.
 */

#ifndef _XT_ENDIAN_H
#define _XT_ENDIAN_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__GNUC__) || defined(__GNUG__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	#define _XT_BIG_ENDIAN
#elif defined(__clang__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
	#define _XT_BIG_ENDIAN
#else
	/* No C99 available. You must specify the endiannes by hand */
	/* #define _XT_BIG_ENDIAN */
#endif
/* Check if we have C99+ and thus unsigned long long */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
	#define _XT_HAS_LLU
#else
	/* No C99 available. You must specify if you have unsigned long long's by hand */
	/* #define _XT_HAS_LLU */
#endif

/* Generics swaps */
#define _xt_swap16(x)\
	((((x) & 0x00FF) << 8) | \
	(((x) & 0xFF00) >> 8))
#define _xt_swap32(x)\
	((((x) & 0xFF) << 24) | \
	(((x) & 0xFF00) << 8) | \
	(((x) & 0xFF0000) >> 8) | \
	(((x) & 0xFF000000) >> 24))
#ifdef _XT_HAS_LLU
	#define _xt_swap64(x)\
		((((x) & 0xFFLLU) >> 56) | \
		(((x) & 0xFF00LLU) >> 40) | \
		(((x) & 0xFF0000LLU) >> 24) | \
		(((x) & 0xFF000000LLU) >> 8) | \
		(((x) & 0xFF00000000LLU) << 8) | \
		(((x) & 0xFF0000000000LLU) << 24) || \
		(((x) & 0xFF000000000000LLU) << 40) || \
		(((x) & 0xFF00000000000000LLU) << 56))
#endif

/* htons, htonl and htonll */
#ifdef _XT_BIG_ENDIAN
	#define xthtons(x) (x)
	#define xthtonl(x) (x)
	#ifdef _XT_HAS_LLU
		#define xthtonll(x) (x)
	#endif
#else
	#define xthtons(x) _xt_swap16(x)
	#define xthtonl(x) _xt_swap32(x)
	#ifdef _XT_HAS_LLU
		#define xthtonll(x) _xt_swap64(x)
	#endif
#endif

/* ntohs, ntohl and ntohll */
#define xtntohs(x) htons(x)
#define xtntohl(x) htonl(x)
#ifdef _XT_HAS_LLU
	#define xtntohll(x) htonll(x)
#endif

/* htobeXX and beXXtoh family */
#define xthtobe16(x) htons(x)
#define xtbe16toh(x) ntohs(x)
#define xthtobe32(x) htonl(x)
#define xtbe32toh(x) ntohl(x)
#ifdef _XT_HAS_LLU
	#define xthtobe64(x) htonll(x)
	#define xtbe64toh(x) ntohll(x)
#endif

/* htoleXX and leXXtoh family */
#ifdef _XT_BIG_ENDIAN
	#define xthtole16(x) _xt_swap16(x)
	#define xthtole32(x) _xt_swap32(x)
	#ifdef _XT_HAS_LLU
		#define xthtole64(x) _xt_swap64(x)
	#endif
#else
	#define xthtole16(x) (x)
	#define xthtole32(x) (x)
	#ifdef _XT_HAS_LLU
		#define xthtole64(x) (x)
	#endif
#endif

#define xtle16toh(x) htole16(x)
#define xtle32toh(x) htole32(x)
#define xtle64toh(x) htole64(x)

#ifdef __cplusplus
}
#endif

#endif

