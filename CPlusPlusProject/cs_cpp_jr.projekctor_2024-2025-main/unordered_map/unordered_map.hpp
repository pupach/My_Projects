#pragma once

#include <array>
#include <cstdio>
#include <cstdlib>
#include <forward_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <vector>

const std::size_t kBeginCapacity = 1000000;

template <typename Key, typename Value, class Hash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>,
          class Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap;

template <typename T, class Allocator, typename Key, typename Value, class Hash,
          class KeyEqual, class Alloc>
class MyForwardList : public std::forward_list<T, Allocator> {
 public:
  friend UnorderedMap<Key, Value, Hash, KeyEqual, Alloc>;
};

template <typename Key, typename Value, class Hash, class KeyEqual, class Alloc>
class UnorderedMap {
 private:
  using node_type = std::pair<std::size_t, std::pair<const Key, Value>>;
  using unconst_node_type = std::pair<std::size_t, std::pair<Key, Value>>;
  using list_type = MyForwardList<
      node_type,
      typename std::allocator_traits<Alloc>::template rebind_alloc<node_type>,
      Key, Value, Hash, KeyEqual, Alloc>;

 public:
  list_type list;
  double max_load_factor = 2;
  std::size_t size_list = 0;

  using key_type = Key;
  using mapped_type = Value;
  using value_type = std::pair<const Key, Value>;
  using unconst_value_type = std::pair<Key, Value>;
  using const_value_type = std::pair<const Key, const Value>;
  using unconst_alloc = typename std::allocator_traits<
      Alloc>::template rebind_alloc<unconst_value_type>;

  using const_alloc = typename std::allocator_traits<Alloc>;

  using allocator_type = typename std::allocator_traits<unconst_alloc>;
  using vector_allocator_type = typename std::allocator_traits<
      Alloc>::template rebind_alloc<typename list_type::iterator>;
  using vector_type =
      std::vector<typename list_type::iterator, vector_allocator_type>;

  vector_type table;
  unconst_alloc alloc;

  class UnorderedMapIterator : public list_type::iterator {
   public:
    using value_type = UnorderedMap::value_type;
    using pointer = value_type*;
    using reference = UnorderedMap::value_type&;
    using size_type = std::size_t;
    using difference_type = std::size_t;
    using const_reference = const UnorderedMap::value_type&;

    UnorderedMapIterator operator++(int) {
      return static_cast<UnorderedMapIterator>(
          list_type::iterator::operator++(1));
    }

    UnorderedMapIterator& operator++() {
      list_type::iterator::operator++();
      return *this;
    }

    value_type* operator->() const {
      return &(list_type::iterator::operator->()->second);
    }

    value_type& operator*() const {
      return (list_type::iterator::operator->()->second);
    }
  };
  class UnorderedMapConstIterator : public list_type::const_iterator {
   public:
    using value_type = UnorderedMap::value_type;
    using pointer = const value_type*;
    using reference = const UnorderedMap::value_type&;
    using size_type = std::size_t;
    using difference_type = std::size_t;
    using const_reference = const UnorderedMap::value_type&;

    const value_type* operator->() const {
      return &(list_type::const_iterator::operator->()->second);
    }

    value_type& operator*() const {
      return (list_type::const_iterator::operator->()->second);
    }
  };

  using iterator = UnorderedMapIterator;
  using const_iterator = UnorderedMapConstIterator;

  UnorderedMap() : list(), table(kBeginCapacity, list.before_begin()) {}
  UnorderedMap(std::size_t count) : list(), table(count, list.before_begin()) {}

  UnorderedMap(const UnorderedMap& other)
      : list(other.list), table(other.table.size()) {
    alloc = std::allocator_traits<unconst_alloc>::
        select_on_container_copy_construction(other.get_allocator());

    auto prev_it = list.before_begin();
    auto cur_it = prev_it;
    auto last_hash = list.before_begin();
    cur_it++;
    while (cur_it != list.end()) {
      if (prev_it != list.before_begin()) {
        if (!equal_hash(prev_it->first, cur_it->first)) {
          last_hash = prev_it;
        }
      }
      iter_by_hash(cur_it->first) = last_hash;
      cur_it++;
      prev_it++;
    }
  }

