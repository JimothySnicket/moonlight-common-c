// Compilation shim: compiles Mic.c from the tests/ directory so that
// #include "Limelight-internal.h" resolves to tests/mocks/Limelight-internal.h
// (the lightweight test stub) rather than the real src/Limelight-internal.h.
//
// MSVC and GCC/Clang resolve quoted includes relative to the including file's
// directory first, so placing this file in tests/ makes "Limelight-internal.h"
// resolve via tests/mocks/ (which is on the include path before src/).
#include "../src/Mic.c"
