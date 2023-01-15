#include "colt_code_highlight.h"

namespace colt::io
{
  Color colt::io::ToColor(lang::Token tkn) noexcept
  {
    using namespace colt;
    using namespace colt::lang;

    if (tkn <= TKN_RIGHT_SQUARE)
      return io::BrightBlackF;
    else if (tkn == TKN_KEYWORD_MUT)
      return io::BlueF;
    else if (TKN_KEYWORD_VOID <= tkn && tkn <= TKN_KEYWORD_PTR)
      return io::GreenF;
    else if (tkn == TKN_CHAR_L || tkn == TKN_STRING_L)
      return io::YellowF;
    else if (tkn <= TKN_DOUBLE_L)
      return io::BrightGreenF;
    else if (tkn == TKN_BOOL_L
      || (tkn == TKN_KEYWORD_EXTERN || tkn == TKN_KEYWORD_VAR)
      || (TKN_KEYWORD_CONST <= tkn && tkn <= TKN_KEYWORD_BIT_AS))
      return io::BlueF;
    else if ((TKN_KEYWORD_IF <= tkn && tkn <= TKN_KEYWORD_RETURN)
      || (TKN_KEYWORD_FOR <= tkn && tkn <= TKN_KEYWORD_CONTINUE)
      || (TKN_KEYWORD_SWITCH <= tkn && tkn <= TKN_KEYWORD_GOTO))
      return io::BrightMagentaF;
    else if (tkn == TKN_IDENTIFIER)
      return io::BrightBlueF;
    return io::BrightBlackF;
  }
}

