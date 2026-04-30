// Test-mode stubs for moonlight-mic unit tests.
// Provides a controllable sendMicPacketOnControlStream and SunshineFeatureFlags.
#include <stdint.h>
#include <string.h>

// ---- SunshineFeatureFlags ----
// The real definition lives in Connection.c; here we provide the test-owned copy.
uint32_t SunshineFeatureFlags = 0;

// ---- sendMicPacketOnControlStream stub ----
// Tests can examine these fields after calling LiSendMicAudioFrame.

// Last buffer passed to sendMicPacketOnControlStream (deep-copied)
static unsigned char _stub_buf[8 + 1500]; // header + max opus
static int           _stub_len = 0;
static int           _stub_call_count = 0;
static int           _stub_return_value = 0; // 0 = success by default

// Reset state before each test case
void stub_reset(void) {
    memset(_stub_buf, 0, sizeof(_stub_buf));
    _stub_len        = 0;
    _stub_call_count = 0;
    _stub_return_value = 0;
}

// Override: stub records the call
int sendMicPacketOnControlStream(const void* data, int length) {
    _stub_call_count++;
    if (length > 0 && length <= (int)sizeof(_stub_buf)) {
        memcpy(_stub_buf, data, (size_t)length);
        _stub_len = length;
    }
    return _stub_return_value;
}

// Accessors for test assertions
const unsigned char* stub_last_buf(void)   { return _stub_buf; }
int                  stub_last_len(void)   { return _stub_len; }
int                  stub_call_count(void) { return _stub_call_count; }
void                 stub_set_return(int v){ _stub_return_value = v; }
