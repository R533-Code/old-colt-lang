/** @file colt_macro.h
* Contains macro helpers used throughout the front-end.
*/

#ifndef HG_COLT_MACRO
#define HG_COLT_MACRO

#include <util/colt_config.h>
#include <util/colt_print.h>

#if defined(__has_builtin)
	#if __has_builtin(__builtin_debugtrap)
		/// @brief Intrinsic trap
		#define colt_intrinsic_dbreak() __builtin_debugtrap()
	#elif __has_builtin(__debugbreak)
		/// @brief Intrinsic trap
		#define colt_intrinsic_dbreak() __debugbreak()
	#endif
#endif

#ifndef colt_intrinsic_dbreak
	#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
		/// @brief Intrinsic trap
		#define colt_intrinsic_dbreak() __debugbreak()
	#else
		/// @brief Intrinsic trap
		#define colt_intrinsic_dbreak() do { (void)std::fgetc(stdin); std::exit(1); } while (0)
	#endif
#endif

#if !defined(COLT_MSVC)
	/// @brief Current function name
	#define COLT_FUNC __FUNCTION__
#else
	/// @brief Current function name
	#define COLT_FUNC __func__
#endif

#ifdef COLT_DEBUG
	/// @brief On Debug configuration, asserts that 'cond' is true
	#define assert_true(cond, err) do { \
			if (!(cond)) \
			{ \
				colt::io::PrintError("Assertion failure: '" #cond "' evaluated to false in function '' on line {}:\nExplanation: " err "\nFile: " __FILE__, COLT_FUNC, __LINE__); \
				colt_intrinsic_dbreak(); \
			} } while (0)
	/// @brief Marks the current branch as unreachable, which aborts if hit on any configuration
	#define colt_unreachable(err) do { colt::io::PrintError("Unreachable branch hit in function '{}' on line {}.\nExplanation: {}\nFile: " __FILE__, COLT_FUNC, __LINE__, err); colt_intrinsic_dbreak(); std::abort(); } while (0)
#else
	/// @brief On Debug configuration, asserts that 'cond' is true
	#define assert_true(cond, err)
	/// @brief Marks the current branch as unreachable, which aborts if hit on any configuration
	#define colt_unreachable(err) std::abort()
#endif

#endif //!HG_COLT_MACRO