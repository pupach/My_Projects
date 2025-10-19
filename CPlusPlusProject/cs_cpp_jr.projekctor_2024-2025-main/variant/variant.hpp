#pragma once
#include <array>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>



template<typename... Ts>
concept NonEmpty = (sizeof...(Ts) > 0);

struct BadVariantAccess : std::exception {};


template <typename... Ts>
union VariantUnion;

template <typename... Ts>
struct Variant;

template <typename... Ts>
struct VariantSizeTraits;


template<typename Head, typename... Tail>
union VariantUnion<Head, Tail...> {
  VariantUnion() {}
  ~VariantUnion() {}

  Head data;
  VariantUnion<Tail...> recr;

  template<typename Y>
  constexpr Y& get() &{
    if constexpr (std::is_same_v<Head, Y>) {
      return data;
    }
    else {
      return recr.template get<Y>();
    }
  }

  template<typename Y>
  constexpr Y&& get() &&{

    if constexpr (std::is_same_v<Head, Y>) {
      return std::move(data);
    }
    else {
      return recr.template get<Y>();
    }
  }

  template<typename Y>
  constexpr const Y& get() const &{
    if constexpr (std::is_same_v<Head, Y>) {
      return data;
    }
    else {
      return std::move(recr).template get<Y>();
    }
  }

  template<typename Y, typename... Args>
   constexpr void create(Args&&... new_data) {
    if constexpr (std::is_same_v<Head, Y>) {
      std::construct_at(&data, std::forward<Args>(new_data)...);
    }
    else {
      return recr.template create<Y>(std::forward<Args>(new_data)...);
    }
  }

  void destroy(std::size_t ind) {
    if (ind == 0) {
      std::destroy_at(&data);
    }
    else {
      return recr.destroy(ind - 1);
    }
  }
};

template<>
union VariantUnion<> {
  VariantUnion() {}
  ~VariantUnion() {}

  static void destroy(std::size_t ind) {
    std::cerr << ind;
    throw BadVariantAccess();
  }
};

template<typename Head, typename... Ts>
struct VariantSizeTraits<Variant<Head, Ts...>> {
  static constexpr std::size_t kSize = VariantSizeTraits<Variant<Ts...>>::kSize + 1;

  template<typename Y>
  static constexpr int kIndexT = (std::is_same_v<Head, Y> ? 0 : VariantSizeTraits<Variant<Ts...>>::template kIndexT<Y> + 1);
};


template<typename T>
struct VariantSizeTraits<Variant<T>> {
  static constexpr std::size_t kSize = 1; // константа не магическая, а логическая

  template<typename Y>
  static constexpr int kIndexT = (std::is_same_v<T, Y> ? 0 : std::numeric_limits<int>::lowest());
};

template<std::size_t I, typename... Ts>
struct VariantTypeTraits;

template<std::size_t I, typename Head, typename... Ts>
struct VariantTypeTraits<I, Head, Ts...>
    : public VariantTypeTraits<I - 1, Ts...> {
  using type = VariantTypeTraits<I - 1, Ts...>::type;
};


template<typename Head, typename... Ts>
struct VariantTypeTraits<0, Head, Ts...> {
  using type = Head;
};

template<typename... TS>
struct VariantField {
  int cur_active = -1;
  VariantUnion<TS...> data;

  VariantField() {}

  ~VariantField() {}

};

template<typename T>
struct VariantBase {
  VariantBase() {
  }


  static constexpr std::type_identity<T> find_best_type(const T&);
  static constexpr std::type_identity<T> find_best_type(T&&);
};

template<typename...>
struct first_type;
template<typename Head, typename... Tail>
struct first_type<Head, Tail...> {
  using type = Head;
};


template<typename... TS>
struct Variant : public VariantField<TS...>, public VariantBase<TS>... {
  //using VariantBase<TS, Variant<TS...>>::VariantBase...;
  //using VariantBase<TS, Variant<TS...>>::operator=...;
  using VariantField<TS...>::VariantField::data;
  using VariantField<TS...>::VariantField::cur_active;
  using VariantBase<TS>::find_best_type...;

  template<typename Head, typename... Args>
    requires NonEmpty<Args...>
  void construct_variontic(const Variant<TS...>& from_copy);

