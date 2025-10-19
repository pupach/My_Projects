#pragma once
#include <bits/stdc++.h>
#include <type_traits>
#include <utility>
#include <string>

#include "Configurator.hpp"

template<typename T>
concept has_output_operator = requires(T a) {
  std::declval<std::stringstream &>() << std::declval<T>();
};

template <typename T>
constexpr bool is_operator_v = has_output_operator<T>;


template<typename T, typename ConfiguratorTag>
std::string GetValueAsString(DefaultSettings *ptr) {
  auto real_ptr = static_cast<Setting<T, ConfiguratorTag>*>(ptr);

  std::stringstream to_ret;// Создаем ПУСТОЙ поток
  if(real_ptr->has_value()) {
    if constexpr (std::is_same_v<T, bool>) {
      to_ret << std::boolalpha;
    }
    to_ret << real_ptr->value();
  }
  else {
    to_ret << "";
  }

  return to_ret.str();
}

template<typename T, typename ConfiguratorTag>
void FromString(DefaultSettings* ptr, const std::string& new_val = std::string()) {
  auto real_ptr = static_cast<Setting<T, ConfiguratorTag>*>(ptr);

  std::stringstream ss(new_val);
  T temp_value;
  if constexpr (std::is_same_v<T, bool>) {
    ss >> std::boolalpha;
  }
  ss >> temp_value;

  real_ptr->SetValue(std::move(temp_value));
}

template<typename T, typename ConfiguratorTag>
void ResetValue(DefaultSettings *ptr) {
  auto real_ptr = static_cast<Setting<T, ConfiguratorTag>*>(ptr);
  real_ptr->reset();
  return;
}

template<typename T, typename ConfiguratorTag>
class Setting : public DefaultSettings{
  inline static VtableForSettings vtable = VtableForSettings();

  std::optional<T> _value = std::nullopt;
  CallableBase<T> *func = nullptr;

  void init() {
    if constexpr (is_operator_v<T>) {
      this->vtable_ptr->to_string = &GetValueAsString<T, ConfiguratorTag>;
      this->vtable_ptr->from_string = &FromString<T, ConfiguratorTag>;
    }
    this->vtable_ptr->reset_val = &ResetValue<T, ConfiguratorTag>;
    type_id = reinterpret_cast<const void *>(&TypeInfoInCompileTime<T>::val);
  }

  void init_config() {
    decltype(auto) config = Configurator<ConfiguratorTag>::GetInstance();
    config.AddSettings(static_cast<DefaultSettings *>(this));
  }

public:


  explicit Setting(const std::string& a) : DefaultSettings(a, &vtable) {
    init();
    init_config();
  }


  Setting(const std::string& a, const T& default_val, const std::string& help_text = "") : DefaultSettings(a, &vtable, help_text), _value(default_val) {
    init();
    init_config();
  }


  Setting(const std::string& a, T&& default_val, const std::string& help_text = "") : DefaultSettings(a, &vtable, help_text), _value(std::move(default_val)) {
    init();
    init_config();

    this->vtable_ptr->reset_val = &ResetValue<T, ConfiguratorTag>;
  }


  template<typename F>
  Setting(const std::string& a, const T& default_val, const std::string& help_text, F func) : DefaultSettings(a, &vtable, help_text), _value(default_val) {
    init();

    auto new_func = new Function<F, T>(func);
    this->func = static_cast<CallableBase<T> *>(new_func);
  }

  template<typename F>
  Setting(const std::string& a, T&& default_val, const std::string& help_text, F func) : DefaultSettings(a, &vtable, help_text), _value(std::move(default_val)) {
    init();
    init_config();

    auto new_func = new Function<F, T>(func);
    this->func = static_cast<CallableBase<T> *>(new_func);
  }

  template <typename D>
  std::optional<T> SetValue(D&& new_val) {
    std::optional<T> old_falue = std::move(_value);
    if(this->func != nullptr) {
      this->func->vtable_ptr->invoke(this->func, std::move(new_val));
    }
    _value = std::forward<D&&>(new_val);
    return old_falue;
  }

  bool HasValue() {
    return _value.has_value();
  }

  bool has_value() {
    return _value.has_value();
  }

  void reset(){
    _value.reset();
  }

  constexpr T& GetValue() {
    if (this->HasValue()) {
      return _value.value();
    }
    else {
      throw 1;
    }
  }

  constexpr T& value() {
    if (this->HasValue()) {
      return _value.value();
    }
    else {
      throw 1;
    }
  }

};