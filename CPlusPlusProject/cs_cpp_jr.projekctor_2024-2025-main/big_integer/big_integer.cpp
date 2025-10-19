#include "big_integer.hpp"

#include <algorithm>
#include <cstdlib>

int BigInt::CountSignificantNumb(uint32_t numb) {
  if (numb == 0) {
    return 1;
  }

  uint32_t st = 1;
  int cnt = 0;
  const int kTen = 10;
  while (st <= numb) {
    cnt++;
    st *= kTen;
  }

  return cnt;
}

BigInt::BigInt(const std::string& str_numb) {
  int amount_sign;
  int count_minus = 0;
  amount_sign = CountSigInStr(str_numb, count_minus);

  if (count_minus % 2 == 1) {
    sign_ = Sign::MINUS;
  } else {
    sign_ = Sign::PLUS;
  }

  int rest_size = str_numb.size();

  do {
    int pos = std::max(rest_size - kMaxDegree, amount_sign + 1);
    int n = rest_size - std::max(rest_size - kMaxDegree, amount_sign + 1);
    bigint_parts_.push_back(std::stol(str_numb.substr(pos, n)));
    rest_size -= kMaxDegree;
  } while (rest_size > amount_sign + 1);

  this->DeleteStartingZeros();

  if (bigint_parts_[bigint_parts_.size() - 1] == 0) {
    sign_ = Sign::PLUS;
  }
}

BigInt::BigInt(int64_t numb) : bigint_parts_(kAmountInt64, 0) {
  sign_ = (numb >= 0 ? Sign::PLUS : Sign::MINUS);
  bigint_parts_[0] = std::abs(numb % (int64_t)kMaxVal);
  bigint_parts_[1] = std::abs((numb / (int64_t)kMaxVal) % (int64_t)kMaxVal);
  bigint_parts_[2] = std::abs((numb / (int64_t)kMaxVal) / (int64_t)kMaxVal);

  this->DeleteStartingZeros();
}

int BigInt::CountSigInStr(const std::string& str_numb, int& count_minus) {
  int count_plus = 0;
  int cnt = 0;
  int amount_sign = 0;
  count_minus = 0;

  while ((str_numb[cnt] == '-') || (str_numb[cnt] == '+')) {
    if (str_numb[cnt] == '-') {
      count_minus++;
    } else {
      count_plus++;
    }
    cnt++;
  }

  amount_sign = count_minus + count_plus - 1;
  return amount_sign;
}

int BigInt::MulIntTenGetDegree(int degree) {
  int count_zeros = 0;

  while (degree >= kMaxDegree) {
    count_zeros++;
    degree -= kMaxDegree;
  }

  bigint_parts_.insert(bigint_parts_.begin(), count_zeros, 0);

  return degree;
}

BigInt& BigInt::MulIntTenDegree(int degree) {
  int numb = 1;
  const int kTen = 10;
  degree = this->MulIntTenGetDegree(degree);
  uint32_t transfer = 0;

  for (int i = 0; i < degree; i++) {
    numb *= kTen;
  }

  for (size_t i = 0; i < bigint_parts_.size(); i++) {
    if ((bigint_parts_[i] != 0) || (transfer != 0)) {
      uint64_t res_mul = bigint_parts_[i] * numb + transfer;
      transfer = res_mul / static_cast<uint32_t>(kMaxVal);
      bigint_parts_[i] = res_mul % static_cast<uint32_t>(kMaxVal);
    }
  }

  if (transfer != 0) {
    bigint_parts_.push_back(transfer);
  }

  return *this;
}

BigInt& BigInt::MulInt(uint32_t numb_to_mul) {
  uint32_t transfer = 0;

  for (size_t i = 0; i < bigint_parts_.size(); i++) {
    if ((bigint_parts_[i] != 0) || (transfer != 0)) {
      uint64_t res_mul =
          static_cast<uint64_t>(bigint_parts_[i]) * numb_to_mul + transfer;
      transfer = res_mul / static_cast<uint32_t>(kMaxVal);
      bigint_parts_[i] = res_mul % static_cast<uint32_t>(kMaxVal);
    }
  }

  if (transfer != 0) {
    bigint_parts_.push_back(transfer);
  }

  return *this;
}

