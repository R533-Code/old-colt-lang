#ifndef HG_COLT_ARGS_PARSER
#define HG_COLT_ARGS_PARSER

#include <array>
#include <utility>
#include <colt/utility/Typedefs.h>
#include <util/colt_macro.h>
#include <colt/data_structs/String.h>
#include <io/colt_print.h>

namespace colt::args
{
  namespace details
  {
    template<typename Array>
    constexpr void comb_sort(Array& array_) noexcept {
      using size_type = typename Array::size_type;
      size_type gap = array_.size();
      bool swapped = false;
      while (gap > size_type{ 1 } || swapped) {
        if (gap > size_type{ 1 }) {
          gap = static_cast<size_type> (gap / 1.247330950103979);
        }
        swapped = false;
        for (size_type i = size_type{ 0 }; gap + i < static_cast<size_type> (array_.size()); ++i) {
          if (array_[i] > array_[i + gap]) {
            auto swap = array_[i];
            array_[i] = array_[i + gap];
            array_[i + gap] = swap;
            swapped = true;
          }
        }
      }
    }

    template <std::size_t N>
    constexpr size_t max_name_size(const std::array<StringView, N>& ar) noexcept
    {
      size_t value = 0;
      for (auto& i : ar)
      {
        if (i.get_size() > value)
          value = i.get_size();
      }
      return value;
    }

    template <std::size_t N>
    constexpr bool is_unique(const std::array<StringView, N>& ar) noexcept
    {
      if constexpr (N == 0)
        return true;

      auto val = ar.front();
      for (size_t i = 1; i < ar.size(); i++)
      {
        if (val == ar[i])
          return false;
        val = ar[i];
      }
      return true;
    }

    constexpr bool is_iequal(StringView a, StringView b) noexcept
    {
      if (a.get_size() != b.get_size())
        return false;
      for (size_t i = 0; i < a.get_size(); i++)
      {
        if (tolower(a[i]) != tolower(b[i]))
          return false;
      }
      return true;
    }
  }

  template<typename Array>
  constexpr Array sort(Array array_) noexcept {
    auto sorted = array_;
    details::comb_sort(sorted);
    return sorted;
  }

  template<typename T, bool positional>
  struct parser;

  template<>
  struct parser<bool, true>
  {
    bool operator()(StringView strv, bool* to_write) const noexcept
    {
      using namespace details;

      if (is_iequal(strv, "f")
        || is_iequal(strv, "false")
        || is_iequal(strv, "off"))
      {
        *to_write = false;
        return true;
      }
      else if (is_iequal(strv, "t")
        || is_iequal(strv, "true")
        || is_iequal(strv, "on"))
      {
        *to_write = true;
        return true;
      }
      return false;
    }
  };

  template<>
  struct parser<bool, false>
  {
    bool operator()(StringView strv, bool* to_write) const noexcept
    {
      using namespace details;
      
      if (strv[0] != '=')
        return false;

      if (is_iequal(strv, "=f")
        || is_iequal(strv, "=false")
        || is_iequal(strv, "=off"))
      {
        *to_write = false;
        return true;
      }
      else if (is_iequal(strv, "=t")
        || is_iequal(strv, "=true")
        || is_iequal(strv, "=on"))
      {
        *to_write = true;
        return true;
      }
      return false;
    }
  };

  template<>
  struct parser<const char*, true>
  {
    bool operator()(StringView strv, const char** to_write) const noexcept
    {
      *to_write = strv.get_data();
      return true;
    }
  };

  template<>
  struct parser<const char*, false>
  {
    bool operator()(StringView strv, const char** to_write) const noexcept
    {
      if (strv.get_front() != '=')
        return false;
      strv.pop_front();
      if (strv.is_empty())
        return false;
      *to_write = strv.get_data();
      return true;
    }
  };

  template<>
  struct parser<StringView, true>
  {
    bool operator()(StringView strv, StringView* to_write) const noexcept
    {
      *to_write = strv;
      return true;
    }
  };

  template<>
  struct parser<StringView, false>
  {
    bool operator()(StringView strv, StringView* to_write) const noexcept
    {
      if (strv.get_front() != '=')
        return false;
      strv.pop_front();
      if (strv.is_empty())
        return false;
      *to_write = strv;
      return true;
    }
  };
}

#endif //!HG_COLT_ARGS_PARSER