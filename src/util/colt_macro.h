/** @file colt_macro.h
* Contains macro helpers used throughout the front-end.
*/

#ifndef HG_COLT_MACRO
#define HG_COLT_MACRO

#include <util/colt_config.h>
#include <util/colt_print.h>

#if defined(__has_builtin)
	#if __has_builtin(__builtin_debugtrap)
		#define colt_intrinsic_dbreak() __builtin_debugtrap()
	#elif __has_builtin(__debugbreak)
		#define colt_intrinsic_dbreak() __debugbreak()
	#endif
#endif

#ifndef colt_intrinsic_dbreak
	#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
		#define colt_intrinsic_dbreak() __debugbreak()
	#else
		#define colt_intrinsic_dbreak() do { (void)std::fgetc(stdin); std::exit(1); } while (0)
	#endif
#endif

#if defined(COLT_MSVC)
	#define COLT_FUNC __FUNCTION__
#else
	#define COLT_FUNC __func__
#endif

#ifdef COLT_DEBUG
	/// @brief On Debug configuration, asserts that 'cond' is true
	#define assert_true(cond, err) do { \
			if (!(cond)) \
			{ \
				colt::io::PrintError("Assertion failure: '" #cond "' evaluated to false in function '"  COLT_FUNC "' on line {}:\nExplanation: " err "\nFile: " __FILE__, __LINE__); \
				colt_intrinsic_dbreak(); \
			} } while (0)
	/// @brief Marks the current branch as unreachable, which aborts if hit on any configuration
	#define colt_unreachable(err) do { colt::io::PrintError("Unreachable branch hit in function '"  COLT_FUNC "' on line {}.\nExplanation: {}.\nFile: " __FILE__, err, __LINE__); colt_intrinsic_dbreak(); std::abort(); } while (0)
#else
	/// @brief On Debug configuration, asserts that 'cond' is true
	#define assert_true(cond, err)
	/// @brief Marks the current branch as unreachable, which aborts if hit on any configuration
	#define colt_unreachable(err) std::abort()
#endif

#endif //!HG_COLT_MACRO