  template<typename Head>
  void construct_variontic(const Variant<TS...>& from_copy);

  template<typename Head, typename... Args>
  requires NonEmpty<Args...>
  void construct_variontic(Variant<TS...>&& from_copy);

  template<typename Head>
  void construct_variontic(Variant<TS...>&& from_copy);


  Variant() : VariantField<TS...>(),
              VariantBase<TS>()...{
    using first = typename first_type<TS...>::type;
    this->data.template create<first>();
    this->cur_active = 0;
  }


  template<typename U, typename T = typename decltype(find_best_type(std::declval<U>()))::type>
  Variant(const U& elem)  : VariantBase<TS>()... {
    constexpr int kVal = VariantSizeTraits<Variant<TS...>>::template kIndexT<T>;
    this->data.template create<typename VariantTypeTraits<kVal, TS...>::type>(elem);
    this->cur_active = kVal;
  }

  Variant(const Variant<TS...>& var) : VariantBase<TS>()... {
    this->construct_variontic<TS...>(var);
    this->cur_active = var.cur_active;
  }

  Variant(Variant<TS...>&& var) : VariantBase<TS>()... {
    this->construct_variontic<TS...>(std::move(var));
    this->cur_active = var.cur_active;
  }

  Variant& operator=(const Variant<TS...>& var) {
    static_assert((!std::is_const_v<TS> && ...));

    if(&var != this) {
      if (this->cur_active != -1) {
        this->data.destroy(this->cur_active);
      }

      this->construct_variontic<TS...>(var);
      this->cur_active = var.cur_active;
    }

    return *this;
  }

  Variant& operator=(Variant<TS...>&& var) {
    static_assert((!std::is_const_v<TS> && ...));

    if(&var != this) {
      if (this->cur_active != -1) {
        this->data.destroy(this->cur_active);
      }

      this->construct_variontic<TS...>(std::move(var));
      this->cur_active = var.cur_active;
    }

    return *this;
  }

  template<typename U, typename T = typename decltype(find_best_type(std::declval<U>()))::type>
    requires ((static_cast<bool>(std::is_assignable<T, U>::value)))
  Variant& operator=(U&& elem){
    constexpr int kIdx =
        VariantSizeTraits<Variant<TS...>>::template kIndexT<T>;

    if (this->cur_active != -1) {
      this->data.destroy(this->cur_active);
    }

    this->data.template create<typename VariantTypeTraits<kIdx, TS...>::type>(
        std::forward<U>(elem));
    this->cur_active = kIdx;
    return *this;
  }

  ~Variant() {
    this->data.destroy(this->cur_active);
  }

  int index() const{
    return this->cur_active;
  }

  template<typename Y>
  Y& get() &{
    if(!this->holds_alternative<Y>()) {
      throw BadVariantAccess();
    }

    return this->data.template get<Y>();
  }

  template<typename Y>
  Y&& get() &&{
    if(!this->holds_alternative<Y>()) {
      throw BadVariantAccess();
    }

    return std::move(this->data.template get<Y>());
  }

  template<typename Y>
  const Y& get() const &{
    if(!this->holds_alternative<Y>()) {
      throw BadVariantAccess();
    }

    return this->data.template get<Y>();
  }

  template<int Index>
  typename VariantTypeTraits<Index, TS...>::type& get() &{
    if(!this->holds_alternative<typename VariantTypeTraits<Index, TS...>::type>()) {
      throw BadVariantAccess();
    }

    return this->data.template get<typename VariantTypeTraits<Index, TS...>::type>();
  }

  template<int Index>
  const typename VariantTypeTraits<Index, TS...>::type& get() const &{
    if(!this->holds_alternative<typename VariantTypeTraits<Index, TS...>::type>()) {
      throw BadVariantAccess();
    }

    return this->data.template get<typename VariantTypeTraits<Index, TS...>::type>();
  }

  template<int Index>
  typename VariantTypeTraits<Index, TS...>::type&& get() &&{
    if(!this->holds_alternative<typename VariantTypeTraits<Index, TS...>::type>()) {
      throw BadVariantAccess();
    }

    return std::move(this->data.template get<typename VariantTypeTraits<Index, TS...>::type>());
  }

