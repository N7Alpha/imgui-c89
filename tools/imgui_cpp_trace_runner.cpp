/* The trace runner is intentionally shared with the C89 reference harness.
 * Compiling this translation unit as C++98 verifies that the public headers
 * and replay contract remain usable from a C++ host without a wrapper ABI. */
#include "imgui_c89_trace_runner.c"
