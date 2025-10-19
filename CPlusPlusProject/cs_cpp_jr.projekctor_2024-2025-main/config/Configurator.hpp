#pragma once
#include <iostream>
#include <optional>
#include <typeinfo>
#include <map>
#include <utility>

template<typename T>
struct CallableBase;

template<typename T>
struct VtableForFunction {
  void (*invoke )(CallableBase<T> *, T) = nullptr;
};

template<typename F, typename T>
struct Function;

template<typename T>
struct CallableBase{
public:
  VtableForFunction<T> * vtable_ptr = nullptr;
  inline static VtableForFunction<T> vtable = VtableForFunction<T>();
};

template<typename F, typename T>
void invoke(CallableBase<T> *ptr, T val) {
  auto real_tr = static_cast<Function<F, T> *>(ptr);

  real_tr->func(std::forward<T>(val));
}

template<typename F, typename T>
struct Function : public CallableBase<T>{
public:
  F func;
  inline static VtableForFunction<T> vtable = VtableForFunction<T>(&invoke<F, T>);

  Function(F func) : CallableBase<T>(), func(func) {
    this->vtable_ptr = &this->vtable;
  }
};



struct UnknownParametr : std::exception{};

struct BadConfigValueType : std::exception{};

template<typename T>
struct TypeInfoInCompileTime {
public:
  T * meow = nullptr;
  static constexpr int val = 0;
};


struct MainConfiguratorTag {};

template <typename T>
class Singleton {
public:
  static constexpr T& GetInstance() {
    return instance;
  }
  static T instance;

};


class DefaultSettings;

struct VtableForSettings {
  std::string(*to_string)(DefaultSettings *) = nullptr;

  void(*from_string)(DefaultSettings *, const std::string&) = nullptr;

  void(*reset_val)(DefaultSettings *) = nullptr;

};




class DefaultSettings {
public:
  inline static VtableForSettings vtable = VtableForSettings();

  VtableForSettings *vtable_ptr = nullptr;
  std::string name_param;
  DefaultSettings *next = nullptr;
  const void * type_id;
  std::string help_text;

  DefaultSettings() {

  }

  constexpr explicit DefaultSettings(const std::string& a, VtableForSettings *v_ptr = &vtable, std::string help_text = std::string()) : name_param(a), vtable_ptr(v_ptr) {
    this->help_text = std::move(help_text);
  }
};

template<typename T, typename ConfiguratorTag = MainConfiguratorTag>
class Setting;


template<typename ConfiguratorTag = MainConfiguratorTag>
class Configurator : public Singleton<Configurator<ConfiguratorTag>> {
private:
  bool flag_init = false;
  DefaultSettings *setting_head = nullptr;
  DefaultSettings *setting_tail = nullptr;

public:

  void Init(std::map<std::string, std::string> init_map) {
    flag_init = true;
    std::map<std::string, std::string>::iterator it;

    for (it = init_map.begin(); it != init_map.end(); it++) {
      auto to_find = find_settings(it->first);
      to_find->vtable_ptr->from_string(to_find, it->second);

    }

  }

  DefaultSettings * find_settings(const std::string& name) {
    DefaultSettings *to_find = setting_head;
    while(name != to_find->name_param) {
      if(to_find == setting_tail) throw UnknownParametr();
      to_find = to_find->next;
    }

    return to_find;
  }

  template<typename T>
  constexpr Setting<T, ConfiguratorTag>& GetValue(const std::string& name) {
    auto to_find = find_settings(name);

    if(to_find->type_id != reinterpret_cast<const void *>(&TypeInfoInCompileTime<T>::val)) {
      throw BadConfigValueType();
    }
    auto real_ptr = static_cast<Setting<T, ConfiguratorTag> *>(to_find);


    return *real_ptr;
  }

  template<typename T>
  constexpr std::optional<T> SetValue(const std::string& name, T&& new_val) {
    auto to_find = find_settings(name);
    if(to_find->type_id != reinterpret_cast<const void *>(&TypeInfoInCompileTime<T>::val)) {
      throw BadConfigValueType();
    }
    auto real_ptr = static_cast<Setting<T, ConfiguratorTag> *>(to_find);
    return real_ptr->SetValue(std::forward<T>(new_val));
  }

  std::string GetValueAsString(const std::string& name) {
    auto to_find = find_settings(name);

    return to_find->vtable_ptr->to_string(to_find);
  }

  constexpr void AddSettings(DefaultSettings *new_setting) {
    if(this->setting_tail != nullptr) {
      this->setting_tail->next = new_setting;
      this->setting_tail = new_setting;
    }
    else {
      setting_head = new_setting;
      setting_tail = new_setting;
    }
  }

  std::map<std::string, std::string> GetHelp() {
    auto begin_ptr = setting_head;
    auto to_ret = std::map<std::string, std::string>();
    while(begin_ptr != nullptr) {
      to_ret[begin_ptr->name_param] = begin_ptr->help_text;
      begin_ptr = begin_ptr->next;
    }
    return to_ret;
  }

  std::string GetHelp(std::string name) {
    auto to_find = find_settings(name);
    return to_find->help_text;
  }

  void Drop(std::string name) {
    auto to_find = find_settings(name);
    to_find->vtable_ptr->reset_val(to_find);
  }

  constexpr Configurator() = default;
  constexpr ~Configurator() = default;
};

template <typename T>
T Singleton<T>::instance = T();

#include "Setting.hpp"