  template<typename Y, typename... Args>
  Y& emplace(Args&&... arg) {
    this->data.destroy(this->cur_active);

    this->data.template create<Y>(std::forward<Args>(arg)...);
    this->cur_active =
        VariantSizeTraits<Variant<TS...>>::template kIndexT<Y>;

    return this->data.template get<Y>();
  }

  template<typename Y, typename U, typename... Args>
  Y& emplace(std::initializer_list<U> ill, Args&&... args) {
    this->data.destroy(this->cur_active);

    this->data.template create<Y>(ill, std::forward<Args>(args)...);
    this->cur_active =
        VariantSizeTraits<Variant<TS...>>::template kIndexT<Y>;
    return this->data.template get<Y>();
  }

  template<int Index, typename... Args>
  typename VariantTypeTraits<Index, TS...>::type& emplace(Args&&... arg);

  template<int Index, typename U, typename... Args>
  typename VariantTypeTraits<Index, TS...>::type& emplace(std::initializer_list<U> ill, Args&&... args);

  template<typename Y>
  constexpr bool holds_alternative() const{
    return static_cast<bool>(VariantSizeTraits<Variant<TS...>>::template kIndexT<Y> == this->cur_active);
  }

};

template <int Index, typename... Ts>
constexpr VariantTypeTraits<Index, Ts...>::type& get(Variant<Ts...>& vis) {
  return vis.template get<Index>();
}

template <typename T, typename... Ts>
constexpr T& get(Variant<Ts...>& vis) {
  return vis.template get<T>();
}

template <int Index, typename... Ts>
constexpr const VariantTypeTraits<Index, Ts...>::type& get(const Variant<Ts...>& vis) {
  return vis.template get<Index>();
}

template <int Index, typename... Ts>
constexpr VariantTypeTraits<Index, Ts...>::type&& get(Variant<Ts...>&& vis) {
  return std::move(std::move(vis).template get<Index>());
}

template <typename T, typename... Ts>
constexpr const T& get(const Variant<Ts...>& vis) {
  return vis.template get<T>();
}

template <typename T, typename... Ts>
constexpr T&& get(Variant<Ts...>&& vis) {
  return std::move(vis).template get<T>();
}

template <typename T, typename... Ts>
constexpr bool holds_alternative(const Variant<Ts...>& vis) {
  return vis.template holds_alternative<T>();
}

template<class Visitor, class... Variants, int... Is>
decltype(auto) invoke_all_impl(Visitor&&  vis,
                               Variants&&... variontics){
  return std::invoke(
      std::forward<Visitor>(vis),
      std::forward<Variants>(variontics).template get<Is>()...);
}


template<class... Callables>
struct Visitor : Callables... {
  using Callables::operator()...;

  explicit constexpr Visitor(Callables&&... fns)
      : Callables{std::forward<Callables>(fns)}... {}

};

template<typename Elem, typename Head, typename... Variants>
struct TableRecr
{
  using clean_head = std::remove_cvref_t<Head>;
  using table_t =
      std::array< typename TableRecr<Elem, Variants...>::table_t,
                  VariantSizeTraits<clean_head>::kSize>;
};
template<typename Elem, typename Head>
struct TableRecr<Elem, Head> {
  using clean_head = std::remove_cvref_t<Head>;
  using table_t = std::array<Elem, VariantSizeTraits<clean_head>::kSize>;
};

template<std::size_t Cur = 0, int Dim, typename Array>
constexpr decltype(auto) leaf(Array& arr,
                    const std::array<int, Dim>& idxs)
{
  if constexpr (Cur + 1 == Dim)
  {
    return arr[ idxs[Cur] ];
  }
  else
  {
    return leaf<Cur + 1, Dim>(arr[ idxs[Cur] ], idxs);
  }
}


template<typename Visit, typename... Variants>
struct BuildTable {
  using tuple = std::tuple<Variants &&...>;
  using result_t =
      decltype(std::declval<Visit>()(get<0>(std::declval<Variants>())...));

  using type_elem = std::function<result_t(Visit&&, tuple&&)>;

  using table_t =
      typename TableRecr<type_elem, Variants...>::table_t;

  static inline table_t table{};


  template<int I, std::size_t Dim, int... Idx>
  static void fill_recr() {
    fill<Dim, Idx..., I>();
    if constexpr (I != 0) {
      fill_recr<I - 1, Dim, Idx...>();
    }

  }

