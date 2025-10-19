#pragma once
#include <string>
#include <variant>
#include <tuple>
#include <array>

const std::size_t MAX_SIZE_ENUM = 200;

namespace details {
  constexpr int max_size = 100;

  using std::to_string;

  std::string to_string(std::string a) { return "\"" + a + "\""; }

  std::string to_string(bool a) {
    if (a) {
      return "true";
    }
    return "false";
  }

  std::string from_array_to_string(std::array<char, MAX_SIZE_ENUM> a) {
    std::string to_ret = "";
    for(std::size_t i = 1; i < MAX_SIZE_ENUM; i++) {
      if(a[i] != '\0') {
        to_ret += a[i];
      }
      else {
        break;
      }
    }
    return to_ret;
  }

  using std::get;

  using std::holds_alternative;

  template <std::size_t I, typename T, typename... U>
  struct TypeByIndexHelper {
    using type = TypeByIndexHelper<I - 1, U...>::type;
  };

  template <typename T, typename... U>
  struct TypeByIndexHelper<0, T, U...> {
    using type = T;
  };

  template <typename T>
  struct TypeByIndexHelper<0, T> {
    using type = T;
  };

  template <std::size_t I, typename T>
  struct TypeByIndex {
    using type = void;
  };

  template <std::size_t I, template<typename...> class T, typename... U>
  struct TypeByIndex<I, T<U...>> {
    using type = TypeByIndexHelper<I, U...>::type;
  };

  template <std::size_t I, template<typename...> class T>
  struct TypeByIndex<I, T<>> {
    using type = void;
  };

  template <std::size_t I, typename T>
  using TypeByIndex_v = TypeByIndex<I, T>::type;

  template <typename T>
  struct SizeVariadicType {
    static const std::size_t size = max_size;
  };

  template < template<typename...> class T, typename... U>
  struct SizeVariadicType<T<U...>> {
    static const std::size_t size = sizeof...(U);
  };

  template < template<typename...> class T>
  struct SizeVariadicType<T<>> {
    static constexpr std::size_t size = 0;
  };
}