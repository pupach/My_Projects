#pragma once
#include <cstddef>
#include <memory>

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

struct ControlBlockBase {
 private:
  template <typename T>
  friend class SharedPtr;

  template <typename T>
  friend class WeakPtr;

  template <typename T, typename Alloc, typename... Args>
  friend SharedPtr<T> AllocateShared(const Alloc& alloc, Args&&... args);

  std::size_t shared_cnt_ = 0;
  std::size_t weak_cnt_ = 0;

 public:
  ControlBlockBase() = default;

  virtual ~ControlBlockBase() {};
  virtual void destroy_obj() {};
  virtual void destroy_class() {};
};

template <typename T, typename Deleter = std::default_delete<T>,
          typename Alloc = std::allocator<T>>
struct ControlBlockRegular : virtual public ControlBlockBase {
  T* ptr_data;
  Deleter del;
  [[no_unique_address]] Alloc alloc = Alloc();

  ControlBlockRegular(T* ptr = nullptr) : ptr_data(ptr) {}

  ControlBlockRegular(T* ptr, Deleter del, Alloc alloc = Alloc())
      : del(del), alloc(alloc) {
    ptr_data = ptr;
  }

  virtual ~ControlBlockRegular() noexcept = default;

  void destroy_obj() noexcept override {
    del(ptr_data);
    ptr_data = nullptr;
  }

  void destroy_class() noexcept override {
    using block_alloc = typename std::allocator_traits<
        Alloc>::template rebind_alloc<ControlBlockRegular<T, Deleter, Alloc>>;
    block_alloc block_allocator = alloc;

    alloc.~Alloc();
    std::allocator_traits<block_alloc>::deallocate(block_allocator, this, 1);
  }
};

template <typename T, typename Alloc = std::allocator<T>>
struct ControlBlockShared : virtual public ControlBlockBase {
  T data;
  Alloc alloc;

  template <typename... Args>
  ControlBlockShared(Alloc alloc, Args&&... args)
      : data(std::forward<Args>(args)...), ControlBlockBase(), alloc(alloc) {}

  virtual ~ControlBlockShared() noexcept = default;

  void destroy_obj() override {
    std::allocator_traits<Alloc>::destroy(alloc, &data);
  }

  void destroy_class() override {
    using block_alloc = typename std::allocator_traits<
        Alloc>::template rebind_alloc<ControlBlockShared<T, Alloc>>;
    block_alloc block_allocator = alloc;
    std::allocator_traits<block_alloc>::deallocate(block_allocator, this, 1);
  }
};

template <typename T>
class SharedPtr {
 private:
  template <typename OtherType>
  friend class SharedPtr;

  template <typename OtherType>
  friend class WeakPtr;

  template <typename Y, typename Alloc, typename... Args>
  friend SharedPtr<Y> AllocateShared(const Alloc& alloc, Args&&... args);

  T* data_ptr_ = nullptr;

  ControlBlockBase* block_ptr_ = nullptr;

  template <typename Y>
  SharedPtr& assign_from(const SharedPtr<Y>& shared_p) noexcept {
    if (static_cast<const void*>(this) != static_cast<const void*>(&shared_p)) {
      decrease_shared_cnt();
      block_ptr_ = shared_p.block_ptr_;
      data_ptr_ = shared_p.data_ptr_;
      block_ptr_->shared_cnt_++;
    }
    return *this;
  }

  template <typename Y>
  SharedPtr& assign_from(SharedPtr<Y>&& shared_p) noexcept {
    if (static_cast<const void*>(this) != static_cast<const void*>(&shared_p)) {
      decrease_shared_cnt();
      block_ptr_ = nullptr;
      data_ptr_ = nullptr;
      swap(shared_p);
    }
    return *this;
  }

  template <typename Y>
  void copy_from(const SharedPtr<Y>& ptr) noexcept {
    block_ptr_ = ptr.block_ptr_;
    data_ptr_ = ptr.data_ptr_;
    if (block_ptr_ != nullptr) {
      block_ptr_->shared_cnt_++;
    }
  }

