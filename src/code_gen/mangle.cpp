/** @file mangle.cpp
* Contains definition of functions declared in 'mangle.h'.
*/

#include "mangle.h"

namespace colt::gen
{
  namespace
  {
    StringView size_t_to_chars(char* begin, char* end, size_t value) noexcept
    {
      auto [ptr, ec] = std::to_chars(begin, end, value);
      assert_true(ec == std::errc{}, "Error converting from int to chars!");
      return { begin, ptr };
    }

    void mangle_arg(String& str, StringView arg, char* buffer, size_t buffer_sz) noexcept
    {
      //Example with pointers:
      // PTR<i8> -> P2i8
      if (arg.begins_with("PTR<"))
      {
        //P for PTR
        str += 'P';
        arg.pop_back(); //pop >
        arg.pop_front_n(4); //pop PTR<
      }
      str += size_t_to_chars(buffer, buffer + buffer_sz, arg.get_size());
      str += arg;
    }

    enum PARAMETER_TYPE
    {
      TYPE, PTR_T, REF_T
    };

    void demangle_arg(String& str, std::pair<StringView, PARAMETER_TYPE> name) noexcept
    {
      if (name.second == PTR_T)
      {
        str += "PTR<";
        str += name.first;
        str += '>';
      }
      else
      {
        str += name.first;
      }

    }
  }

  String mangle(StringView fn_name, StringView ret, lang::TypeNameIter args) noexcept
  {
    //Identifiers begins with _C for colt.
    //The underscore followed by an uppercase is a reserved identifier
    //for C and C++, so no fewer risk of clashing with existing symbols.
    auto str = String{ "_C" };
    //Buffer in which to convert unsigned integers to strings
    //As sizes are represented as size_t, the max value that can be represented
    //is 2^64 = 18_446_744_073_709_551_616 (20 characters max)
    char buffer[20];

    //Write size of name: Print -> _C5Print
    str += size_t_to_chars(buffer, buffer + 20, fn_name.get_size());
    //Then name
    str += fn_name;
    //Mangle return type
    mangle_arg(str, ret, buffer, 20);

    //Do the same thing for each arguments
    for (auto arg : iter::adapter_of<lang::TypeNameIter>(args))
      mangle_arg(str, arg, buffer, 20);

    return str;
  }

  String mangle(PTR<const lang::FnDeclExpr> fn_decl) noexcept
  {
    if (fn_decl->is_extern() || fn_decl->is_main())
      return String{ fn_decl->get_name() };
    return mangle(fn_decl->get_name(), fn_decl->get_return_type()->get_name(), fn_decl->args_typename_iter());
  }

  String demangle(StringView mangled_name) noexcept
  {
    StringView copy = mangled_name;
    if (mangled_name.get_size() < 3 || !mangled_name.begins_with("_C"))
      return String{ copy };
    //Pop "_C"
    mangled_name.pop_front_n(2);

    String result = {};
    //Will store the resulting identifiers
    SmallVector<std::pair<StringView, PARAMETER_TYPE>, 8> identifiers_name;

    while (mangled_name.is_not_empty())
    {
      //Will be a view over the names/typenames character sizes
      auto digits_end = mangled_name.begin();
      while (isdigit(*digits_end))
        ++digits_end;
      if (digits_end == mangled_name.begin())
        return String{ copy };

      PARAMETER_TYPE type = TYPE;
      if (mangled_name.get_front() == 'P') //P for PTR
      {
        type = PTR_T;
        mangled_name.pop_front();
      }

      size_t name_size;
      std::from_chars(mangled_name.begin(), digits_end, name_size);

      //Pop back the digits
      mangled_name.pop_front_n(digits_end - mangled_name.begin());

      identifiers_name.push_back({
        { mangled_name.begin(), mangled_name.begin() + name_size}, type
        });
      //Pop back the identifier
      mangled_name.pop_front_n(name_size);
    }
    if (identifiers_name.get_size() < 2)
      return String{ copy };
    //Write function name
    result += identifiers_name[0].first;
    result += '(';
    //Write arguments type
    if (identifiers_name.get_size() > 2)
      demangle_arg(result, identifiers_name[2]);

    for (size_t i = 3; i < identifiers_name.get_size(); i++)
    {
      result += ", ";
      demangle_arg(result, identifiers_name[i]);
    }
    result += ")->";
    //Write return type
    demangle_arg(result, identifiers_name[1]);

    return result;
  }
}

