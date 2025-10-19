
#include "json.hpp"
#include <vector>
#include <iostream>
#include <string>
//TODO В идеале хочется, чтобы пользователь мог для любых комбинауий типов сделать
// пользовательскую перегрузку ниже проблемы
//Ниже описанные проблемы возникают только для непользовательских типов(ADL не находит перегрузки)



template <typename T>
concept Broken = requires(T a) {
  requires std::bidirectional_iterator<decltype(a.begin())>;
};

/*
template<typename T> requires(Broken<T>)
std::string ConvertToJson(T a) {        //ERROR из за конфликта концептов
  return "meow";
}*/


template<typename T> requires(Broken<T>)
std::string to_string(T a) {        //OK если подключить json.hpp ниже и не видит если выше
  return "meow";
}

struct A {
 int x = 1;
};

std::string ConvertToJson(A a) {
  return "meow";
}

//#include "json.hpp" TODO мне не нравится идея подключать hpp в середине файла,
//                         иначе пользователь не может определять метод to_string(если просто переопределять ConvertToJson могут случаться конфликты в сложных случаях)


int main() {
  std::vector<int> a{1, 2};
  std::cerr << ConvertToJson(a); // Хочу meow

  A b;
  std::cerr << ConvertToJson(b);

}