 public:
  void swap(SharedPtr<T>& other) {
    std::swap(block_ptr_, other.block_ptr_);
    std::swap(data_ptr_, other.data_ptr_);
  }

  void swap(SharedPtr<T>&& other) {
    std::swap(block_ptr_, other.block_ptr_);
    std::swap(data_ptr_, other.data_ptr_);
  }

  SharedPtr(std::nullptr_t) noexcept { reset<T>(nullptr); }

  SharedPtr() noexcept { reset<T>(nullptr); }

  SharedPtr(const SharedPtr<T>& ptr) { copy_from(ptr); }

  template <typename Y,
            std::enable_if_t<std::is_convertible<Y, T>::value, bool> = true>
  SharedPtr(const SharedPtr<Y>& ptr) noexcept {
    copy_from(ptr);
  }

  SharedPtr(SharedPtr<T>&& ptr) noexcept { swap(ptr); }

  template <typename Y = T,
            std::enable_if_t<std::is_convertible<Y, T>::value, bool> = true>
  SharedPtr(SharedPtr<Y>&& ptr) {
    swap(ptr);
  }

  template <typename Y = T, typename Deleter = std::default_delete<Y>,
            typename Alloc = std::allocator<Y>>
  explicit SharedPtr(Y* ptr, Deleter del = Deleter(), Alloc alloc = Alloc()) {
    reset<Y, Deleter, Alloc>(ptr, del, alloc);
  }

  SharedPtr<T>& operator=(const SharedPtr<T>& shared_p) noexcept {
    return assign_from(shared_p);
  }

  template <typename Y = T>
    requires((std::is_convertible<Y*, T*>::value))
  SharedPtr<T>& operator=(const SharedPtr<Y>& shared_p) noexcept {
    return assign_from(shared_p);
  }

  SharedPtr<T>& operator=(SharedPtr<T>&& shared_p) noexcept {
    return assign_from(std::move(shared_p));
  }

  template <typename Y = T,
            std::enable_if_t<std::is_convertible<Y, T>::value, bool> = true>
  SharedPtr<T>& operator=(SharedPtr<Y>&& shared_p) noexcept {
    return assign_from(std::move(shared_p));
  }

  void decrease_shared_cnt() noexcept {
    if (block_ptr_ != nullptr) {
      block_ptr_->shared_cnt_--;
      if (block_ptr_->shared_cnt_ == 0) {
        block_ptr_->destroy_obj();
      }
      if ((block_ptr_->weak_cnt_ == 0) && (block_ptr_->shared_cnt_ == 0)) {
        block_ptr_->destroy_class();
      }
    }
  }

  ~SharedPtr() {
    decrease_shared_cnt();
    block_ptr_ = nullptr;
    data_ptr_ = nullptr;
  }

  T& operator*() const noexcept { return *data_ptr_; }

  T* operator->() const noexcept { return data_ptr_; }

  std::size_t use_count() const noexcept {
    if (block_ptr_ == nullptr) {
      return 0;
    }
    return block_ptr_->shared_cnt_;
  }

  T* get() const { return data_ptr_; }

  template <typename Y = T, typename Deleter = std::default_delete<T>,
            typename Alloc = std::allocator<T>,
            std::enable_if_t<std::is_convertible<Y, T>::value, bool> = true>
  void reset(Y* ptr = nullptr, Deleter del = Deleter(), Alloc alloc = Alloc()) {
    decrease_shared_cnt();
    if (ptr != nullptr) {
      using block_alloc = typename std::allocator_traits<
          Alloc>::template rebind_alloc<ControlBlockRegular<Y, Deleter, Alloc>>;
      block_alloc block_allocator = alloc;
      auto* block =
          std::allocator_traits<block_alloc>::allocate(block_allocator, 1);

      try {
        std::allocator_traits<block_alloc>::construct(block_allocator, block,
                                                      ptr, del, alloc);
        block_ptr_ = block;
      } catch (...) {
        std::allocator_traits<block_alloc>::deallocate(block_allocator, block,
                                                       1);
      }

      block_ptr_->shared_cnt_++;
      data_ptr_ = static_cast<T*>(ptr);
    } else {
      data_ptr_ = nullptr;
      block_ptr_ = nullptr;
    }
  }
};

