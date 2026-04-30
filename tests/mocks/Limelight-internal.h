// Minimal Limelight-internal.h stub for moonlight-mic test builds.
// Provides only the declarations that Mic.c depends on.
#pragma once

// Pull in the minimal platform definitions (byte-order macros, stdlib)
#include "Platform.h"
// Pull in the minimal public API stub (LI_MIC_MAX_OPUS_BYTES, function decl)
#include "Limelight.h"
// Pull in the wire-format types (SS_MIC_FRAME_HEADER, SS_FF_MIC_INPUT, etc.)
// We source this from the real src/ directory via the test's include path ordering.
#include "Mic.h"

// The only global from Limelight-internal.h that Mic.c reads
extern uint32_t SunshineFeatureFlags;

// The control-stream funnel that Mic.c calls; provided by the test stub TU
int sendMicPacketOnControlStream(const void* data, int length);
