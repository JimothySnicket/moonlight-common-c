// Lightweight replacement for Limelight-internal.h used only when
// MOONLIGHT_MIC_TEST_MODE is defined (i.e. the mic_tests cmake target).
// Provides the exact set of declarations that Mic.c depends on, without
// pulling in ENet, OpenSSL, or the full platform socket headers.
#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Byte-swap helpers (little-endian host assumed; tests always run on x86-64)
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

#define BE16(x) BSWAP16(x)
#define BE32(x) BSWAP32(x)

// Public API stub — just enough for LI_MIC_MAX_OPUS_BYTES
#define LI_MIC_MAX_OPUS_BYTES 1500

// Wire-format types defined in the real Mic.h
#include "Mic.h"

// Global that the real Connection.c defines; here provided by mic_stubs.c
extern uint32_t SunshineFeatureFlags;

// Control-stream funnel; provided by mic_stubs.c
int sendMicPacketOnControlStream(const void* data, int length);