void BigInt::AddWithoutZeros(BigInt& example) {
  bigint_parts_ = std::vector<uint32_t>(0, 0);
  int i;

  for (i = example.bigint_parts_.size() - 1; i > -1; i--) {
    if (example.bigint_parts_[i] != 0) {
      break;
    }
  }

  for (int j = 0; j < i + 1; j++) {
    bigint_parts_.push_back(example.bigint_parts_[j]);
  }
  if (bigint_parts_.empty()) {
    bigint_parts_.push_back(0);
  }
}

void BigInt::DeleteStartingZeros() {
  int i = bigint_parts_.size() - 1;

  while (i > 0) {
    if (bigint_parts_[i] != 0) {
      break;
    }
    bigint_parts_.pop_back();
    i--;
  }
}

BigInt& BigInt::operator++() {
  *this += 1;
  return *this;
}

BigInt BigInt::operator++(int) {
  BigInt new_int = *this;
  *this += 1;
  return new_int;
}

BigInt& BigInt::operator--() {
  *this -= 1;
  return *this;
}

BigInt BigInt::operator--(int) {
  BigInt new_int = *this;
  *this -= 1;
  return new_int;
}

BigInt BigInt::operator-() const {
  BigInt new_int = *this;
  if (new_int.bigint_parts_.back() != 0) {
    new_int.sign_ = (new_int.sign_ == Sign::PLUS ? Sign::MINUS : Sign::PLUS);
  }

  return new_int;
}

BigInt BigInt::operator-(const BigInt& other) const {
  BigInt result = *this;
  result -= other;

  return result;
}

BigInt BigInt::operator+(const BigInt& other) const {
  BigInt result = *this;
  result += other;

  return result;
}

BigInt BigInt::operator*(const BigInt& other) const {
  BigInt result = *this;
  result *= other;

  return result;
}

BigInt BigInt::operator/(const BigInt& other) const {
  BigInt result = *this;
  result /= other;

  return result;
}

BigInt BigInt::operator%(const BigInt& other) const {
  BigInt result = *this;
  result %= other;

  return result;
}
std::vector<uint32_t>& BigInt::GetBigIntParts() { return bigint_parts_; }

const std::vector<uint32_t>& BigInt::GetBigIntParts() const {
  return bigint_parts_;
}

std::size_t BigInt::BigIntPartsSize() const { return bigint_parts_.size(); }

std::strong_ordering BigInt::SpaceShip(const BigInt& other,
                                       std::strong_ordering less,
                                       std::strong_ordering greater) const {
  for (int i = bigint_parts_.size() - 1; i > -1; i--) {
    if (bigint_parts_[i] > other.GetBigIntParts()[i]) {
      return greater;
    }
    if (bigint_parts_[i] < other.GetBigIntParts()[i]) {
      return less;
    }
  }

  return std::strong_ordering::equal;
}

std::strong_ordering BigInt::operator<=>(const BigInt& other) const {
  if ((bigint_parts_.size() == 1) && (other.bigint_parts_.size() == 1) &&
      (bigint_parts_[0] == 0) && (other.bigint_parts_[0] == 0)) {
    return std::strong_ordering::equivalent;
  }
  if ((this->sign_ == Sign::PLUS) && (other.sign_ == Sign::MINUS)) {
    return std::strong_ordering::greater;
  }
  if ((this->sign_ == Sign::MINUS) && (other.sign_ == Sign::PLUS)) {
    return std::strong_ordering::less;
  }
  std::strong_ordering greater = std::strong_ordering::greater;
  if (this->sign_ == Sign::MINUS) {
    greater = std::strong_ordering::less;
  }
  std::strong_ordering less = std::strong_ordering::less;
  if (this->sign_ == Sign::MINUS) {
    less = std::strong_ordering::greater;
  }
  if (bigint_parts_.size() > other.bigint_parts_.size()) {
    return greater;
  }
  if (bigint_parts_.size() < other.bigint_parts_.size()) {
    return less;
  }

  return SpaceShip(other, less, greater);
}

