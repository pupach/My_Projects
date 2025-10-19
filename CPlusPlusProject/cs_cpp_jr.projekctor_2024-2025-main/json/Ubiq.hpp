#include <array>
#include <charconv>
#include <cstring>
#include <format>
#include <iterator>
#include <string>
#include <vector>
#include "details.hpp"

const std::size_t MAX_SIZE_NAME = 10;

#define BAD_CONVERSION '#'
#define GOOD_CONVERSION '!'

template<typename _Tp, size_t _Nm>
constexpr std::array<std::remove_cv_t<_Tp>, MAX_SIZE_ENUM> to_array(_Tp (&__a)[_Nm])
    noexcept(std::is_nothrow_constructible_v<_Tp, _Tp&>) {

  std::array<std::remove_cv_t<_Tp>, MAX_SIZE_ENUM> to_ret = {};
  auto arr = std::to_array(__a);
  std::copy(arr.begin(), arr.end(), to_ret.begin());

  return to_ret;
}

constexpr std::string extract_value_name(std::array<char, MAX_SIZE_ENUM> s) {
  bool find = false;
  std::string str = "";
  for(std::size_t i = 1; i < MAX_SIZE_ENUM; i++) {
    if(s[i] != '\0') {
      if((s[i] == ':') && (s[i - 1] == ':')) {
        find = true;
      }
      else if(find) {
        if(s[i] == ')') {
          return "\"" + str + "\"";
        }
        str += s[i];
      }
    }
  }
  return "\"" + str + "\"";
}

constexpr std::pair<const char *, std::errc> from_chars(const char * begin, std::size_t size){
  for(std::size_t i = 0; i < size; i++) {
    if((0 > (*(begin + i) - '0')) || (9 < (*(begin + i) - '0'))) return {nullptr, std::errc::io_error};
  }
  return {nullptr, std::errc()};
}


constexpr std::array<char, MAX_SIZE_ENUM> extract_value(const char *s) { //TODO c std::array работать неудобно, а эту функцию непонятно как переписать на std::string_view
  std::size_t N = 0;
  while(s[N] != '\0') N++;
  std::size_t size = 0;
  char buffer[MAX_SIZE_ENUM] = {};
  constexpr char key[] = "I = ";
  for (std::size_t i = 0; i + sizeof(key) - 1 < N; ++i)
  {
    bool match = true;
    for (std::size_t k = 0; k < sizeof(key) - 1; ++k) {
      if (s[i + k] != key[k]) {
        match = false;
        break;
      }
    }

    if (match)
    {
      while((s[i + sizeof(key) + size - 1] != ';') and (s[i + sizeof(key) + size - 1] != ']')) {
        buffer[size + 1] = s[i + sizeof(key) - 1 + size];
        size++;
      }

      int result{};
      auto [ptr, ec] = from_chars(buffer + 1, size);
      if(ec == std::errc()) {
        buffer[0] = GOOD_CONVERSION;
        return to_array(buffer);
      }

      buffer[0] = BAD_CONVERSION;
      return to_array(buffer);
    }
  }
  return {};
}


template<typename T, T I>
constexpr const char * Available()
{
  return __PRETTY_FUNCTION__;
}


template<std::size_t I = 0>
struct Ubiq {
  std::array<std::array<char, MAX_SIZE_ENUM>, MAX_SIZE_NAME + 1> & data;

  explicit constexpr Ubiq(std::array<std::array<char, MAX_SIZE_ENUM>, MAX_SIZE_NAME + 1> & to_ret) : data(to_ret) {}

  template <typename T>
  constexpr operator T() const noexcept {
    data[I] = extract_value(Available<T,static_cast<T>(I)>());
    if constexpr (I != MAX_SIZE_NAME) {
      Ubiq<I + 1> l(data);
      return (T)l;
    }
    else {
      return T();
    }
  }
};

template <std::size_t>
struct Univers {
  template <typename T>
  constexpr operator T() const noexcept;
};

template <typename T, typename... Args>
concept AggregateConstructibleFrom = requires(Args&&... args) {
  T{std::forward<Args>(args)...};
};


template <typename T, std::size_t... I>
constexpr std::size_t GetFieldsCountImpl(std::index_sequence<I...>) {
  return sizeof...(I) - 1;
}

template <typename T, std::size_t... I>
  requires AggregateConstructibleFrom<T, Univers<I>...>
