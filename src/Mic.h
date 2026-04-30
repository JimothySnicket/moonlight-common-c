#pragma once

#include <stdint.h>

// Packet type for client-to-host mic audio frames, sent via the existing
// AES-GCM encrypted control stream on CTRL_CHANNEL_GENERIC.
// Allocated from the Sunshine 0x55xx extension range; 0x5504-0x550F left
// as gap for any Sunshine-side additions before this value.
#define SS_MIC_OPUS_PTYPE 0x5510

#pragma pack(push, 1)

// Fields are big-endian (matches RTP audio convention used host-to-client).
// Immediately follows the inner NVCTL_ENET_PACKET_HEADER_V2 payload and
// precedes the Opus frame bytes.
typedef struct _SS_MIC_FRAME_HEADER {
    uint16_t sequenceNumber;   // BE16; monotonic, wraps at 65535, first packet = 0
    uint16_t opusFrameLength;  // BE16; byte length of the Opus payload following this struct
    uint32_t timestampSamples; // BE32; 48 kHz sample count since first frame this session
                               // (increments by 960 = 48000 * 0.020 per packet)
} SS_MIC_FRAME_HEADER, *PSS_MIC_FRAME_HEADER;

#pragma pack(pop)

_Static_assert(sizeof(SS_MIC_FRAME_HEADER) == 8, "SS_MIC_FRAME_HEADER must be 8 bytes");