  template<std::size_t Dim, int... Idx>
  static void fill()
  {
    if constexpr (Dim == 0)
    {
      auto& elem = leaf<0, sizeof...(Variants)>(
          table,
          std::array<int, sizeof...(Idx)>{Idx...}
      );
      elem = []<typename Visitor, typename Tuple>(Visitor&& visitor,
                                                  Tuple&&   tup) -> result_t
      {
        return std::apply(
            [&](auto&&... vars) -> result_t
            {
              return std::invoke(
                  std::forward<Visitor>(visitor),
                  ::get<Idx>(std::forward<decltype(vars)>(vars))...
              );
            },
            std::forward<Tuple>(tup)
        );
      };
    } else {
      using curr_t = std::tuple_element_t<
          sizeof...(Variants) - Dim,
          std::tuple<Variants...>>;

      constexpr int kNN =
          (int)VariantSizeTraits<std::remove_cvref_t<curr_t>>::kSize;
      fill_recr<kNN - 1, Dim - 1, Idx...>();
    }
  }
};



template<class Visitor, class... Variants>
decltype(auto) visit(Visitor&& vis, Variants&&... vars)
{
  using table_t = typename BuildTable<Visitor, Variants...>::table_t;
  constexpr std::size_t kDim = sizeof...(Variants);

  static const table_t kTable = []{
    BuildTable<Visitor, Variants...>::template fill<kDim>();
    return BuildTable<Visitor, Variants...>::table;
  }();

  std::array<int, kDim> idxs{ static_cast<int>(vars.index())... };
  auto& elem = leaf<0, kDim>(kTable, idxs);

  return elem(
      std::forward<Visitor>(vis),
      std::forward_as_tuple(std::forward<Variants>(vars)...)
  );
}

template<typename... TS>
template<typename Head, typename... Args>
  requires NonEmpty<Args...>
void Variant<TS...>::construct_variontic(const Variant<TS...>& from_copy) { //СЛОВО va***nt(даже в комментариях) забанено! Как с этим жить!
  if(!from_copy.template holds_alternative<Head>()) {
    return construct_variontic<Args...>(from_copy);
  }


  return this->data.template create<Head>( ::get<Head>(from_copy));
}

template<typename... TS>
template<typename Head>
void Variant<TS...>::construct_variontic(const Variant<TS...>& from_copy) {
  if(!from_copy.template holds_alternative<Head>()) {
    throw BadVariantAccess();
  }

  return this->data.template create<Head>( ::get<Head>(from_copy));

}

template<typename... TS>
template<typename Head, typename... Args>
requires NonEmpty<Args...>
void Variant<TS...>::construct_variontic(Variant<TS...>&& from_copy) {
  if(!from_copy.template holds_alternative<Head>()) {
    return construct_variontic<Args...>(from_copy);
  }

  return this->data.template create<Head>( ::get<Head>(std::move(from_copy)));

}

template<typename... TS>
template<typename Head>
void Variant<TS...>::construct_variontic(Variant<TS...>&& from_copy) {
  if(!from_copy.template holds_alternative<Head>()) {
    throw BadVariantAccess();
  }

  return this->data.template create<Head>( ::get<Head>(std::move(from_copy)));

}

template<typename... TS>
template<int Index, typename... Args>
typename VariantTypeTraits<Index, TS...>::type& Variant<TS...>::emplace(Args&&... arg) {
  if (this->cur_active != -1) {
    this->data.destroy(this->cur_active);
}

  this->data.template create<typename VariantTypeTraits<Index, TS...>::type>(std::forward<Args>(arg)...);
  this->cur_active =
      Index;

  return ::get<Index>(*this);
}

template<typename... TS>
template<int Index, typename U, typename... Args>
typename VariantTypeTraits<Index, TS...>::type& Variant<TS...>::emplace(std::initializer_list<U> illl, Args&&... args) {
  if (this->cur_active != -1) {
    this->data.destroy(this->cur_active);
}

  this->data.template create<typename VariantTypeTraits<Index, TS...>::type>(illl, std::forward<Args>(args)...);
  this->cur_active =
      Index;
  return ::get<Index>(*this);

}
