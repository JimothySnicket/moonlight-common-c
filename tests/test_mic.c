// P4: Tests for Mic.h wire-format constants, LiSendMicAudioFrame, and
// the capability gate (P1/P2/P3 additions to moonlight-common-c).
//
// Harness: plain C, no external test framework.
// Compile via the mic_tests CMake target with -DENABLE_MIC_TESTS=ON.

#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Pull in the real Mic.h for type layout assertions
#include "../src/Mic.h"

// Accessors into mic_stubs.c
extern uint32_t              SunshineFeatureFlags;
void                         stub_reset(void);
const unsigned char*         stub_last_buf(void);
int                          stub_last_len(void);
int                          stub_call_count(void);
void                         stub_set_return(int v);

// The function under test
int LiSendMicAudioFrame(const unsigned char* opusData, int opusLen, uint16_t seqNumber);

// LI_MIC_MAX_OPUS_BYTES is defined in Limelight.h but we need it here for the
// bounds-check test without pulling in the heavy headers.
#define LI_MIC_MAX_OPUS_BYTES 1500
#define SS_FF_MIC_INPUT       0x0100
#define SAMPLES_PER_FRAME     960

// ---- Minimal test harness ----

static int g_pass = 0;
static int g_fail = 0;

#define ASSERT(cond, msg)                                        \
    do {                                                         \
        if (cond) {                                              \
            g_pass++;                                            \
            printf("  PASS: %s\n", msg);                        \
        } else {                                                 \
            g_fail++;                                            \
            printf("  FAIL: %s  (line %d)\n", msg, __LINE__);   \
        }                                                        \
    } while (0)

#define ASSERT_EQ(a, b, msg)                                                     \
    do {                                                                         \
        if ((a) == (b)) {                                                        \
            g_pass++;                                                            \
            printf("  PASS: %s\n", msg);                                        \
        } else {                                                                 \
            g_fail++;                                                            \
            printf("  FAIL: %s  got %d expected %d  (line %d)\n",               \
                   msg, (int)(a), (int)(b), __LINE__);                           \
        }                                                                        \
    } while (0)

// ---- Individual test cases ----

// TC1: Header struct layout — sizeof and offsetof
static void test_header_layout(void) {
    printf("TC1: Header layout\n");

    ASSERT_EQ((int)sizeof(SS_MIC_FRAME_HEADER), 8,
              "sizeof(SS_MIC_FRAME_HEADER) == 8");

    ASSERT_EQ((int)offsetof(SS_MIC_FRAME_HEADER, sequenceNumber), 0,
              "offsetof(sequenceNumber) == 0");

    ASSERT_EQ((int)offsetof(SS_MIC_FRAME_HEADER, opusFrameLength), 2,
              "offsetof(opusFrameLength) == 2");

    ASSERT_EQ((int)offsetof(SS_MIC_FRAME_HEADER, timestampSamples), 4,
              "offsetof(timestampSamples) == 4");
}

// TC2: Packet construction — happy path
// seqNumber=42, opusLen=300, data=0x00,0x01,0x02,...
static void test_packet_construction(void) {
    printf("TC2: Packet construction (happy path)\n");

    stub_reset();
    SunshineFeatureFlags = SS_FF_MIC_INPUT;

    unsigned char opus[300];
    int i;
    for (i = 0; i < 300; i++) {
        opus[i] = (unsigned char)(i & 0xFF);
    }

    int ret = LiSendMicAudioFrame(opus, 300, 42);

    ASSERT_EQ(ret, 0,       "return == 0 (success)");
    ASSERT_EQ(stub_call_count(), 1, "sendMicPacketOnControlStream called once");
    ASSERT_EQ(stub_last_len(), 308, "total packet length == 308 (8 header + 300 opus)");

    const unsigned char* buf = stub_last_buf();

    // --- Header bytes (big-endian on the wire) ---
    // sequenceNumber = 42 = 0x002A  → bytes [0x00, 0x2A]
    ASSERT_EQ(buf[0], 0x00, "header[0] = 0x00 (seqNum BE hi)");
    ASSERT_EQ(buf[1], 0x2A, "header[1] = 0x2A (seqNum BE lo)");

    // opusFrameLength = 300 = 0x012C → bytes [0x01, 0x2C]
    ASSERT_EQ(buf[2], 0x01, "header[2] = 0x01 (opusLen BE hi)");
    ASSERT_EQ(buf[3], 0x2C, "header[3] = 0x2C (opusLen BE lo)");

    // timestampSamples = 42 * 960 = 40320 = 0x00009D80
    // → bytes [0x00, 0x00, 0x9D, 0x80]
    ASSERT_EQ(buf[4], 0x00, "header[4] = 0x00 (timestamp BE byte0)");
    ASSERT_EQ(buf[5], 0x00, "header[5] = 0x00 (timestamp BE byte1)");
    ASSERT_EQ(buf[6], 0x9D, "header[6] = 0x9D (timestamp BE byte2)");
    ASSERT_EQ(buf[7], 0x80, "header[7] = 0x80 (timestamp BE byte3)");

    // --- Payload bytes ---
    int payload_ok = 1;
    for (i = 0; i < 300; i++) {
        if (buf[8 + i] != (unsigned char)(i & 0xFF)) {
            payload_ok = 0;
            break;
        }
    }
    ASSERT(payload_ok, "payload matches input opus data byte-for-byte");
}