void BigInt::DivCheckSign(const BigInt& other, BigInt& res) {
  other.CountDegree(*this);
  BigInt::Sign res_sign;

  if (other.sign_ == sign_) {
    res_sign = Sign::PLUS;
  } else {
    res_sign = Sign::MINUS;
  }

  this->AddWithoutZeros(res);
  sign_ = res_sign;

  if (bigint_parts_[BigIntPartsSize() - 1] == 0) {
    sign_ = Sign::PLUS;
  }
}

uint32_t BigInt::PointerHelperLoop(const BigInt& other, BigInt& res,
                                   uint32_t transfer, std::size_t i, size_t j) {
  uint64_t res_mul =
      static_cast<uint64_t>(bigint_parts_[i]) * other.bigint_parts_[j] +
      transfer;
  uint32_t res_f = static_cast<uint32_t>(res_mul % kMaxVal);
  uint32_t res_s = static_cast<uint32_t>(res_mul / kMaxVal);
  transfer =
      (res_f + res.bigint_parts_[i + j]) / static_cast<uint32_t>(kMaxVal) +
      res_s;
  res.bigint_parts_[i + j] =
      static_cast<uint32_t>((res_f + res.bigint_parts_[i + j]) % kMaxVal);

  return transfer;
}

BigInt& BigInt::operator*=(const BigInt& other) {
  std::size_t real_size = bigint_parts_.size();
  uint32_t transfer = 0;

  for (size_t i = 0; i < other.bigint_parts_.size() + 1; i++) {
    bigint_parts_.push_back(0);
  }

  BigInt res(bigint_parts_.size(), 0);

  for (std::size_t i = 0; i < real_size; i++) {
    for (size_t j = 0; j < other.bigint_parts_.size(); j++) {
      transfer = this->PointerHelperLoop(other, res, transfer, i, j);
    }

    std::size_t cnt = other.bigint_parts_.size();

    while (transfer != 0) {
      uint64_t res_mul = transfer;
      uint32_t res_f = static_cast<uint32_t>(res_mul % kMaxVal);
      uint32_t res_s = static_cast<uint32_t>(res_mul / kMaxVal);
      transfer = (res_f + res.bigint_parts_[i + cnt]) / kMaxVal + res_s;
      res.bigint_parts_[i + cnt] =
          static_cast<uint32_t>((res_f + res.bigint_parts_[i + cnt]) % kMaxVal);
      cnt++;
    }
  }

  return MulCheckSign(other, res);
}

BigInt& BigInt::MulCheckSign(const BigInt& other, BigInt& res) {
  BigInt::Sign sign1;

  if (other.sign_ == sign_) {
    sign1 = Sign::PLUS;
  } else {
    sign1 = Sign::MINUS;
  }

  this->AddWithoutZeros(res);
  sign_ = sign1;

  if (bigint_parts_[bigint_parts_.size() - 1] == 0) {
    sign_ = Sign::PLUS;
  }

  return *this;
}

uint32_t BigInt::BinSearchNumber(const BigInt& main, const BigInt& divider,
                                 uint32_t begin, uint32_t end) {
  if (begin == end) {
    return begin;
  }

  uint32_t middle = (static_cast<uint64_t>(begin) + end) / 2;

  if (middle == begin) {
    BigInt res_mul2 = divider;
    res_mul2.MulInt(end);
    if (res_mul2 <= main) {
      return end;
    }
    return begin;
  }

  BigInt res_mul = divider;
  res_mul.MulInt(middle);

  if (main > res_mul) {
    return BinSearchNumber(main, divider, middle, end);
  }
  if (main == const_cast<const BigInt&>(res_mul)) {
    return middle;
  }

  return BinSearchNumber(main, divider, begin, middle);
}

