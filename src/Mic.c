#include "Limelight-internal.h"

// Maximum encoded Opus frame size supported by LiSendMicAudioFrame().
// A 20 ms mono frame at 48 kHz encoded with libopus is well under 1 KB at any
// reasonable bitrate. 1500 bytes is chosen to leave headroom for the 8-byte
// SS_MIC_FRAME_HEADER plus UDP/ENet framing while remaining below the
// path-MTU floor used by moonlight-common-c elsewhere.
//
// LI_MIC_MAX_OPUS_BYTES is defined in Limelight.h for callers.
#define MAX_OPUS_BYTES LI_MIC_MAX_OPUS_BYTES

// Number of PCM samples per 20 ms frame at 48 kHz mono.
#define SAMPLES_PER_FRAME 960

// Dispatch one Opus-encoded mic frame via the existing AES-GCM control tunnel.
// See Limelight.h for parameter and return-code documentation.
int LiSendMicAudioFrame(const unsigned char* opusData, int opusLen, uint16_t seqNumber) {
    unsigned char packet[sizeof(SS_MIC_FRAME_HEADER) + MAX_OPUS_BYTES];
    PSS_MIC_FRAME_HEADER header;

    // --- Input validation ---
    if (opusData == NULL) {
        return -1;
    }
    if (opusLen <= 0 || opusLen > MAX_OPUS_BYTES) {
        return -2;
    }

    // --- Build contiguous packet buffer ---
    header = (PSS_MIC_FRAME_HEADER)packet;
    header->sequenceNumber   = BE16(seqNumber);
    header->opusFrameLength  = BE16((uint16_t)opusLen);
    header->timestampSamples = BE32((uint32_t)seqNumber * SAMPLES_PER_FRAME);

    memcpy(packet + sizeof(SS_MIC_FRAME_HEADER), opusData, (size_t)opusLen);

    // --- Send via control stream with ENET_PACKET_FLAG_UNSEQUENCED ---
    if (sendMicPacketOnControlStream(packet,
                                     (int)(sizeof(SS_MIC_FRAME_HEADER) + (size_t)opusLen)) != 0) {
        return -3;
    }

    return 0;
}
