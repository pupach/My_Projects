#pragma once
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>

template <typename T, typename Allocator = std::allocator<T>>
class List {
 private:
  class BaseNode {
   public:
    BaseNode* left;
    BaseNode* right;
    BaseNode() : left(this), right(this) {}
  };

  struct Node : public BaseNode {
    T data;

    Node() = default;
    Node(const T& item) : data(item), BaseNode() {}
  };

  using allocator_type_t = typename std::allocator_traits<Allocator>;
  using node_allocator =
      typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
  using node_allocator_traits =
      typename std::allocator_traits<Allocator>::template rebind_traits<Node>;

  std::size_t list_size_;
  List::node_allocator node_alloc_;
  BaseNode main_node_;
  Allocator alloc_;

  void error_deadlock(BaseNode* begin_node, std::size_t numb) {
    Node* new_node;
    for (std::size_t i = 0; i < numb; i++) {
      new_node = (Node*)begin_node->right;
      destroy_node((Node*)begin_node);
      begin_node = new_node;
    }
  }

  void destroy_node(Node* ptr) {
    List::node_allocator_traits ::destroy(node_alloc_, ptr);
    node_alloc_.deallocate(ptr, static_cast<std::size_t>(1));
  }

  Node* allocate_node() {
    Node* new_node = List::node_allocator_traits ::allocate(node_alloc_, 1);
    new_node->left = nullptr;
    new_node->right = nullptr;
    return new_node;
  }

  Node* create_node() {
    Node* new_node = allocate_node();
    try {
      List::node_allocator_traits ::construct(node_alloc_, new_node);
    } catch (...) {
      node_alloc_.deallocate(new_node, static_cast<std::size_t>(1));
      throw;
    }
    return new_node;
  }

  Node* create_node(T&& elem) {
    Node* new_node = allocate_node();
    try {
      List::node_allocator_traits ::construct(node_alloc_, new_node,
                                              std::move(elem));
    } catch (...) {
      node_alloc_.deallocate(new_node, static_cast<std::size_t>(1));
      throw;
    }
    return new_node;
  }

  Node* create_node(const T& elem) {
    Node* new_node = allocate_node();
    try {
      List::node_allocator_traits ::construct(node_alloc_, new_node, elem);
    } catch (...) {
      node_alloc_.deallocate(new_node, static_cast<std::size_t>(1));
      throw;
    }
    return new_node;
  }

  void equal_node(Node* first, Node* second) {
    first->left = second->left;
    first->right = second->right;
    List::allocator_type_t::destroy(node_alloc_, &(first->data));
    List::allocator_type_t::construct(node_alloc_, &(first->data),
                                      second->data);
  }

  const Allocator& define_new_alloc(const List& other) {
    if (List::node_allocator_traits::propagate_on_container_copy_assignment::
            value) {
      return other.alloc_;
    }
    return alloc_;
  }

  void if_allocators_equal(const List& other) {
    std::size_t start_size = list_size_;
    alloc_ = this->define_new_alloc(other);

    const_iterator other_it = other.cbegin();
    iterator this_it = begin();
    try {
      while ((this_it != end()) && (other_it != other.cend())) {
        *this_it = *other_it;
        this_it++;
        other_it++;
      }
      if (other_it != other.cend()) {
        while (other_it != other.cend()) {
          Node* new_node = create_node(*other_it);
          insert(this_it, new_node);
          other_it++;
        }
      } else {
        while (list_size_ != other.size()) {
          this->pop_back();
        }
      }
    } catch (...) {
      while (list_size_ > start_size) {
        pop_back();
      }
      throw;
    }
  }

  void if_allocators_not_equal(const List& other) {
    std::size_t start_size = list_size_;
    const Allocator& old_alloc = alloc_;
    const Allocator& new_alloc = this->define_new_alloc(other);

    const_iterator other_it = other.cbegin();
    const_iterator this_it = cbegin();
    try {
      alloc_ = new_alloc;
      while (other_it != other.cend()) {
        Node* new_node = create_node(*other_it);
        insert(this_it, new_node);
        other_it++;
      }

      alloc_ = old_alloc;
      while (list_size_ != other.size()) {
        pop_back();
      }

      alloc_ = new_alloc;
    } catch (...) {
      alloc_ = new_alloc;
      while (list_size_ > start_size) {
        pop_front();
      }
      throw;
    }
  }

  template <bool IsConst = false>
  struct ListIterator {
   public:
    BaseNode* ptr_node;

    using value_type = T;
    using pointer = typename std::conditional<IsConst, const T*, T*>::type;
    using reference = typename std::conditional<IsConst, const T&, T&>::type;
    using size_type = std::size_t;
    using difference_type = std::size_t;
    using const_reference = const T&;
    using iterator_category = std::bidirectional_iterator_tag;

    ListIterator() = default;
    ListIterator(const ListIterator<IsConst>& other) {
      this->ptr_node = other.ptr_node;
    };
    ListIterator(BaseNode* ptr) : ptr_node(ptr) {}

    ListIterator<IsConst> operator=(const ListIterator<IsConst>& other) {
      this->ptr_data = other.ptr_data;
      return *this;
    }

    reference operator*() { return static_cast<Node*>(ptr_node)->data; }

    T* operator->() { return &(static_cast<Node*>(ptr_node)->data); }

    bool operator==(const ListIterator<IsConst>& other) {
      return this->ptr_node == other.ptr_node;
    }

    bool operator!=(const ListIterator<IsConst>& other) {
      return this->ptr_node != other.ptr_node;
    }

    ListIterator<IsConst> operator++(int) {
      ptr_node = ptr_node->right;
      return ListIterator<IsConst>(ptr_node->left);
    }

    ListIterator<IsConst>& operator++() {
      ptr_node = ptr_node->right;
      return *this;
    }