void BigInt::DivWithDiffSign(const BigInt& other) {
  this->sign_ = Sign::PLUS;
  BigInt positive_other = other;
  positive_other.sign_ = Sign::PLUS;
  *this /= positive_other;

  if ((this->bigint_parts_.size() == 1) && (this->bigint_parts_[0] == 0)) {
    this->sign_ = Sign::PLUS;
  } else {
    this->sign_ = Sign::MINUS;
  }
}

BigInt& BigInt::operator/=(const BigInt& others) {
  if (this->sign_ != others.sign_) {
    DivWithDiffSign(others);
    return *this;
  }

  this->sign_ = Sign::PLUS;
  BigInt other = others;
  other.sign_ = Sign::PLUS;
  BigInt result_div(bigint_parts_.size() - other.bigint_parts_.size() + 1, 0);
  BigInt to_div = other;

  while (*this >= other) {
    this->DeleteStartingZeros();
    int transfer = CountDegree(other);
    to_div.MulIntTenDegree(transfer * kMaxDegree);
    uint32_t quotient = BinSearchNumber(*this, to_div, 0, kMaxVal);
    result_div.bigint_parts_[transfer] = quotient;
    to_div.MulInt(quotient);
    *this -= to_div;
    this->DeleteStartingZeros();
    to_div = other;
  }

  DivCheckSign(other, result_div);

  return *this;
}

int BigInt::CountDegree(const BigInt& other) const {
  int transfer = 0;
  BigInt to_div = other;

  if (bigint_parts_.size() > other.bigint_parts_.size() + 1) {
    to_div.MulIntTenDegree(
        (bigint_parts_.size() - other.bigint_parts_.size() - 1) * kMaxDegree);
    transfer += (bigint_parts_.size() - other.bigint_parts_.size() - 1);
  }

  if (bigint_parts_.size() > to_div.bigint_parts_.size()) {
    BigInt new_new_new = to_div;
    new_new_new.MulIntTenDegree(kMaxDegree);
    if (new_new_new <= *this) {
      to_div.MulIntTenDegree(kMaxDegree);
      transfer += 1;
    }
  }

  return transfer;
}

BigInt& BigInt::operator%=(const BigInt& other) {
  BigInt a = *this / other;
  a = *this - (a * other);
  bigint_parts_ = a.bigint_parts_;

  this->DeleteStartingZeros();

  if (bigint_parts_[bigint_parts_.size() - 1] == 0) {
    sign_ = Sign::PLUS;
  }

  return *this;
}

void BigInt::PlusHelperLoop(const BigInt& other) {
  uint32_t transfer = 0;

  for (size_t i = 0; i < other.bigint_parts_.size(); i++) {
    uint32_t new_numb = other.bigint_parts_[i] + bigint_parts_[i] + transfer;
    bigint_parts_[i] = new_numb % static_cast<uint32_t>(kMaxVal);
    transfer = new_numb / static_cast<uint32_t>(kMaxVal);
  }

  for (size_t i = other.bigint_parts_.size(); i < bigint_parts_.size(); i++) {
    uint32_t new_numb = bigint_parts_[i] + transfer;
    bigint_parts_[i] = new_numb % static_cast<uint32_t>(kMaxVal);
    transfer = new_numb / static_cast<uint32_t>(kMaxVal);
  }

  if (transfer >= 1) {
    bigint_parts_.push_back(1);
  }
}

BigInt& BigInt::operator+=(const BigInt& other) {
  if (other.sign_ != sign_) {
    sign_ = other.sign_;
    *this -= other;

    if (this->sign_ == Sign::PLUS) {
      this->sign_ = Sign::MINUS;
    } else {
      this->sign_ = Sign::PLUS;
    }
    if ((bigint_parts_.size() == 1) && (bigint_parts_[0] == 0)) {
      this->sign_ = Sign::PLUS;
    }

    return *this;
  }

  IncreaseToSize(other.bigint_parts_.size());

  this->PlusHelperLoop(other);

  return *this;
}

