// Minimal Limelight.h stub for moonlight-mic test builds.
// Provides only what Mic.c depends on from the real header.
#pragma once
#include <stdint.h>
#include <stdbool.h>

// Maximum encoded Opus frame size accepted by LiSendMicAudioFrame().
// Must stay in sync with the value in the real Limelight.h.
#define LI_MIC_MAX_OPUS_BYTES 1500

int LiSendMicAudioFrame(const unsigned char* opusData, int opusLen, uint16_t seqNumber);
