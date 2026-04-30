// Minimal Platform.h stub for moonlight-mic test builds.
// Provides byte-order macros that Mic.c depends on.
#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Byte-swap helpers — replicate what the real Platform.h provides on LE hosts
// (Windows/Linux x86-64 are always little-endian).
#ifdef _MSC_VER
#pragma intrinsic(_byteswap_ushort)
#define BSWAP16(x) _byteswap_ushort(x)
#pragma intrinsic(_byteswap_ulong)
#define BSWAP32(x) _byteswap_ulong(x)
#elif (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
#define BSWAP16(x) __builtin_bswap16(x)
#define BSWAP32(x) __builtin_bswap32(x)
#else
static inline uint16_t BSWAP16(uint16_t x) { return (uint16_t)((x << 8) | (x >> 8)); }
static inline uint32_t BSWAP32(uint32_t x) {
    return ((x & 0xFF000000u) >> 24) | ((x & 0x00FF0000u) >> 8)
         | ((x & 0x0000FF00u) << 8)  | ((x & 0x000000FFu) << 24);
}
#endif

// Big-endian macros for a little-endian host (always the case for test targets)
#define BE16(x) BSWAP16(x)
#define BE32(x) BSWAP32(x)

// No-op logging macro (Mic.c doesn't call Limelog, but transitively-included
// headers reference it; suppress it cleanly)
#define Limelog(s, ...) ((void)0)

// LC_ASSERT not used in Mic.c but referenced by some internal macros
#define LC_ASSERT(x) ((void)0)
#define LC_ASSERT_VT(x) ((void)0)