  UnorderedMap(UnorderedMap&& other) {
    alloc = std::move(other.alloc);
    list = std::move(other.list);
    table = std::move(other.table);
    table = vector_type(table.size(), list.before_begin());

    auto prev_it = list.before_begin();
    auto cur_it = prev_it;
    auto last_hash = list.before_begin();
    cur_it++;
    while (cur_it != list.end()) {
      if (prev_it != list.before_begin()) {
        if (!equal_hash(prev_it->first, cur_it->first)) {
          last_hash = prev_it;
        }
      }
      iter_by_hash(cur_it->first) = last_hash;
      cur_it++;
      prev_it++;
    }
  }

  UnorderedMap& operator=(const UnorderedMap& other) {
    if (std::allocator_traits<
            allocator_type>::propagate_on_container_copy_assignment::value) {
      alloc = other.alloc;
    }
    list = other.list;
    table = other.table;
    table = vector_type(table.size(), list.before_begin());

    auto prev_it = list.before_begin();
    auto cur_it = prev_it;
    auto last_hash = list.before_begin();
    cur_it++;
    while (cur_it != list.end()) {
      if (prev_it != list.before_begin()) {
        if (!equal_hash(prev_it->first, cur_it->first)) {
          last_hash = prev_it;
        }
      }
      iter_by_hash(cur_it->first) = last_hash;
      cur_it++;
      prev_it++;
    }

    // ALLOC
    return *this;
  }

  UnorderedMap& operator=(UnorderedMap&& other) {
    if (std::allocator_traits<
            allocator_type>::propagate_on_container_move_assignment::value) {
      alloc = other.alloc;
    }
    list.clear();
    table.clear();

    alloc = other.alloc;
    list = std::move(other.list);
    table = std::move(other.table);
    table = vector_type(table.size(), list.before_begin());
    auto prev_it = list.before_begin();
    auto cur_it = prev_it;
    auto last_hash = list.before_begin();
    cur_it++;
    while (cur_it != list.end()) {
      if (prev_it != list.before_begin()) {
        if (!equal_hash(prev_it->first, cur_it->first)) {
          last_hash = prev_it;
        }
      }
      iter_by_hash(cur_it->first) = last_hash;
      cur_it++;
      prev_it++;
    }

    // ALLOC
    return *this;
  }

  std::pair<iterator, bool> check_not_exists(const Key& key, std::size_t hash) {
    if (table.empty()) {
      return {UnorderedMapIterator(list.before_begin()), true};
    }
    typename list_type::iterator iter(iter_by_hash(hash));
    // std::cerr << hash % table.size() << "\n";
    if (size() < 1) {
      return {UnorderedMapIterator(list.before_begin()), true};
    }
    // auto it_prev = iter;
    iter++;
    while ((iter->first % table.size()) == (hash % table.size())) {
      // std::cerr << key << " " << iter->second.first  << "\n";
      if (KeyEqual{}(key, iter->second.first)) {
        return {UnorderedMapIterator(iter), false};
      }
      // it_prev = iter;
      iter++;
      if (iter == list.end()) {
        break;
      }
    }
    return {UnorderedMapIterator(iter), true};
  }

  unconst_alloc get_allocator() const noexcept { return alloc; }

  std::pair<iterator, bool> insert(const unconst_value_type& val) {
    return emplace(val.first, val.second);
  }

  std::pair<iterator, bool> insert(unconst_value_type&& val) {
    return emplace(std::move(val));
  }

  template <class InputIt>
  void insert(InputIt first, InputIt last) {
    while (first != last) {
      // std::cerr << "insert begin std::distance(first, last) = " <<
      // std::distance(first, last) << "\n";
      insert(*first);
      // std::cerr << "insert end\n";
      first++;
    }
  }

