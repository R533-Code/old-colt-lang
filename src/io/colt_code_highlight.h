#ifndef HG_COLT_CODE_HIGHLIGHT
#define HG_COLT_CODE_HIGHLIGHT

#include <colt/data_structs/String.h>
#include <lexer/colt_lexer.h>

namespace colt::args
{
  extern bool NoColor;
}

namespace colt::io
{
  struct HighlightCode
  {
    StringView strv;
  };
  
  Color ToColor(lang::Token tkn) noexcept;
}

template<>
struct fmt::formatter<colt::io::HighlightCode>
{
  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

  template<typename FormatContext>
  auto format(const colt::io::HighlightCode& str, FormatContext& ctx)
  {
    using namespace colt;
    using namespace colt::lang;

    if (args::NoColor)
      return fmt::format_to(ctx.out(), "{}", str.strv);

    auto iter = ctx.out();

    Lexer lex = { str.strv, false };

    Token tkn = lex.get_next_token();
    StringView lexeme = lex.get_current_lexeme();
    u64 current_line = lex.get_current_line();
    u64 skipped_spaces = lex.get_skipped_spaces_count();

    while (tkn != TKN_EOF)
    {
      //Lookahead for function calls
      if (tkn == TKN_IDENTIFIER)
      {
        StringView identifier = lex.get_current_lexeme();
        u64 skipped_spaces2 = lex.get_skipped_spaces_count();
        u32 current_line2 = lex.get_current_line();

        tkn = lex.get_next_token();
        lexeme = lex.get_current_lexeme();
        current_line = lex.get_current_line();
        skipped_spaces = lex.get_skipped_spaces_count();
        
        io::Color identifier_color;
        if (tkn == TKN_LEFT_PAREN)
          identifier_color = io::BrightYellowF;
        else
          identifier_color = io::BrightBlueF;

        iter = fmt::format_to(iter, "{: <{}}{:\n<{}}{}{}",
          "", skipped_spaces2,
          "", lex.get_current_line() - current_line2,
          identifier_color, identifier);
        iter = fmt::format_to(iter, "{: <{}}{:\n<{}}{}{}",
          "", skipped_spaces,
          "", lex.get_current_line() - current_line,
          io::ToColor(tkn), lexeme);

        tkn = lex.get_next_token();
        lexeme = lex.get_current_lexeme();
        current_line = lex.get_current_line();
        skipped_spaces = lex.get_skipped_spaces_count();

        continue;
      }
      iter = fmt::format_to(iter, "{: <{}}{:\n<{}}{}{}",
        "", skipped_spaces,
        "", lex.get_current_line() - current_line,
        io::ToColor(tkn), lexeme);

      tkn = lex.get_next_token();
      lexeme = lex.get_current_lexeme();
      current_line = lex.get_current_line();
      skipped_spaces = lex.get_skipped_spaces_count();
    }    
    return fmt::format_to(iter, "{}{: <{}}", io::Reset, "", skipped_spaces);
  }
};

#endif //!HG_COLT_CODE_HIGHLIGHT