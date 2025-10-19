#pragma once
#include "details.hpp"
#include <concepts>
#include <iostream>
#include <optional>


namespace concepts {
template <typename T> struct Optional {};

template <typename T> struct Optional<std::optional<T>> {
  using value_type = T;
};

template <typename T>
concept IsOptional =
    requires(T a) { typename Optional<std::remove_cvref_t<T>>::value_type; };

template <typename T>
concept IsFloatPoint = std::is_floating_point_v<T>;

template <typename T>
concept IsEnum = std::is_enum_v<T>;

using namespace details; // TODO анлогичная проблема как в json.hpp 61
template <typename T>
concept BasicTypes = !IsFloatPoint<T> && !IsEnum<T> && requires(T a) {
  { to_string(a) } -> std::same_as<std::string>;
};

template <typename T>
concept ForwardContainer = !BasicTypes<T> && requires(T a) {
  requires std::forward_iterator<decltype(a.begin())>;
};

template <typename T>
concept NonTrivialkeepedContainer =
    ForwardContainer<T> && requires { typename T::key_type; };

using details::holds_alternative;
template <typename T>
concept ContWithHldAlt = !ForwardContainer<T> && requires(T a) {
  holds_alternative<details::TypeByIndex_v<0, T>>(a);
};

using details::get;
template <typename T>
concept Getr = requires(T a) { get<0>(a); };
template <typename T>
concept ContWithTemGetr = !ContWithHldAlt<T> && !ForwardContainer<T> &&
                          (details::SizeVariadicType<T>::size == 0 || Getr<T>);


template <typename T>
concept IsStruct =
    !IsOptional<T> && !IsEnum<T> && !BasicTypes<T> && !ForwardContainer<T> && //TODO Не нравится писать каждый раз кучу строк !concept<T>, можно ли как то этого избежать?
    !IsFloatPoint<T> && !IsEnum<T> && !ContWithTemGetr<T> && !ContWithHldAlt<T>;

}

