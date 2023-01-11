#ifndef COLT_HG_FN_EXPORTS
#define COLT_HG_FN_EXPORTS

#ifdef _WIN32
/// @brief Makes a function available for JIT
#define COLT_EXPORT_FN __declspec(dllexport)
#else
/// @brief Makes a function available for JIT
#define COLT_EXPORT_FN
#endif

/// @brief Expands to 'extern "C"'
#define COLT_NO_MANGLE extern "C"
/// @brief Macro to simplify exporting of functions
#define COLT_EXPORT COLT_NO_MANGLE COLT_EXPORT_FN

#include <colt/utility/Typedefs.h>

COLT_EXPORT colt::i64 _ColtRand(colt::i64 a, colt::i64 b);

COLT_EXPORT void _ColtPrinti8(colt::i8 a);
COLT_EXPORT void _ColtPrinti16(colt::i16 a);
COLT_EXPORT void _ColtPrinti32(colt::i32 a);
COLT_EXPORT void _ColtPrinti64(colt::i64 a);
COLT_EXPORT void _ColtPrintu8(colt::u8 a);
COLT_EXPORT void _ColtPrintu16(colt::u16 a);
COLT_EXPORT void _ColtPrintu32(colt::u32 a);
COLT_EXPORT void _ColtPrintu64(colt::u64 a);
COLT_EXPORT void _ColtPrintu8HEX(colt::u8 a);
COLT_EXPORT void _ColtPrintu16HEX(colt::u16 a);
COLT_EXPORT void _ColtPrintu32HEX(colt::u32 a);
COLT_EXPORT void _ColtPrintu64HEX(colt::u64 a);
COLT_EXPORT void _ColtPrintbool(bool a);
COLT_EXPORT void _ColtPrintf32(colt::f32 a);
COLT_EXPORT void _ColtPrintf64(colt::f64 a);
COLT_EXPORT void _ColtPrintchar(char a);
COLT_EXPORT void _ColtPrintlstring(colt::lstring a);
COLT_EXPORT void _ColtPrintPTR(colt::PTR<const void> a);

#endif //!COLT_HG_FN_EXPORTS