constexpr std::size_t GetFieldsCountImpl(std::index_sequence<I...>) {
  return GetFieldsCountImpl<T>(std::index_sequence<I..., sizeof...(I)>());
}

template <typename T>
constexpr std::size_t GetFieldsCount() {
  return GetFieldsCountImpl<T>(std::index_sequence<16>());
}

template <auto I>
constexpr const char* GetValue() {
  return __PRETTY_FUNCTION__;
}

template<typename T>
const T struc{};

template <typename T>
consteval std::array<const char *, 1> GetNameFieldsTupleImpl1() {
  auto& [a1] = struc<T>;
  return {GetValue<&a1>()};
}

template <typename T>
consteval std::array<const char *, 2> GetNameFieldsTupleImpl2() {
  auto& [a1, b1] = struc<T>;
  return {GetValue<&a1>(), GetValue<&b1>()};
}

template <typename T>
consteval std::array<const char *, 3> GetNameFieldsTupleImpl3() {
  auto& [a1, b1, c1] = struc<T>;
  return {GetValue<&a1>(), GetValue<&b1>(), GetValue<&c1>()};
}

template <typename T>
consteval std::array<const char *, 4> GetNameFieldsTupleImpl4() {
  auto& [a1, b1, c1, d1] = struc<T>;
  return {GetValue<&a1>(), GetValue<&b1>(), GetValue<&c1>(), GetValue<&d1>()};
}

template <typename T>
consteval std::array<const char *, 5> GetNameFieldsTupleImpl5() {
  auto& [a1, b1, c1, d1, a2] = struc<T>;
  return {GetValue<&a1>(), GetValue<&b1>(), GetValue<&c1>(), GetValue<&d1>(), GetValue<&a2>()};
}

template <typename T>
consteval std::array<const char *, 6> GetNameFieldsTupleImpl6() {
  auto& [a1, b1, c1, d1, a2, a3] = struc<T>;
  return {GetValue<&a1>(), GetValue<&b1>(), GetValue<&c1>(), GetValue<&d1>(), GetValue<&a2>(), GetValue<&a3>()};
}

template <typename T>
consteval std::array<const char *, 7> GetNameFieldsTupleImpl7() {
  auto& [a1, b1, c1, d1, a2, a3, a4] = struc<T>;
  return {GetValue<&a1>(), GetValue<&b1>(), GetValue<&c1>(), GetValue<&d1>(), GetValue<&a2>(), GetValue<&a3>(), GetValue<&a4>()};
}

template <typename T>
consteval std::array<const char *, 8> GetNameFieldsTupleImpl8() {
  auto& [a1, b1, c1, d1, a2, a3, a4, a5] = struc<T>;
  return {GetValue<&a1>(), GetValue<&b1>(), GetValue<&c1>(), GetValue<&d1>(), GetValue<&a2>(), GetValue<&a3>(), GetValue<&a4>(), GetValue<&a5>()};
}

template <typename T>
consteval std::array<const char *, 9> GetNameFieldsTupleImpl9() {
  auto& [a1, b1, c1, d1, a2, a3, a4, a5, a6] = struc<T>;
  return {GetValue<&a1>(), GetValue<&b1>(), GetValue<&c1>(), GetValue<&d1>(), GetValue<&a2>(), GetValue<&a3>(), GetValue<&a4>(), GetValue<&a5>(), GetValue<&a6>()};
}

template <typename T>
consteval std::array<const char *, 10> GetNameFieldsTupleImpl10() {
  auto& [a1, b1, c1, d1, a2, a3, a4, a5, a6, a7] = struc<T>;
  return {GetValue<&a1>(), GetValue<&b1>(), GetValue<&c1>(), GetValue<&d1>(), GetValue<&a2>(), GetValue<&a3>(), GetValue<&a4>(), GetValue<&a5>(), GetValue<&a6>(), GetValue<&a7>()};
}

template <typename T>
consteval std::array<const char *, 11> GetNameFieldsTupleImpl11() {
  auto& [a1, b1, c1, d1, a2, a3, a4, a5, a6, a7, a8] = struc<T>;
  return {GetValue<&a1>(), GetValue<&b1>(), GetValue<&c1>(), GetValue<&d1>(), GetValue<&a2>(), GetValue<&a3>(), GetValue<&a4>(), GetValue<&a5>(), GetValue<&a6>(), GetValue<&a7>(), GetValue<&a8>()};
}