void BigInt::CheckSignMinusEqual(const BigInt& other) {
  sign_ = other.sign_;
  *this += other;

  if (this->sign_ == Sign::PLUS) {
    this->sign_ = Sign::MINUS;
  } else {
    this->sign_ = Sign::PLUS;
  }

  if ((bigint_parts_.size() == 1) && (bigint_parts_[0] == 0)) {
    this->sign_ = Sign::PLUS;
  }
}

void BigInt::MinusCheckSign(long long int transfer, bool flag) {
  if ((transfer == 1) || flag) {
    if (sign_ == Sign::MINUS) {
      sign_ = Sign::PLUS;
    } else {
      sign_ = Sign::MINUS;
    }
  }

  if (bigint_parts_[bigint_parts_.size() - 1] == 0) {
    sign_ = Sign::PLUS;
  }
  this->DeleteStartingZeros();
}

void BigInt::IncreaseToSize(std::size_t new_size) {
  if (bigint_parts_.size() < new_size) {
    for (std::size_t i = 0; i < new_size - bigint_parts_.size(); i++) {
      bigint_parts_.push_back(0);
    }
  }
}

long long BigInt::OperatorMinusPrepare(const BigInt& other, bool& flag_change) {
  size_t real_size = bigint_parts_.size();
  IncreaseToSize(other.bigint_parts_.size());
  long long transfer = 0;

  for (size_t i = 0; i < other.bigint_parts_.size(); i++) {
    uint32_t new_numb =
        bigint_parts_[i] + kMaxVal - other.bigint_parts_[i] - transfer;

    if ((real_size <= i + 1) &&
        (bigint_parts_[i] < other.bigint_parts_[i] + transfer)) {
      new_numb =
          -bigint_parts_[i] + other.bigint_parts_[i] + transfer + kMaxVal;
      flag_change = true;
    }

    bigint_parts_[i] = new_numb % static_cast<uint32_t>(kMaxVal);
    transfer = new_numb / static_cast<uint32_t>(kMaxVal);
    ((transfer == 0) ? transfer = 1 : transfer = 0);
  }

  return transfer;
}

BigInt& BigInt::operator-=(const BigInt& other) {
  if (other.sign_ != sign_) {
    CheckSignMinusEqual(other);
    return *this;
  }

  size_t real_size = bigint_parts_.size();
  bool flag_change = false;
  long long transfer = OperatorMinusPrepare(other, flag_change);

  for (size_t i = other.bigint_parts_.size(); i < bigint_parts_.size(); i++) {
    uint32_t new_numb = bigint_parts_[i] + kMaxVal - transfer;
    if ((real_size <= i + 1) && (bigint_parts_[i] < transfer)) {
      new_numb = -bigint_parts_[i] + kMaxVal + transfer;
      flag_change = true;
    }
    bigint_parts_[i] = new_numb % static_cast<uint32_t>(kMaxVal);
    transfer = new_numb / static_cast<uint32_t>(kMaxVal);
    ((transfer == 0) ? transfer = 1 : transfer = 0);
  }

  MinusCheckSign(transfer, flag_change);

  return *this;
}

std::istream& operator>>(std::istream& stream, BigInt& big_int) {
  std::string str;
  char symb = ' ';

  while ((bool)std::isspace(symb)) {
    symb = static_cast<char>(stream.get());
  }
  while (!(bool)std::isspace(symb) && symb != EOF) {
    str += symb;
    symb = static_cast<char>(stream.get());
  }

  big_int = BigInt(str);

  return stream;
}

std::ostream& operator<<(std::ostream& stream, const BigInt& big_int) {
  if (big_int.sign_ == BigInt::Sign::MINUS) {
    stream << '-';
  }

  stream << (big_int.GetBigIntParts())[big_int.BigIntPartsSize() - 1];

  for (int i = big_int.BigIntPartsSize() - 2; i > -1; i--) {
    int len = BigInt::CountSignificantNumb(big_int.GetBigIntParts()[i]);

    for (int j = 0; j < BigInt::kMaxDegree - len; j++) {
      stream << 0;
    }
    stream << big_int.GetBigIntParts()[i];
  }

  return stream;
}