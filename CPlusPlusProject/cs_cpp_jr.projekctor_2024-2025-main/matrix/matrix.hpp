#pragma once
#include <stdint.h>

#include <iostream>
#include <vector>

template <size_t N, size_t M, typename T>
class Matrix;

template <size_t N, size_t M, typename T = int64_t>
class MatrixPrototype {
 public:
  MatrixPrototype() : table_(N, std::vector<T>(M, T(0))) {}

  explicit MatrixPrototype(const std::vector<std::vector<T>>& init_vector)
      : table_(init_vector) {}

  explicit MatrixPrototype(const T& elem)
      : table_(N, std::vector<T>(M, elem)) {}

  const std::vector<std::vector<T>>& GetTable() const { return this->table_; }
  std::vector<std::vector<T>>& GetTable() { return this->table_; }

  T operator()(size_t first, size_t second) const;

  MatrixPrototype& operator+=(const MatrixPrototype& others);

  MatrixPrototype& operator-=(const MatrixPrototype& others);

  MatrixPrototype& operator*=(const T& others);

  Matrix<N, M, T> operator-(const MatrixPrototype& other) const;

  Matrix<N, M, T> operator+(const MatrixPrototype& other) const;

  Matrix<N, M, T> operator*(const T& other) const;

  template <size_t K>
  Matrix<N, K, T> operator*(const MatrixPrototype<M, K, T>& other) const;

  Matrix<M, N, T> Transposed() const;

  bool operator==(const MatrixPrototype& other);

  MatrixPrototype& operator=(const MatrixPrototype& other) {
    table_ = other.GetTable();
    return *this;
  }

 protected:
  MatrixPrototype(const MatrixPrototype& other) : table_(other.GetTable()) {}

  std::vector<std::vector<T>> table_;
};

template <size_t N, size_t M, typename T = int64_t>
class Matrix : public MatrixPrototype<N, M, T> {
 public:
  using MatrixPrototype<N, M, T>::MatrixPrototype;

  Matrix(const MatrixPrototype<N, M, T>& other)
      : MatrixPrototype<N, M, T>(other) {}
};

template <size_t N, typename T>
class Matrix<N, N, T> : public MatrixPrototype<N, N, T> {
 public:
  using MatrixPrototype<N, N, T>::MatrixPrototype;

  Matrix(const MatrixPrototype<N, N, T>& other)
      : MatrixPrototype<N, N, T>(other) {}

  T Trace() const;
};

template <size_t N, size_t M, typename T>
Matrix<N, M, T> MatrixPrototype<N, M, T>::operator+(
    const MatrixPrototype& other) const {
  Matrix<N, M, T> result = *this;
  result += other;

  return result;
}

template <size_t N, typename T>
T Matrix<N, N, T>::Trace() const {
  T acc_sum = T();

  for (size_t i = 0; i < N; i++) {
    acc_sum += this->table_[i][i];
  }

  return acc_sum;
}

template <size_t N, size_t M, typename T>
template <size_t K>
Matrix<N, K, T> MatrixPrototype<N, M, T>::operator*(
    const MatrixPrototype<M, K, T>& other) const {
  std::vector<std::vector<T>> new_table(N, std::vector<T>(K, 0));

  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < K; j++) {
      for (size_t k = 0; k < M; k++) {
        new_table[i][j] += this->table_[i][k] * (other.GetTable())[k][j];
      }
    }
  }
  return Matrix<N, K, T>(new_table);
}

template <size_t N, size_t M, typename T>
Matrix<M, N, T> MatrixPrototype<N, M, T>::Transposed() const {
  Matrix<M, N, T> new_matrix = {};

  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < M; j++) {
      (new_matrix.GetTable())[j][i] = (this->table_)[i][j];
    }
  }
  return new_matrix;
}

template <size_t N, size_t M, typename T>
bool MatrixPrototype<N, M, T>::operator==(const MatrixPrototype& other) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      if ((other.GetTable())[i][j] != this->table_[i][j]) {
        return false;
      }
    }
  }
  return true;
}

template <size_t N, size_t M, typename T>
T MatrixPrototype<N, M, T>::operator()(size_t first, size_t second) const {
  return this->table_[first][second];
}

template <size_t N, size_t M, typename T>
MatrixPrototype<N, M, T>& MatrixPrototype<N, M, T>::operator+=(
    const MatrixPrototype<N, M, T>& others) {
  for (size_t i = 0; i < others.GetTable().size(); i++) {
    for (size_t j = 0; j < (others.GetTable())[0].size(); j++) {
      this->table_[i][j] += (others.GetTable())[i][j];
    }
  }
  return *this;
}

template <size_t N, size_t M, typename T>
MatrixPrototype<N, M, T>& MatrixPrototype<N, M, T>::operator-=(
    const MatrixPrototype<N, M, T>& others) {
  for (size_t i = 0; i < others.GetTable().size(); i++) {
    for (size_t j = 0; j < (others.GetTable())[0].size(); j++) {
      this->table_[i][j] -= (others.GetTable())[i][j];
    }
  }
  return *this;
}

template <size_t N, size_t M, typename T>
MatrixPrototype<N, M, T>& MatrixPrototype<N, M, T>::operator*=(
    const T& others) {
  for (size_t i = 0; i < this->table_.size(); i++) {
    for (size_t j = 0; j < this->table_[0].size(); j++) {
      this->table_[i][j] *= others;
    }
  }
  return *this;
}

template <size_t N, size_t M, typename T>
Matrix<N, M, T> MatrixPrototype<N, M, T>::operator-(
    const MatrixPrototype<N, M, T>& other) const {
  Matrix<N, M, T> result = *this;
  result -= other;

  return result;
}

template <size_t N, size_t M, typename T>
Matrix<N, M, T> MatrixPrototype<N, M, T>::operator*(const T& other) const {
  Matrix<N, M, T> result = *this;
  result *= other;

  return result;
}