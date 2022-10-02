/** @file typedefs.h
* Contains typedefs used throughout the front end.
*/

#ifndef HG_COLT_TYPEDEFS
#define HG_COLT_TYPEDEFS

#include <cstdint>

namespace colt
{
	template<typename T>
	/// @brief Pointer
	/// @tparam T The type pointed to by the pointer
	using PTR = T*;

	/// @brief signed 8-bit integer
	using i8 = int8_t;
	/// @brief signed 16-bit integer
	using i16 = int16_t;
	/// @brief signed 32-bit integer
	using i32 = int32_t;
	/// @brief signed 64-bit integer
	using i64 = int64_t;
	/// @brief unsigned 8-bit integer
	using u8 = uint8_t;
	/// @brief unsigned 16-bit integer
	using u16 = uint16_t;
	/// @brief unsigned 32-bit integer
	using u32 = uint32_t;
	/// @brief unsigned 64-bit integer
	using u64 = uint64_t;
	/// @brief Pointer to characters
	using lstring = const char*;

	/// @brief Undiscriminated union over a byte
	union BYTE
	{
		i8 i8_v;
		u8 u8_v;
		bool bool_v;
		char char_v;

	};

	/// @brief Undiscriminated union over a word (2-bytes)
	union WORD
	{
		i8 i8_v;
		u8 u8_v;
		bool bool_v;
		char char_v;
		BYTE BYTE_v;

		i16 i16_v;
		u16 u16_v;
	};

	/// @brief Undiscriminated union over a double word (4-bytes)
	union DWORD
	{
		i8 i8_v;
		u8 u8_v;
		bool bool_v;
		char char_v;
		BYTE BYTE_v;

		i16 i16_v;
		u16 u16_v;
		WORD WORD_v;

		i32 i32_v;
		u32 u32_v;
		float float_v;
	};

	/// @brief Undiscriminated union over a quadruple word (8-bytes)
	union QWORD
	{
		i8 i8_v;
		u8 u8_v;
		bool bool_v;
		char char_v;
		BYTE BYTE_v;

		i16 i16_v;
		u16 u16_v;
		WORD WORD_v;

		i32 i32_v;
		u32 u32_v;
		float float_v;
		DWORD DWORD_v;

		i64 i64_v;
		u64 u64_v;
		double double_v;
	};
}

#endif //!HG_TYPEDEFS