template <typename T>
consteval std::array<const char *, 12> GetNameFieldsTupleImpl12() {
  auto& [a1, b1, c1, d1, a2, a3, a4, a5, a6, a7, a8, a9] = struc<T>;
  return {GetValue<&a1>(), GetValue<&b1>(), GetValue<&c1>(), GetValue<&d1>(), GetValue<&a2>(), GetValue<&a3>(), GetValue<&a4>(), GetValue<&a5>(), GetValue<&a6>(), GetValue<&a7>(), GetValue<&a8>(), GetValue<&a9>()};
}

template <typename T>
consteval std::array<const char *, 13> GetNameFieldsTupleImpl13() {
  auto& [a1, b1, c1, d1, a2, a3, a4, a5, a6, a7, a8, a9, a10] = struc<T>;
  return {GetValue<&a1>(), GetValue<&b1>(), GetValue<&c1>(), GetValue<&d1>(), GetValue<&a2>(), GetValue<&a3>(), GetValue<&a4>(), GetValue<&a5>(), GetValue<&a6>(), GetValue<&a7>(), GetValue<&a8>(), GetValue<&a9>(), GetValue<&a10>()};
}

template <typename T>
consteval std::array<const char *, 14> GetNameFieldsTupleImpl14() {
  auto& [a1, b1, c1, d1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11] = struc<T>;
  return {GetValue<&a1>(), GetValue<&b1>(), GetValue<&c1>(), GetValue<&d1>(), GetValue<&a2>(), GetValue<&a3>(), GetValue<&a4>(), GetValue<&a5>(), GetValue<&a6>(), GetValue<&a7>(), GetValue<&a8>(), GetValue<&a9>(), GetValue<&a10>(), GetValue<&a11>()};
}

template<typename T, std::size_t I>
struct Numb {
  static consteval std::array<const char *, I> get() {}
};

template<typename T>
struct Numb<T, 1> {
  static consteval std::array<const char *, 1> get() {
    return GetNameFieldsTupleImpl1<T>();
  }
};

template<typename T>
struct Numb<T, 2> {
  static consteval std::array<const char *, 2> get() {
    return GetNameFieldsTupleImpl2<T>();
  }
};

template<typename T>
struct Numb<T, 3> {
  static consteval std::array<const char *, 3> get() {
    return GetNameFieldsTupleImpl3<T>();
  }
};

template<typename T>
struct Numb<T, 4> {
  static consteval std::array<const char *, 4> get() {
    return GetNameFieldsTupleImpl4<T>();
  }
};

template<typename T>
struct Numb<T, 5> {
  static consteval std::array<const char *, 5> get() {
    return GetNameFieldsTupleImpl5<T>();
  }
};

template<typename T>
struct Numb<T, 6> {
  static consteval std::array<const char *, 6> get() {
    return GetNameFieldsTupleImpl6<T>();
  }
};

template<typename T>
struct Numb<T, 7> {
  static consteval std::array<const char *, 7> get() {
    return GetNameFieldsTupleImpl7<T>();
  }
};

template<typename T>
struct Numb<T, 8> {
  static consteval std::array<const char *, 8> get() {
    return GetNameFieldsTupleImpl8<T>();
  }
};

template<typename T>
struct Numb<T, 9> {
  static consteval std::array<const char *, 9> get() {
    return GetNameFieldsTupleImpl9<T>();
  }
};

template<typename T>
struct Numb<T, 10> {
  static consteval std::array<const char *, 10> get() {
    return GetNameFieldsTupleImpl10<T>();
  }
};

template<typename T>
struct Numb<T, 11> {
  static consteval std::array<const char *, 11> get() {
    return GetNameFieldsTupleImpl11<T>();
  }
};

template<typename T>
struct Numb<T, 12> {
  static consteval std::array<const char *, 12> get() {
    return GetNameFieldsTupleImpl12<T>();
  }
};

template<typename T>
struct Numb<T, 13> {
  static consteval std::array<const char *, 13> get() {
    return GetNameFieldsTupleImpl13<T>();
  }
};

template<typename T>
struct Numb<T, 14> {
  static consteval std::array<const char *, 14> get() {
    return GetNameFieldsTupleImpl14<T>();
  }
};
template<typename T>
struct Mem {
  static constexpr auto field = Numb<T, GetFieldsCount<T>()>::get();
};

template <std::size_t I>
struct size_t_ {};


