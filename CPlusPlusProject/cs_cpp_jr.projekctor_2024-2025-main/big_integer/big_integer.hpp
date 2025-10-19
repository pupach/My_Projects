#pragma once
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

class BigInt {
 public:
  friend std::ostream& operator<<(std::ostream& stream, const BigInt& big_int);

  BigInt& operator+=(const BigInt& other);

  BigInt& operator-=(const BigInt& other);

  BigInt& operator*=(const BigInt& other);

  BigInt& operator%=(const BigInt& other);

  BigInt& operator/=(const BigInt& other);

  BigInt() = default;

  BigInt(int len, int val) : bigint_parts_(len, val), sign_(Sign::PLUS) {}

  BigInt(const std::string& str_numb);

  BigInt(int64_t numb);

  std::strong_ordering operator<=>(const BigInt& other) const;

  bool operator==(const BigInt&) const = default;

  bool operator<(const BigInt&) const = default;

  bool operator>(const BigInt&) const = default;

  bool operator>=(const BigInt&) const = default;

  bool operator<=(const BigInt&) const = default;

  bool operator!=(const BigInt&) const = default;

  BigInt operator-(const BigInt& other) const;

  BigInt operator++(int);

  BigInt& operator++();

  BigInt operator--(int);

  BigInt& operator--();

  BigInt operator+(const BigInt& other) const;

  BigInt operator*(const BigInt& other) const;

  BigInt operator%(const BigInt& other) const;

  BigInt operator/(const BigInt& other) const;

  BigInt operator-() const;

 private:
  enum class Sign {
    PLUS = 1,
    MINUS = -1,
  };

  Sign sign_;
  static const auto kMaxDegree = 9;
  static const uint64_t kMaxVal = 1e9;
  static const int kAmountInt64 = 3;

  std::vector<uint32_t> bigint_parts_;

  std::vector<uint32_t>& GetBigIntParts();

  BigInt& MulInt(uint32_t numb_to_mul);

  uint32_t BinSearchNumber(const BigInt& main, const BigInt& divider,
                           uint32_t begin, uint32_t end);

  const std::vector<uint32_t>& GetBigIntParts() const;

  std::size_t BigIntPartsSize() const;

  static int CountSignificantNumb(uint32_t numb);

  void DivCheckSign(const BigInt& other, BigInt& res);

  static int CountSigInStr(const std::string& str_numb, int& count_minus);

  void AddWithoutZeros(BigInt& example);

  int MulIntTenGetDegree(int degree);

  uint32_t PointerHelperLoop(const BigInt& other, BigInt& res,
                             uint32_t transfer, std::size_t i, size_t j);

  BigInt& MulIntTenDegree(int degree);

  BigInt& MulCheckSign(const BigInt& other, BigInt& res);

  void PlusHelperLoop(const BigInt& other);

  void DeleteStartingZeros();

  void CheckSignMinusEqual(const BigInt& other);

  void DivWithDiffSign(const BigInt& other);

  void MinusCheckSign(long long int transfer, bool flag);

  void IncreaseToSize(size_t new_size);

  int CountDegree(const BigInt& other) const;

  long long OperatorMinusPrepare(const BigInt& other, bool& flag_change);

  std::strong_ordering SpaceShip(const BigInt& other, std::strong_ordering less,
                                 std::strong_ordering greater) const;
};

std::ostream& operator<<(std::ostream& stream, const BigInt& big_int);

std::istream& operator>>(std::istream& stream, BigInt& big_int);
