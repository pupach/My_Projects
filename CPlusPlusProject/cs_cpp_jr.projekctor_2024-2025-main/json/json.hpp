#pragma once
#include "concept_type.hpp"
#include "details.hpp"
#include "Ubiq.hpp"
#include <array>
#include <charconv>
#include <cstring>
#include <format>
#include <iterator>
#include <string>
#include <vector>

#define TO_JSON(value, ind) \
if constexpr(concepts::IsStruct<decltype(value)>) {  /* TODO ветка согздана, чтобы было менее 900 раскрытий шаблонов в глубину(без нее их 1024), но кажется костылем. Как можно переделать? */ \
  n = GetFieldsTupleImpl<decltype(value)>(value, size_t_<GetFieldsCount<decltype(value)>()>());\
  }  \
 else {  \
    n = ConvertToJson(value); \
} \
if(!n.empty()) {\
  to_ret += extract_value_name(extract_value(fields.field[ind]));\
  to_ret += ": ";\
  to_ret += n;\
  to_ret += ", ";\
}


template<typename ENUM_TYPE> requires (std::is_enum_v<ENUM_TYPE>)
constexpr std::array<std::array<char, MAX_SIZE_ENUM>, MAX_SIZE_NAME + 1>  MakeTypeArray() {
  std::array<std::array<char, MAX_SIZE_ENUM>, MAX_SIZE_NAME + 1>  to_ret = {};

  Ubiq<0> type(to_ret);
  ENUM_TYPE b = (ENUM_TYPE)type;

  return to_ret;
}

template <auto T>
constexpr std::string GetName() {
  return __PRETTY_FUNCTION__;
}


template<typename T> requires(concepts::ForwardContainer<T>)
std::string ConvertArrayToJson(const T& value, std::string next_del = ", ", std::string del = ", ", std::array<std::string, 2> skob = {"[", "]"});

template<typename T> requires(concepts::ForwardContainer<T>)
std::string ConvertArrayToJson(const T& value, std::string next_del, std::string del, std::array<std::string, 2> skob, std::array<std::string, 2> next_skob);

template<typename T> requires(concepts::ContWithTemGetr<T>)
std::string ConvertGetrTypeToJson(const T& value, std::string del, std::array<std::string, 2> skob);

template<typename T> requires(concepts::ContWithTemGetr<T>)
std::string ConvertGetrTypeToJson(const T& value, std::string del, std::array<std::string, 2> skob, std::string next_del, std::array<std::string, 2> next_skob);

template<typename T> requires(concepts::ContWithHldAlt<T>)
std::string ConvertHldAltTypeToJson(const T& value, std::array<std::string, 2> skob = {"", ""});

template<typename T> requires (concepts::BasicTypes<T>)
std::string ConvertToJson(const T& value, std::string del = ", ", std::array<std::string, 2> skob = {"", ""}) {
 // using details::to_string; TODO так писать нельзя, тк тогда он не полезет в глобальный scope(в котором может и не быть пользовательского to_string)
  using namespace details; // TODO так писать не хочется, тк выгружаем кучу лишнего
  return skob[0] + std::format("{}", to_string(value)) + skob[1];
}

template<typename T> requires (concepts::IsFloatPoint<T>)
std::string ConvertToJson(const T& value, std::string del = ", ", std::array<std::string, 2> skob = {"", ""}) {
  return skob[0] + std::format("{:g}", value) + skob[1];
}

template<typename T> requires (concepts::ForwardContainer<T>)
std::string ConvertToJson(const T& value, std::string del = ", ", std::array<std::string, 2> skob = {"[", "]"}) {
  return ConvertArrayToJson(value, ", ", del, skob);
}


template<typename T> requires (concepts::NonTrivialkeepedContainer<T>)
std::string ConvertToJson(const T& value, std::string del = ", ", std::array<std::string, 2> skob = {"{", "}"}) {
  return ConvertArrayToJson(value, ": ", del, skob, {"", ""});
}