template <typename T>
class WeakPtr {
 private:
  template <typename OtherType>
  friend class SharedPtr;

  template <typename OtherType>
  friend class WeakPtr;

  ControlBlockBase* block_ptr_ = nullptr;
  T* data_ptr_ = nullptr;

 public:
  WeakPtr() noexcept = default;

  template <typename Y = T,
            std::enable_if_t<std::is_convertible<Y, T>::value, bool> = true>
  WeakPtr(const SharedPtr<Y>& ptr) {
    block_ptr_ = ptr.block_ptr_;
    data_ptr_ = ptr.data_ptr_;
    block_ptr_->weak_cnt_++;
  }

  template <typename Y = T,
            std::enable_if_t<std::is_convertible<Y, T>::value, bool> = true>
  WeakPtr(const WeakPtr<Y>& ptr) {
    block_ptr_ = ptr.block_ptr_;
    data_ptr_ = ptr.data_ptr_;
    block_ptr_->weak_cnt_++;
  }

  template <typename Y = T,
            std::enable_if_t<std::is_convertible<Y, T>::value, bool> = true>
  WeakPtr(WeakPtr<Y>&& ptr) {
    swap(ptr);
  }

  template <typename Y = T,
            std::enable_if_t<std::is_convertible<Y, T>::value, bool> = true>
  WeakPtr<T>& operator=(const WeakPtr<Y>& ptr) {
    decrease_weak_cnt();
    block_ptr_ = ptr.block_ptr_;
    data_ptr_ = ptr.data_ptr_;
    block_ptr_->weak_cnt_++;

    return *this;
  }

  template <typename Y = T,
            std::enable_if_t<std::is_convertible<Y, T>::value, bool> = true>
  WeakPtr<T>& operator=(WeakPtr<Y>&& ptr) {
    decrease_weak_cnt();
    swap(ptr);

    return *this;
  }

  void decrease_weak_cnt() {
    if (block_ptr_ != nullptr) {
      block_ptr_->weak_cnt_--;
      if ((block_ptr_->weak_cnt_ == 0) && (block_ptr_->shared_cnt_ == 0)) {
        block_ptr_->destroy_class();  // w!e!a!k!_!p!t!r должен как то понимать,
                                      // что обьект под его указателем - уже не
                                      // действителен соответственно удалить
                                      //  Shared_ptr полностью мы не можем. А
                                      //  удалить его когда то надо!
      }
    }
  }

  ~WeakPtr() { decrease_weak_cnt(); }

  bool expired() const {
    if (block_ptr_ != nullptr) {
      return block_ptr_->shared_cnt_ <= 0;
    }
    return true;
  }

  SharedPtr<T>* lock() { return SharedPtr<T>(data_ptr_); }

  void swap(WeakPtr<T>& ptr) {
    std::swap(block_ptr_, ptr.block_ptr_);
    std::swap(data_ptr_, ptr.data_ptr_);
  }
};

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> AllocateShared(const Alloc& alloc, Args&&... args) {
  using block_alloc = typename std::allocator_traits<
      Alloc>::template rebind_alloc<ControlBlockShared<T, Alloc>>;

  block_alloc block_allocator = alloc;
  ControlBlockShared<T, Alloc>* block_ptr;

  try {
    block_ptr =
        std::allocator_traits<block_alloc>::allocate(block_allocator, 1);
  } catch (...) {
    throw;
  }

  try {
    std::allocator_traits<block_alloc>::construct(
        block_allocator, block_ptr, alloc, std::forward<Args>(args)...);
  } catch (...) {
    std::allocator_traits<block_alloc>::deallocate(block_allocator, block_ptr,
                                                   1);

    throw;
  }

  SharedPtr<T> ptr;
  ptr.block_ptr_ = block_ptr;
  ptr.block_ptr_->shared_cnt_++;
  ptr.data_ptr_ = &(block_ptr->data);
  return ptr;
}

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
  return AllocateShared<T, std::allocator<T>, Args...>(
      std::allocator<T>(), std::forward<Args>(args)...);
}