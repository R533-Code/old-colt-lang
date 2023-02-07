#ifndef COLT_HG_INT
#define COLT_HG_INT

#include "colt_config.h"
#include "colt_macro.h"
#include <cstdint>
#include <type_traits>
#include <numeric>
#include <algorithm>

namespace colt
{
	enum IntOpResult
	{
		OP_VALID,
		OP_OVERFLOW,
		OP_UNDERFLOW
	};

	constexpr const char* IntOpResultToStr(IntOpResult res) noexcept
	{
		switch (res)
		{
		case colt::OP_VALID:
			return "OP_VALID";
		case colt::OP_OVERFLOW:
			return "OP_OVERFLOW";
		case colt::OP_UNDERFLOW:
			return "OP_UNDERFLOW";
		default:
			colt_unreachable("Invalid enum!");
		}
	}

	template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	IntOpResult add(T a, T x, T* result) noexcept
	{
		if constexpr (std::is_signed_v<T>)
		{
			if (x > 0 && a > std::numeric_limits<T>::max() - x)
				return OP_OVERFLOW;
			if (x < 0 && a < std::numeric_limits<T>::min() - x)
				return OP_UNDERFLOW;
			*result = a + x;
			return OP_VALID;
		}
		else
		{
			if constexpr (!std::is_same_v<T, uint64_t>)
			{
				uint64_t res = a;
				res += x;
				*result = res;
				if (res & (std::numeric_limits<uint64_t>::max() + 1))
					return OP_OVERFLOW;
				return OP_VALID;
			}
			else
			{
				uint64_t res = a;
				res += x;
				*result = res;
				if (std::max(a, x) > res)
					return OP_OVERFLOW;
				return OP_VALID;
			}
		}
	}

	template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	IntOpResult sub(T a, T x, T* result) noexcept
	{
		if constexpr (std::is_signed_v<T>)
		{
			if (x < 0 && a > std::numeric_limits<T>::max() + x)
				return OP_OVERFLOW;
			if (x > 0 && a < std::numeric_limits<T>::min() + x)
				return OP_UNDERFLOW;
			*result = a - x;
			return OP_VALID;
		}
		else
		{
			*result = a - x;
			if (x > a)
				return OP_UNDERFLOW;
			return OP_VALID;
		}
	}

	template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	IntOpResult mul(T a, T x, T* result) noexcept
	{
		if constexpr (std::is_signed_v<T>)
		{
			if (a == -1 && x == std::numeric_limits<T>::min())
				return OP_OVERFLOW;
			if (x == -1 && a == std::numeric_limits<T>::min())
				return OP_OVERFLOW;
			if (x > 0 && (a > std::numeric_limits<T>::max() / x || a < std::numeric_limits<T>::min() / x))
				return OP_OVERFLOW;
			if (x < 0 && (a < std::numeric_limits<T>::max() / x || a > std::numeric_limits<T>::min() / x))
				return OP_UNDERFLOW;
			*result = a * x;
			return OP_VALID;
		}
		else
		{
			if constexpr (!std::is_same_v<T, uint64_t>)
			{
				uint64_t res = a;
				res += x;
				*result = res;
				if (res & (std::numeric_limits<uint64_t>::max() & ~static_cast<uint64_t>(std::numeric_limits<T>::max())))
					return OP_OVERFLOW;
				return OP_VALID;
			}
			else
			{
				uint64_t res = a;
				res *= x;
				*result = res;
				if (a != 0 && res / a != x)
					return OP_OVERFLOW;
				return OP_VALID;
			}
		}
	}

	template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	IntOpResult div(T a, T x, T* result) noexcept
	{
		if constexpr (std::is_signed_v<T>)
		{
			if (x == -1 && a == std::numeric_limits<T>::min())
				return OP_OVERFLOW;
			*result = a / x;
			return OP_VALID;
		}
		else
		{
			*result = a / x;
			return OP_VALID;
		}
	}
}

#endif //!COLT_HG_INT