  template <typename... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    auto ret = try_emplace(std::forward<Args>(args)...);
    if (ret.second) {
      size_list++;
    }
    check_hash();
    return ret;
  }

  std::pair<iterator, bool> emplace_in_begin(
      unconst_value_type* pairs, std::size_t new_hash,
      typename list_type::iterator iter) {
    std::pair<iterator, bool> ret(iterator(), true);
    auto next = iter;
    next++;

    if (((size() > 0) && equal_hash(new_hash, next->first))) {
      ret = check_not_exists(pairs->first, new_hash);
    }
    if (ret.second) {
      iter = list.emplace_after(iter, std::move(new_hash), std::move(*pairs));
      typename list_type::iterator prev = iter;
      if (size() > 0) {
        iter++;
        if (!equal_hash(iter->first, new_hash)) {
          iter_by_hash((*(iter)).first) = prev;
        }
      }
      ret = {UnorderedMapIterator(prev), true};
    }
    return ret;
  }

  template <typename... Args>
  std::pair<iterator, bool> try_emplace(Args&&... args) {
    std::pair<iterator, bool> ret(iterator(), true);

    unconst_value_type* pairs = allocator_type::allocate(alloc, 1);
    allocator_type ::construct(alloc, pairs, std::forward<Args>(args)...);
    std::size_t new_hash = Hash{}(pairs->first);
    typename list_type::iterator iter(iter_by_hash(new_hash));

    if ((iter_by_hash(new_hash) == list.before_begin())) {
      ret = emplace_in_begin(pairs, new_hash, iter);
    } else {
      ret = check_not_exists(pairs->first, new_hash);
      if (ret.second) {
        ret.first = UnorderedMapIterator(
            list.emplace_after(iter, std::move(new_hash), std::move(*pairs)));
        ret.second = true;
      }
    }
    allocator_type::deallocate(alloc, pairs, 1);
    return ret;
  }

  std::pair<iterator, bool> try_emplace(unconst_value_type&& pairs) {
    std::size_t new_hash = Hash{}(pairs.first);
    typename list_type::iterator iter(iter_by_hash(new_hash));
    if ((iter_by_hash(new_hash) == list.before_begin())) {
      return emplace_in_begin(&pairs, new_hash, iter);
    }
    std::pair<iterator, bool> ret = check_not_exists(pairs.first, new_hash);
    if (!ret.second) {
      return ret;
    }
    ret.first = UnorderedMapIterator(
        list.emplace_after(iter, std::move(new_hash), std::move(pairs)));
    ret.second = true;
    return ret;
  }

  Value& at(const Key& key) {
    // std::cerr << "begin at key =" << key << "\n";
    auto ret = check_not_exists(key, Hash{}(key));
    // ret.first++;
    if (!ret.second) {
      return (ret.first)->second;
    }

    throw std::out_of_range("1");
  }

  const Value& at(const Key& key) const {
    // std::cerr << "begin at key =" << key << "\n";
    auto ret = check_not_exists(key, Hash{}(key));
    // ret.first++;
    if (!ret.second) {
      return *(ret.first)->second;
    }

    throw std::out_of_range("1");
  }

  std::size_t size() { return size_list; }

  Value& operator[](const Key& key) {
    auto ret = check_not_exists(key, Hash{}(key));
    // ret.first++;
    if (ret.second) {
      // std::cerr << "insert begin\n";
      ret = insert({key, Value()});
      // std::cerr << "insert end\n";
    }
    std::cerr << (ret.first)->second << "h\n";

    return (ret.first)->second;
  }

  Value& operator[](Key&& key) {
    auto ret = check_not_exists(std::move(key), Hash{}(std::move(key)));
    // ret.first++;
    if (ret.second) {
      // std::cerr << "insert begin\n";
      ret = insert({std::move(key), Value()});
      // std::cerr << "insert end\n";
    }
    // std::cerr << (ret.first)->second << "h\n";

    return ((ret.first))->second;
  }

  typename list_type::iterator& iter_by_hash(std::size_t hash) {
    return table[hash % table.size()];
  }

  bool equal_hash(std::size_t hash1, std::size_t hash2) {
    return (hash1 % table.size()) == (hash2 % table.size());
  }

  iterator erase(iterator pos) {
    typename list_type::iterator previuos =
        iter_by_hash(static_cast<typename list_type::iterator>(pos)->first);
    typename list_type::iterator prev = previuos;
    // std::cerr << static_cast<_list_type::iterator>(pos)->first << "\n";
    // std::cerr << "equality iterators = " << (_list.before_begin() ==
    // previuos)  << " distance = " << std::distance( _list.before_begin(),
    // _list.end()) << "\n";

    while (previuos != pos) {
      prev = previuos;
      previuos++;
    }
    // std::cerr << "erase val = " << previuos->second.first << " expect erase =
    // "  << pos->first << "equality iterators = " << (previuos == pos)  << "
    // distance = " << std::distance(previuos, _list.end()) << "\n";
    typename list_type::iterator next = previuos;
    next++;
    // std::cerr << " distance = " << std::distance(previuos, _list.end()) <<
    // "\n";
    if (next != list.end()) {
      if (!equal_hash(next->first, previuos->first)) {
        iter_by_hash(next->first) = prev;
      }
    }
    if ((next == list.end())) {
      if ((prev == list.before_begin()) ||
          (!equal_hash(prev->first, previuos->first))) {
        iter_by_hash(previuos->first) = list.before_begin();
      }
    } else {
      if ((prev == list.before_begin()) ||
          (!equal_hash(prev->first, previuos->first))) {
        if (!equal_hash(next->first, previuos->first)) {
          iter_by_hash(previuos->first) = list.before_begin();
        }
      }
    }
    size_list--;
    return UnorderedMapIterator(list.erase_after(prev));
  }

  iterator erase(const_iterator pos) {
    typename list_type::iterator previuos =
        iter_by_hash(static_cast<typename list_type::iterator>(pos)->first);
    typename list_type::iterator prev = previuos;
    while ((previuos++) != pos) {
      prev = previuos;
    }
    // std::cerr << "erase val = " << previuos->second.first << " expect erase =
    // "  << pos->first << "\n";
    typename list_type::iterator next = previuos++;
    if (next != list.end()) {
      if (!equal_hash(next->first, previuos->first)) {
        iter_by_hash(next->first) = prev;
      }
    }
    if ((next == list.end())) {
      if ((prev == list.before_begin()) ||
          (!equal_hash(prev->first, previuos->first))) {
        iter_by_hash(previuos->first) = list.before_begin();
      }
    } else {
      if ((prev == list.before_begin()) ||
          (!equal_hash(prev->first, previuos->first))) {
        if (!equal_hash(next->first, previuos->first)) {
          iter_by_hash(previuos->first) = list.before_begin();
        }
      }
    }
    size_list--;
    return UnorderedMapIterator(list.erase_after(prev));
  }

  iterator erase(const_iterator first, const_iterator last) {
    iterator iter;
    while (first != last) {
      first = erase(first);
    }
    return iter;
  }

  iterator erase(iterator first, iterator last) {
    iterator iter;
    while (first != last) {
      first = erase(first);
    }
    return iter;
  }

  std::size_t erase(const Key& key) {
    auto ret = check_not_exists(key, Hash{}(key));
    if (!ret.second) {
      erase(ret.first);
      return 1;
    }
    return 0;
  }

  iterator find(const Key& key) {
    std::pair<iterator, bool> ret = check_not_exists(key, Hash{}(key));
    if (ret.second) {
      return end();
    }
    return ret.first;
  }
  const_iterator find(const Key& key) const {
    std::pair<iterator, bool> ret = check_not_exists(key, Hash{}(key));
    if (ret.second) {
      return end();
    }
    return ret.first;
  }

  iterator begin() noexcept { return UnorderedMapIterator(list.begin()); }
  const_iterator begin() const noexcept {
    return UnorderedMapConstIterator(list.begin());
  }
  const_iterator cbegin() const noexcept {
    return UnorderedMapConstIterator(list.cbegin());
  }

  iterator end() noexcept { return UnorderedMapIterator(list.end()); }
  const_iterator end() const noexcept {
    return UnorderedMapConstIterator(list.end());
  }
  const_iterator cend() const noexcept {
    return UnorderedMapConstIterator(list.cend());
  }

  void reserve(std::size_t count) { rehash(count); }

  void check_hash() {
    std::size_t count = 0;

    if ((int)table.size() < (int)(((double)size()) / max_load_factor) - 1) {
      count = (std::size_t)(((double)size()) / max_load_factor) * 2 + 2;
    } else {
      return;
    }

    rehash(count);
  }

  void rehash(std::size_t count) {
    if (count < (std::size_t)(((double)size()) / max_load_factor) - 1) {
      count = (std::size_t)(((double)size()) / max_load_factor) * 2 + 2;
    }
    UnorderedMap<Key, Value, Hash, KeyEqual, Alloc> new_map(count);
    auto iter = list.begin();
    auto prev = iter;

    while (iter != list.end()) {
      iter++;
      auto casted = reinterpret_cast<std::pair<Key, Value>*>(&prev->second);
      new_map.emplace(std::move(*casted));
      prev = iter;
    }

    *this = std::move(new_map);
  }

  double get_max_load_factor() const { return max_load_factor; }

  void set_max_load_factor(double new_max_load_factor) {
    max_load_factor = new_max_load_factor;
  }
};