template<typename T> requires (concepts::ContWithTemGetr<T>)
std::string ConvertToJson(const T& value, std::string del = ", ", std::array<std::string, 2> skob = {"[", "]"}) {
  return ConvertGetrTypeToJson(value, del, skob);
}

template<typename T> requires (concepts::ContWithHldAlt<T>)
std::string ConvertToJson(const T& value, std::string del = ", ", std::array<std::string, 2> skob = {"", ""}) {
  return ConvertHldAltTypeToJson(value, skob);
}

template<typename T>
std::string ConvertToJson(const std::optional<T> & value, std::string del = ", ", std::array<std::string, 2> skob = {"", ""}) {
  if(value.has_value()) {
    return ConvertToJson(value.value());
  }
  return "";
}

template<typename T> requires(concepts::ContWithTemGetr<T>)
std::string ConvertGetrTypeToJson(const T& value, std::string del, std::array<std::string, 2> skob, std::string next_del, std::array<std::string, 2> next_skob) {
  constexpr std::size_t size = details::SizeVariadicType<T>::size;

  if(size == 0) return skob[0] + skob[1];

  return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    std::string to_ret = skob[0];


    auto append = [&](std::string next_json) {
      if((to_ret != skob[0]) && (next_json != "")) to_ret += del;
      to_ret += next_json;
    };
    (append(ConvertToJson(details::get<Is>(value), next_del, next_skob)), ...);

    to_ret += skob[1];

    if(del == ": ") {//TODO эта ветка была создана для обработки std::pair в std::map в случае std::nullopt;
                     //Вообще непонятно, почему мы не хотим сохранять пустые ключи, ведь они тоже занимают место
      if(to_ret.find(": ") == std::string::npos) {
        return std::string("");
      }
    }
    return to_ret;
  }(std::make_index_sequence<size>{});
}


template<typename T> requires(concepts::ContWithTemGetr<T>)
std::string ConvertGetrTypeToJson(const T& value, std::string del, std::array<std::string, 2> skob) {
  constexpr std::size_t size = details::SizeVariadicType<T>::size;

  if(size == 0) return skob[0] + skob[1];

  return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    std::string to_ret = skob[0];


    auto append = [&](std::string next_json) {
      if((to_ret != skob[0]) && (next_json != "")) to_ret += del;
      to_ret += next_json;
    };
    (append(ConvertToJson(details::get<Is>(value))), ...);

   to_ret += skob[1];

   if(del == ": ") {
     if(to_ret.find(": ") == std::string::npos) {
       return std::string(""); //кринж
     }
   }
   return to_ret;
  }(std::make_index_sequence<size>{});
}

template<typename T> requires(concepts::ContWithHldAlt<T>)
std::string ConvertHldAltTypeToJson(const T& value, std::array<std::string, 2> skob) {
  constexpr std::size_t size = details::SizeVariadicType<T>::size;

  if(size == 0) return skob[0] + skob[1];

  return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    std::string to_ret = skob[0];

    auto append = [&]<std::size_t I>(bool next_type) {
      if(next_type) {
        to_ret += ConvertToJson(details::get<I>(value));
      }
    };
    (append.template operator()<Is>(details::holds_alternative<details::TypeByIndex_v<Is, T>>(value)), ...);

    to_ret += skob[1];


    return to_ret;
  }(std::make_index_sequence<size>{});
}

template<typename T> requires (concepts::IsEnum<T>)
std::string ConvertToJson(const T& value, std::string del = ", ", std::array<std::string, 2> skob = {"\"", "\""}) {
  constexpr std::array<std::array<char, MAX_SIZE_ENUM>, MAX_SIZE_NAME + 1>  NameArray = MakeTypeArray<T>();
  constexpr std::array<std::size_t, MAX_SIZE_NAME + 1> size_arrays = [&NameArray] {
    std::array<std::size_t, MAX_SIZE_NAME + 1> sizes{};

    for (std::size_t idx = 0; idx < sizes.size(); ++idx) {
      const auto& src = NameArray[idx];

      std::size_t len = 1;
      while (len < src.size() && src[len] != '\0')
        ++len;
      sizes[idx] = len;
    }
    return sizes;
  }();

  return skob[0] + std::string(std::begin(NameArray[value]) + 1, std::begin(NameArray[value]) + size_arrays[value]) + skob[1];
}

