#include <algorithm>
#include <array>
#include <compare>
#include <iostream>

#define SOLVE(l1, l2, l3, l4, l5, l6, l7, l8, l9)                              \
  recr(elem{std::array<int, 3>{l1, l2, l3}, std::array<int, 3>{l4, l5, l6},    \
            std::array<int, 3>{l7, l8, l9}});

constexpr std::size_t MAX_AMOUNT_VARIANT_1 = 530'017;

constexpr std::size_t MAX_AMOUNT_VARIANT_2 = 1'123'701;

constexpr std::size_t absC(int f) { return f > 0 ? f : -f; }

constexpr std::size_t manh_rast(int x1, int y1, int x2, int y2) {
  return absC(x1 - x2) + absC(y1 - y2);
}

using coor = std::pair<int, int>;
using elem = std::array<std::array<int, 3>, 3>;

std::ostream &operator<<(std::ostream &os, elem &el) {
  os << el[0][0] << " " << el[0][1] << " " << el[0][2] << "\n"
     << el[1][0] << " " << el[1][1] << " " << el[1][2] << "\n"
     << el[2][0] << " " << el[2][1] << " " << el[2][2] << "\n";
  return os;
}

constexpr std::array<coor, 9> dest = {coor{2, 2}, coor{0, 0}, coor{0, 1},
                                      coor{0, 2}, coor{1, 0}, coor{1, 1},
                                      coor{1, 2}, coor{2, 0}, coor{2, 1}};

constexpr long long pow10(int pow) {
  long long res = 1;
  for (auto i = 0; i < pow; i++) {
    res *= 10;
  }
  return res;
}

template <std::size_t MOD> constexpr long long hash_func(elem el) {
  long long ret = 0;
  for (auto i = 0; i < 3; i++) {
    for (auto j = 0; j < 3; j++) {
      ret += pow10(i * 3 + j) * el[i][j];
      ret %= MOD;
    }
  }
  return ret;
}

struct Heap {
  using elem_heap = std::tuple<int, int, elem, coor>;

  [[maybe_unused]] static constexpr auto cmp = [](const elem_heap &a,
                                                  const elem_heap &b) {
    return (std::get<0>(a) + std::get<1>(a)) >
           (std::get<0>(b) + std::get<1>(b));
  };

  using array_for_heap = std::array<elem_heap, 7000>;

  array_for_heap _heap = {};
  std::size_t size_heap = 0;
  std::array<bool, MAX_AMOUNT_VARIANT_1> hash_table_1 = {false};
  std::array<bool, MAX_AMOUNT_VARIANT_2> hash_table_2 = {false};

  constexpr void push(elem_heap el) {
    auto [cur_rast, last_rast, cur_pos, cur_null] = el;
    if (!hash_table_1[hash_func<MAX_AMOUNT_VARIANT_1>(cur_pos)] ||
        !hash_table_2[hash_func<MAX_AMOUNT_VARIANT_2>(cur_pos)]) {
      hash_table_1[hash_func<MAX_AMOUNT_VARIANT_1>(cur_pos)] = true;
      hash_table_2[hash_func<MAX_AMOUNT_VARIANT_2>(cur_pos)] = true;
      if (size_heap >= 20000) {
        _heap[size_heap - 1] = el;
      } else {
        _heap[size_heap] = el;
        size_heap++;
      }

      std::push_heap(_heap.begin(), _heap.begin() + size_heap, cmp);
    }
  }

  constexpr elem_heap pop() {
    std::pop_heap(_heap.begin(), _heap.begin() + size_heap, cmp);
    --size_heap;
    return _heap[size_heap]; // ?????????
  }
};

constexpr std::size_t global_rast(const elem &from) {
  std::size_t res = 0;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (from[i][j] != 0) {
        res += manh_rast(i, j, dest[from[i][j]].first, dest[from[i][j]].second);
      }
    }
  }

  return res;
}

constexpr bool IsImposible(elem &begin_pos) {
  std::array<int, 9> pos = {};
  for (auto i = 0; i < 3; i++) {
    for (auto j = 0; j < 3; j++) {
      pos[i * 3 + j] = begin_pos[i][j];
    }
  }
  std::size_t inv = 0;
  for (auto i = 0; i < 9; i++) {
    for (auto j = i; j < 9; j++) {
      if ((pos[i] == 0) || (pos[j] == 0)) {
        continue;
      }
      if (pos[i] > pos[j])
        inv++;
    }
  }

  return (inv % 2) == 1;
}

constexpr bool IsInvalid(elem &begin_pos) {
  std::array<bool, 9> pos = {};
  for (auto i = 0; i < 3; i++) {
    for (auto j = 0; j < 3; j++) {
      if (begin_pos[i][j] >= 9)
        return true;
      if (pos[begin_pos[i][j]])
        return true;
      pos[begin_pos[i][j]] = 1;
    }
  }
  return false;
}

constexpr std::size_t A_star(Heap &heap) {
  while (true) {
    auto [cur_rast, last_rast, cur_pos, cur_null] = heap.pop();
    if (last_rast == 0)
      return cur_rast;

    if (cur_null.first != 0) {
      auto new_pos = cur_pos;
      std::swap(new_pos[cur_null.first][cur_null.second],
                new_pos[cur_null.first - 1][cur_null.second]);

      heap.push({cur_rast + 1,
                 (int)global_rast(new_pos),
                 new_pos,
                 {cur_null.first - 1, cur_null.second}});
    }

    if (cur_null.first != 2) {
      auto new_pos = cur_pos;
      std::swap(new_pos[cur_null.first][cur_null.second],
                new_pos[cur_null.first + 1][cur_null.second]);

      heap.push({cur_rast + 1,
                 (int)global_rast(new_pos),
                 new_pos,
                 {cur_null.first + 1, cur_null.second}});
    }

    if (cur_null.second != 0) {
      auto new_pos = cur_pos;
      std::swap(new_pos[cur_null.first][cur_null.second],
                new_pos[cur_null.first][cur_null.second - 1]);

      heap.push({cur_rast + 1,
                 (int)global_rast(new_pos),
                 new_pos,
                 {cur_null.first, cur_null.second - 1}});
    }

    if (cur_null.second != 2) {
      auto new_pos = cur_pos;
      std::swap(new_pos[cur_null.first][cur_null.second],
                new_pos[cur_null.first][cur_null.second + 1]);

      heap.push({cur_rast + 1,
                 (int)global_rast(new_pos),
                 new_pos,
                 {cur_null.first, cur_null.second + 1}});
    }
  }
}

constexpr int recr(elem el) {
  if (IsInvalid(el))
    return -2;
  if (IsImposible(el))
    return -1;

  Heap heap = {};

  coor cur_nul = {};
  for (auto i = 0; i < 3; i++) {
    for (auto j = 0; j < 3; j++) {
      if (el[i][j] == 0) {
        cur_nul.first = i;
        cur_nul.second = j;
      }
    }
  }
  heap.push({0, global_rast(el), el, cur_nul});
  return (int)A_star(heap);
}