    ListIterator<IsConst> operator--(int) {
      ptr_node = ptr_node->left;
      return ListIterator<IsConst>(ptr_node->right);
    }

    ListIterator<IsConst>& operator--() {
      ptr_node = ptr_node->left;
      return *this;
    }
  };

 public:
  using iterator = ListIterator<false>;
  using value_type = T;
  using allocator_type = Allocator;
  using const_iterator = ListIterator<true>;

  Allocator& get_allocator() { return alloc_; }
  const Allocator& get_allocator() const { return alloc_; }

  List() : main_node_() {}
  List(std::size_t count, const T& value, const Allocator& alloc = Allocator())
      : main_node_(), alloc_(alloc), list_size_(0) {
    Node* new_node;

    for (std::size_t i = 0; i < count; i++) {
      try {
        new_node = create_node(value);
      } catch (...) {
        error_deadlock(main_node_.right, i);
        throw;
      }
      this->insert(this->end(), new_node);
    }
  }
  explicit List(size_t count, const Allocator& alloc = Allocator())
      : main_node_() {
    this->alloc_ = alloc;
    list_size_ = 0;
    Node* new_node;

    for (std::size_t i = 0; i < count; i++) {
      try {
        new_node = create_node();
      } catch (...) {
        error_deadlock(main_node_.right, i);
        throw;
      }
      this->insert(this->end(), new_node);
    }
  }

  List(const List& other)
      : alloc_(List::allocator_type_t::select_on_container_copy_construction(
            other.get_allocator())) {
    list_size_ = 0;
    Node* new_node;
    auto iter = other.cbegin();
    for (std::size_t i = 0; i < other.size(); i++, iter++) {
      try {
        new_node = create_node(*iter);
      } catch (...) {
        error_deadlock(main_node_.right, i);
        throw;
      }
      this->insert(this->end(), new_node);
    }
  }

  List(std::initializer_list<T> init, const Allocator& alloc = Allocator()) {
    list_size_ = 0;
    this->alloc_ = alloc;
    Node* new_node;
    const T* iterator_init = init.begin();
    for (std::size_t i = 0; i < init.size(); i++) {
      try {
        new_node = create_node(*iterator_init);
      } catch (...) {
        error_deadlock(main_node_.right, i);
        throw;
      }
      this->insert(this->end(), new_node);
      iterator_init++;
    }
  }

  ~List() {
    for (std::size_t i = 0; i < list_size_;) {
      this->pop_back();
    }
  }

  List& operator=(const List& other) {
    if (&other == this) {
      return *this;
    }
    if ((alloc_ == other.alloc_)) {
      if_allocators_equal(other);
    } else {
      if_allocators_not_equal(other);
    }
    return *this;
  }

  void insert(const_iterator iter, Node* new_node) {
    new_node->left = iter.ptr_node->left;
    new_node->right = iter.ptr_node;
    new_node->left->right = new_node;
    new_node->right->left = new_node;
    list_size_++;
  }

  void insert(iterator iter, Node* new_node) {
    new_node->left = iter.ptr_node->left;
    new_node->right = iter.ptr_node;
    new_node->left->right = new_node;
    new_node->right->left = new_node;
    list_size_++;
  }

  iterator begin() { return ListIterator<false>(main_node_.right); }

  const_iterator begin() const {
    return ListIterator<true>(const_cast<BaseNode*>(main_node_.right));
  }

  const_iterator cbegin() const {
    return ListIterator<true>(const_cast<BaseNode*>(main_node_.right));
  }

  iterator end() { return ListIterator<false>((&main_node_)); }

  const_iterator end() const {
    return ListIterator<true>(const_cast<BaseNode*>(&main_node_));
  }

  const_iterator cend() const {
    return ListIterator<true>(const_cast<BaseNode*>(&main_node_));
  }

  iterator rbegin() { return ListIterator<false>(&main_node_); }

  const_iterator rbegin() const {
    return ListIterator<true>(const_cast<const BaseNode*>(&main_node_));
  }

  iterator rend() { return ListIterator<false>(main_node_.right); }

  const_iterator rend() const {
    return ListIterator<true>(const_cast<BaseNode*>(main_node_.right));
  }

  const_iterator crbegin() const {
    return ListIterator<true>(const_cast<BaseNode*>(&main_node_));
  }

  const_iterator crend() const {
    return ListIterator<true>(const_cast<BaseNode*>(main_node_.right));
  }

  T& front() { return (main_node_.left)->data; }
  const T& front() const { return (main_node_.left)->data; }
  T& back() { return (main_node_.right)->data; }
  const T& back() const { return (main_node_.right)->data; }
  bool empty() { return (list_size_ == 0); }
  std::size_t size() const { return list_size_; }
  void push_back(const T& elem) {
    Node* new_node = create_node(elem);
    insert(end(), new_node);
  }
  void push_back(T&& elem) {
    Node* new_node = create_node(std::move(elem));
    insert(end(), new_node);
  }
  void push_front(const T& elem) {
    Node* new_node = create_node(elem);
    insert(begin(), new_node);
  }
  void push_front(T&& elem) {
    Node* new_node = create_node(std::move(elem));
    insert(begin(), new_node);
  }
  void pop_back() {
    list_size_--;
    Node* to_destroy = static_cast<Node*>(main_node_.left);
    (main_node_.left)->left->right = &main_node_;
    main_node_.left = (main_node_.left)->left;
    destroy_node(to_destroy);
  }
  void pop_front() {
    list_size_--;
    Node* to_destroy = static_cast<Node*>(main_node_.right);
    (main_node_.right)->right->left = &main_node_;
    main_node_.right = (main_node_.right)->right;
    destroy_node(to_destroy);
  }
};