template<typename T> requires(concepts::ForwardContainer<T>)
std::string ConvertArrayToJson(const T& value, std::string next_del, std::string del, std::array<std::string, 2> skob, std::array<std::string, 2> next_skob) {
  std::string to_ret = skob[0];
  for(auto it = value.begin(); it != value.end(); it++) {
    auto json = ConvertToJson(*it, next_del, next_skob);
    if((to_ret != skob[0]) && (json != "")) to_ret += del;
    to_ret += json;
  }
  to_ret += skob[1];

  return to_ret;
}

template<typename T> requires(concepts::ForwardContainer<T>)
std::string ConvertArrayToJson(const T& value, std::string next_del, std::string del, std::array<std::string, 2> skob) {
  std::string to_ret = skob[0];
  for(auto it = value.begin(); it != value.end(); it++) {
      auto json = ConvertToJson(*it, next_del);
      if((to_ret != skob[0]) && (json != "")) to_ret += del;
      to_ret += json;
  }
  to_ret += skob[1];

  return to_ret;
}

template <typename T>
std::string GetFieldsTupleImpl(T& value, size_t_<3>) {
  auto& [a, b, c] = value;

  Mem<T> fields;
  std::string to_ret = "{";
  std::string n;
  TO_JSON(a, 0);
  TO_JSON(b, 1);
  TO_JSON(c, 2);
  if(to_ret != "{") {
    to_ret.pop_back();
    to_ret.pop_back();
  }

  to_ret += "}";

  return  to_ret;
}

template <typename T>
std::string GetFieldsTupleImpl(T& value, size_t_<2>) {
  auto& [a, b] = value;

  Mem<T> fields;
  std::string to_ret = "{";
  std::string n;
  TO_JSON(a, 0);
  TO_JSON(b, 1);
  if(to_ret != "{") {
    to_ret.pop_back();
    to_ret.pop_back();
  }

  to_ret += "}";
  return  to_ret;
}

template <typename T>
std::string GetFieldsTupleImpl(T& value, size_t_<1>) {
  auto& [a] = value;

  Mem<T> fields;
  std::string to_ret = "{";
  std::string n;
  TO_JSON(a, 0);
  if(to_ret != "{") {
    to_ret.pop_back();
    to_ret.pop_back();
  }

  to_ret += "}";
  return  to_ret;
}

template <typename T>
std::string GetFieldsTupleImpl(T& value, size_t_<4>) {
  auto& [a, b, c, d] = value;

  Mem<T> fields;
  std::string to_ret = "{";
  std::string n;
  TO_JSON(a, 0);
  TO_JSON(b, 1);
  TO_JSON(c, 2);
  TO_JSON(d, 3);
  if(to_ret != "{") {
    to_ret.pop_back();
    to_ret.pop_back();
  }

  to_ret += "}";

  return  to_ret;
}

template <typename T>
std::string GetFieldsTupleImpl(T& value, size_t_<5>) {
  auto& [a, b, c, d, a1] = value;

  Mem<T> fields;
  std::string to_ret = "{";
  std::string n;
  TO_JSON(a, 0);
  TO_JSON(b, 1);
  TO_JSON(c, 2);
  TO_JSON(d, 3);
  TO_JSON(a1, 4);
  if(to_ret != "{") {
    to_ret.pop_back();
    to_ret.pop_back();
  }

  to_ret += "}";
  return  to_ret;
}

