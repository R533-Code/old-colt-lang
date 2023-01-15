#ifndef HG_COLT_CODE_HIGHLIGHT
#define HG_COLT_CODE_HIGHLIGHT

#include <colt/data_structs/String.h>
#include <lexer/colt_lexer.h>

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

    auto iter = ctx.out();

    Lexer lex = { str.strv };

    u64 old_offset = 1;
    Token tkn = lex.get_next_token();
    u64 new_offset = lex.get_current_offset();

    while (tkn != TKN_EOF)
    {
      //Lookahead for function calls
      if (tkn == TKN_IDENTIFIER)
      {
        StringView identifier = { str.strv.get_data() + old_offset - 1, str.strv.get_data() + new_offset - 1 };
        old_offset = new_offset;

        tkn = lex.get_next_token();
        new_offset = lex.get_current_offset();
        io::Color identifier_color;
        if (tkn == TKN_LEFT_PAREN)
          identifier_color = io::BrightYellowF;
        else
          identifier_color = io::BrightBlueF;

        iter = fmt::format_to(iter, "{}{}", identifier_color, identifier);
        iter = fmt::format_to(iter, "{}{}", io::ToColor(tkn),
          StringView{ str.strv.get_data() + old_offset - 1, str.strv.get_data() + new_offset - 1 });

        old_offset = new_offset;

        tkn = lex.get_next_token();
        new_offset = lex.get_current_offset();

        continue;
      }
      iter = fmt::format_to(iter, "{}{}", io::ToColor(tkn),
        StringView{ str.strv.get_data() + old_offset - 1, str.strv.get_data() + new_offset - 1 });
      old_offset = new_offset;

      tkn = lex.get_next_token();
      new_offset = lex.get_current_offset();
    }    
    return fmt::format_to(iter, "{}{: <{}}", io::Reset, "", lex.get_skipped_spaces_count());
  }
};

#endif //!HG_COLT_CODE_HIGHLIGHT