// TC3: Input validation — null data
static void test_null_data(void) {
    printf("TC3: Input validation — null data\n");

    stub_reset();
    SunshineFeatureFlags = SS_FF_MIC_INPUT;

    int ret = LiSendMicAudioFrame(NULL, 100, 0);

    ASSERT_EQ(ret, -1,              "null data → return -1");
    ASSERT_EQ(stub_call_count(), 0, "sendMicPacketOnControlStream NOT called");
}

// TC4: Input validation — zero length
static void test_zero_length(void) {
    printf("TC4: Input validation — zero length\n");

    stub_reset();
    SunshineFeatureFlags = SS_FF_MIC_INPUT;

    unsigned char buf[16] = {0};
    int ret = LiSendMicAudioFrame(buf, 0, 0);

    ASSERT_EQ(ret, -2,              "zero length → return -2");
    ASSERT_EQ(stub_call_count(), 0, "sendMicPacketOnControlStream NOT called");
}

// TC5: Input validation — oversized opus frame
static void test_oversized_length(void) {
    printf("TC5: Input validation — oversized length\n");

    stub_reset();
    SunshineFeatureFlags = SS_FF_MIC_INPUT;

    unsigned char buf[16] = {0};
    int ret = LiSendMicAudioFrame(buf, LI_MIC_MAX_OPUS_BYTES + 1, 0);

    ASSERT_EQ(ret, -2,              "oversized length → return -2");
    ASSERT_EQ(stub_call_count(), 0, "sendMicPacketOnControlStream NOT called");
}

// TC6: Capability gate — host NOT advertising SS_FF_MIC_INPUT
static void test_gate_no_advertise(void) {
    printf("TC6: Capability gate — host NOT advertising SS_FF_MIC_INPUT\n");

    stub_reset();
    SunshineFeatureFlags = 0; // host does NOT have the mic flag

    unsigned char opus[100];
    memset(opus, 0xAB, sizeof(opus));

    int ret = LiSendMicAudioFrame(opus, (int)sizeof(opus), 1);

    ASSERT_EQ(ret, 0,               "silent success (no-op) → return 0");
    ASSERT_EQ(stub_call_count(), 0, "sendMicPacketOnControlStream NOT called");
}

// TC7: Capability gate — host advertising SS_FF_MIC_INPUT
static void test_gate_with_advertise(void) {
    printf("TC7: Capability gate — host advertising SS_FF_MIC_INPUT\n");

    stub_reset();
    SunshineFeatureFlags = SS_FF_MIC_INPUT;

    unsigned char opus[100];
    memset(opus, 0xAB, sizeof(opus));

    int ret = LiSendMicAudioFrame(opus, (int)sizeof(opus), 1);

    ASSERT_EQ(ret, 0,               "success → return 0");
    ASSERT_EQ(stub_call_count(), 1, "sendMicPacketOnControlStream WAS called");
    ASSERT_EQ(stub_last_len(), 108, "packet = 8 header + 100 opus = 108 bytes");
}

// ---- main ----

int main(void) {
    printf("=== moonlight-common-c mic tests ===\n\n");

    test_header_layout();
    test_packet_construction();
    test_null_data();
    test_zero_length();
    test_oversized_length();
    test_gate_no_advertise();
    test_gate_with_advertise();

    printf("\n--- Results: %d passed, %d failed ---\n", g_pass, g_fail);

    return (g_fail == 0) ? 0 : 1;
}