template <typename T>
std::string GetFieldsTupleImpl(T& value, size_t_<6>) {
  auto& [a, b, c, d, a1, a2] = value;

  Mem<T> fields;
  std::string to_ret = "{";
  std::string n;
  TO_JSON(a, 0);
  TO_JSON(b, 1);
  TO_JSON(c, 2);
  TO_JSON(d, 3);
  TO_JSON(a1, 4);
  TO_JSON(a2, 5);
  if(to_ret != "{") {
    to_ret.pop_back();
    to_ret.pop_back();
  }

  to_ret += "}";
  return  to_ret;
}

template <typename T>
std::string GetFieldsTupleImpl(T& value, size_t_<7>) {
  auto& [a, b, c, d, a1, a2, a3] = value;

  Mem<T> fields;
  std::string to_ret = "{";
  std::string n;
  TO_JSON(a, 0);
  TO_JSON(b, 1);
  TO_JSON(c, 2);
  TO_JSON(d, 3);
  TO_JSON(a1, 4);
  TO_JSON(a2, 5);
  TO_JSON(a3, 6);
  if(to_ret != "{") {
    to_ret.pop_back();
    to_ret.pop_back();
  }

  to_ret += "}";
  return  to_ret;
}

template <typename T>
std::string GetFieldsTupleImpl(T& value, size_t_<8>) {
  auto& [a, b, c, d, a1, a2, a3, a4] = value;

  Mem<T> fields;
  std::string to_ret = "{";
  std::string n;
  TO_JSON(a, 0);
  TO_JSON(b, 1);
  TO_JSON(c, 2);
  TO_JSON(d, 3);
  TO_JSON(a1, 4);
  TO_JSON(a2, 5);
  TO_JSON(a3, 6);
  TO_JSON(a4, 7);
  if(to_ret != "{") {
    to_ret.pop_back();
    to_ret.pop_back();
  }

  to_ret += "}";

  return  to_ret;
}

template <typename T>
std::string GetFieldsTupleImpl(T& value, size_t_<9>) {
  auto& [a, b, c, d, a1, a2, a3, a4, a5] = value;

  Mem<T> fields;
  std::string to_ret = "{";
  std::string n;
  TO_JSON(a, 0);
  TO_JSON(b, 1);
  TO_JSON(c, 2);
  TO_JSON(d, 3);
  TO_JSON(a1, 4);
  TO_JSON(a2, 5);
  TO_JSON(a3, 6);
  TO_JSON(a4, 7);
  TO_JSON(a5, 8);
  if(to_ret != "{") {
    to_ret.pop_back();
    to_ret.pop_back();
  }

  to_ret += "}";
  return  to_ret;
}
template <typename T>
std::string GetFieldsTupleImpl(T& value, size_t_<10>) {
  auto& [a, b, c, d, a1, a2, a3, a4, a5, a6] = value;

  Mem<T> fields;
  std::string to_ret = "{";
  std::string n;
  TO_JSON(a, 0);
  TO_JSON(b, 1);
  TO_JSON(c, 2);
  TO_JSON(d, 3);
  TO_JSON(a1, 4);
  TO_JSON(a2, 5);
  TO_JSON(a3, 6);
  TO_JSON(a4, 7);
  TO_JSON(a5, 8);
  TO_JSON(a6, 9);
  if(to_ret != "{") {
    to_ret.pop_back();
    to_ret.pop_back();
  }

  to_ret += "}";
  return  to_ret;
}

template <typename T>
std::string GetFieldsTupleImpl(T& value, size_t_<11>) {
  auto& [a, b, c, d, a1, a2, a3, a4, a5, a6, a7] = value;

  Mem<T> fields;
  std::string to_ret = "{";
  std::string n;
  TO_JSON(a, 0);
  TO_JSON(b, 1);
  TO_JSON(c, 2);
  TO_JSON(d, 3);
  TO_JSON(a1, 4);
  TO_JSON(a2, 5);
  TO_JSON(a3, 6);
  TO_JSON(a4, 7);
  TO_JSON(a5, 8);
  TO_JSON(a6, 9);
  TO_JSON(a7, 10);
  if(to_ret != "{") {
    to_ret.pop_back();
    to_ret.pop_back();
  }

  to_ret += "}";
  return  to_ret;
}

