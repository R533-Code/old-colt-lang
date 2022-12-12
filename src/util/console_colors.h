/** @file console_colors.h
* Defines constant macros representing strings which allows to modify the font and background colors of consoles.
*/

#ifndef HG_COLT_CONSOLE_COLORS
#define HG_COLT_CONSOLE_COLORS

#include <fmt/core.h>
#include <cmd/colt_args.h>

/******************************************************
FOREGROUND COLORS FOR CONSOLE
******************************************************/

const char* CONSOLE_COLORS[] =
{
  "", //EMPTY
  "\x1B[30m",//CONSOLE_FOREGROUND_BLACK
  "\x1B[31m",//CONSOLE_FOREGROUND_RED
  "\x1B[32m",//CONSOLE_FOREGROUND_GREEN
  "\x1B[33m",//CONSOLE_FOREGROUND_YELLOW
  "\x1B[34m",//CONSOLE_FOREGROUND_BLUE
  "\x1B[35m",//CONSOLE_FOREGROUND_MAGENTA
  "\x1B[36m",//CONSOLE_FOREGROUND_CYAN
  "\x1B[37m",//CONSOLE_FOREGROUND_WHITE

  "\x1B[90m",//CONSOLE_FOREGROUND_BRIGHT_BLACK
  "\x1B[91m",//CONSOLE_FOREGROUND_BRIGHT_RED
  "\x1B[92m",//CONSOLE_FOREGROUND_BRIGHT_GREEN
  "\x1B[93m",//CONSOLE_FOREGROUND_BRIGHT_YELLOW
  "\x1B[94m",//CONSOLE_FOREGROUND_BRIGHT_BLUE
  "\x1B[95m",//CONSOLE_FOREGROUND_BRIGHT_MAGENTA
  "\x1B[96m",//CONSOLE_FOREGROUND_BRIGHT_CYAN
  "\x1B[97m",//CONSOLE_FOREGROUND_BRIGHT_WHITE

  "\x1B[40m",//CONSOLE_BACKGROUND_BLACK
  "\x1B[41m",//CONSOLE_BACKGROUND_RED
  "\x1B[42m",//CONSOLE_BACKGROUND_GREEN
  "\x1B[43m",//CONSOLE_BACKGROUND_YELLOW
  "\x1B[44m",//CONSOLE_BACKGROUND_BLUE
  "\x1B[45m",//CONSOLE_BACKGROUND_MAGENTA
  "\x1B[46m",//CONSOLE_BACKGROUND_CYAN
  "\x1B[47m",//CONSOLE_BACKGROUND_WHITE

  "\x1B[100m",//CONSOLE_BACKGROUND_BRIGHT_BLACK
  "\x1B[101m",//CONSOLE_BACKGROUND_BRIGHT_RED
  "\x1B[102m",//CONSOLE_BACKGROUND_BRIGHT_GREEN
  "\x1B[103m",//CONSOLE_BACKGROUND_BRIGHT_YELLOW
  "\x1B[104m",//CONSOLE_BACKGROUND_BRIGHT_BLUE
  "\x1B[105m",//CONSOLE_BACKGROUND_BRIGHT_MAGENTA
  "\x1B[106m",//CONSOLE_BACKGROUND_BRIGHT_CYAN
  "\x1B[107m",//CONSOLE_BACKGROUND_BRIGHT_WHITE

  "\x1B[0m",//CONSOLE_COLOR_RESET
  "\x1B[2m",//CONSOLE_FONT_BOLD
  "\x1B[4m",//CONSOLE_FONT_UNDERLINE
  "\x1B[5m",//CONSOLE_FONT_FLICKER
  "\x1B[7m",//CONSOLE_COLOR_REVERSE
};

namespace colt::io
{
  struct color_t
  {
    u64 index;
  };

  static constexpr color_t BlackF = color_t{ 1 };
  static constexpr color_t RedF = color_t{ 2 };
  static constexpr color_t GreenF = color_t{ 3 };
  static constexpr color_t YellowF = color_t{ 4 };
  static constexpr color_t BlueF = color_t{ 5 };
  static constexpr color_t MagentaF = color_t{ 6 };
  static constexpr color_t CyanF = color_t{ 7 };
  static constexpr color_t WhiteF = color_t{ 8 };
    
  static constexpr color_t BrightBlackF = color_t{ 9 };
  static constexpr color_t BrightRedF = color_t{ 10 };
  static constexpr color_t BrightGreenF = color_t{ 11 };
  static constexpr color_t BrightYellowF = color_t{ 12 };
  static constexpr color_t BrightBlueF = color_t{ 13 };
  static constexpr color_t BrightMagentaF = color_t{ 14 };
  static constexpr color_t BrightCyanF = color_t{ 15 };
  static constexpr color_t BrightWhiteF = color_t{ 16 };

  static constexpr color_t BlackB = color_t{ 17 };
  static constexpr color_t RedB = color_t{ 18 };
  static constexpr color_t GreenB = color_t{ 19 };
  static constexpr color_t YellowB = color_t{ 20 };
  static constexpr color_t BlueB = color_t{ 21 };
  static constexpr color_t MagentaB = color_t{ 22 };
  static constexpr color_t CyanB = color_t{ 23 };
  static constexpr color_t WhiteB = color_t{ 24 };

  static constexpr color_t BrightBlackB = color_t{ 25 };
  static constexpr color_t BrightRedB = color_t{ 26 };
  static constexpr color_t BrightGreenB = color_t{ 27 };
  static constexpr color_t BrightYellowB = color_t{ 28 };
  static constexpr color_t BrightBlueB = color_t{ 29 };
  static constexpr color_t BrightMagentaB = color_t{ 30 };
  static constexpr color_t BrightCyanB = color_t{ 31 };
  static constexpr color_t BrightWhiteB = color_t{ 32 };

  static constexpr color_t Reset = color_t{ 33 };
  static constexpr color_t Bold = color_t{ 34 };
  static constexpr color_t Underline = color_t{ 35 };
  static constexpr color_t Flicker = color_t{ 36 };
  static constexpr color_t SwitchFB = color_t{ 37 };
}

#ifdef COLT_USE_FMT

template<>
/// @brief {fmt} specialization of BinaryOperator
struct fmt::formatter<colt::io::color_t>
{
  template<typename FormatContext>
  /// @brief fmt overload
  /// @tparam FormatContext The context to write 
  /// @param op The BinaryOperator to write
  /// @param ctx The context
  /// @return context
  auto format(const colt::io::color_t& op, FormatContext& ctx)
  {
    return fmt::format_to(ctx.out(), "{}",
      CONSOLE_COLORS[op.index * static_cast<colt::u64>(colt::args::GlobalArguments.colored_output)]);
  }
};

#endif //COLT_USE_FMT

#endif //!HG_CONSOLE_COLORS