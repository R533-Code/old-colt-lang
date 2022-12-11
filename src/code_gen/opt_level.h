/** @file opt_level.h
* Contains the OptimizationLevel enum.
*/

#ifndef COLT_OPT_LEVEL
#define COLT_OPT_LEVEL

namespace colt::gen
{
	/// @brief The level of optimization to apply.
	/// The value '0' is not used by any of the enums, simplifying
	/// checks in 'colt_args.cpp'.
	enum class OptimizationLevel
	{
		/// @brief Disable nearly all optimization
		O0 = 1,
		/// @brief Optimize quickly without hindering debuggability.
		O1 = 2,
		/// @brief Optimize for fast execution as much as possible without significantly
		/// increasing compile times
		O2 = 3,
		/// @brief Optimize for fast execution as much as possible.
		O3 = 4,
		/// @brief Similar to O2 but tries to optimize for small code size instead of fast execution
		Os = 5,
		/// @brief A very specialized mode that will optimize for code size at any and all
	  /// costs.
		Oz = 6
	};
}

#endif //!COLT_OPT_LEVEL