template <typename T>
std::string GetFieldsTupleImpl(T& value, size_t_<12>) {
  auto& [a, b, c, d, a1, a2, a3, a4, a5, a6, a7, a8] = value;

  Mem<T> fields;
  std::string to_ret = "{";
  std::string n;
  TO_JSON(a, 0);
  TO_JSON(b, 1);
  TO_JSON(c, 2);
  TO_JSON(d, 3);
  TO_JSON(a1, 4);
  TO_JSON(a2, 5);
  TO_JSON(a3, 6);
  TO_JSON(a4, 7);
  TO_JSON(a5, 8);
  TO_JSON(a6, 9);
  TO_JSON(a7, 10);
  TO_JSON(a8, 11);
  if(to_ret != "{") {
    to_ret.pop_back();
    to_ret.pop_back();
  }

  to_ret += "}";
  return  to_ret;
}

template <typename T>
std::string GetFieldsTupleImpl(T& value, size_t_<13>) {
  auto& [a, b, c, d, a1, a2, a3, a4, a5, a6, a7, a8, a9] = value;

  Mem<T> fields;
  std::string to_ret = "{";
  std::string n;

  TO_JSON(a, 0);
  TO_JSON(b, 1);
  TO_JSON(c, 2);
  TO_JSON(d, 3);
  TO_JSON(a1, 4);
  TO_JSON(a2, 5);
  TO_JSON(a3, 6);
  TO_JSON(a4, 7);
  TO_JSON(a5, 8);
  TO_JSON(a6, 9);
  TO_JSON(a7, 10);
  TO_JSON(a8, 11);
  TO_JSON(a9, 12);
  if(to_ret != "{") {
    to_ret.pop_back();
    to_ret.pop_back();
  }

  to_ret += "}";
  return  to_ret;
}

template <typename T>
std::string GetFieldsTupleImpl(T& value, size_t_<14>) {
  auto& [a, b, c, d, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10] = value;

  Mem<T> fields;
  std::string to_ret = "{";
  std::string n;

  TO_JSON(a, 0);
  TO_JSON(b, 1);
  TO_JSON(c, 2);
  TO_JSON(d, 3);
  TO_JSON(a1, 4);
  TO_JSON(a2, 5);
  TO_JSON(a3, 6);
  TO_JSON(a4, 7);
  TO_JSON(a5, 8);
  TO_JSON(a6, 9);
  TO_JSON(a7, 10);
  TO_JSON(a8, 11);
  TO_JSON(a9, 12);
  TO_JSON(a10, 13);
  if(to_ret != "{") {
    to_ret.pop_back();
    to_ret.pop_back();
  }

  to_ret += "}";
  return  to_ret;
}

template <typename T>
std::string GetFieldsTupleImpl(T& value, size_t_<15>) {
  auto& [a, b, c, d, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11] = value;

  Mem<T> fields;
  std::string to_ret = "{";
  std::string n;

  TO_JSON(a, 0);
  TO_JSON(b, 1);
  TO_JSON(c, 2);
  TO_JSON(d, 3);
  TO_JSON(a1, 4);
  TO_JSON(a2, 5);
  TO_JSON(a3, 6);
  TO_JSON(a4, 7);
  TO_JSON(a5, 8);
  TO_JSON(a6, 9);
  TO_JSON(a7, 10);
  TO_JSON(a8, 11);
  TO_JSON(a9, 12);
  TO_JSON(a10, 13);
  TO_JSON(a11, 14);
  if(to_ret != "{") {
    to_ret.pop_back();
    to_ret.pop_back();
  }

  to_ret += "}";
  return  to_ret;
}

template<typename T> requires(concepts::IsStruct<T>)
std::string ConvertToJson(const T& value, std::string del = ", ", std::array<std::string, 2> skob = {"{", "}"}) {

  return GetFieldsTupleImpl(value, size_t_<GetFieldsCount<T>()>());
}