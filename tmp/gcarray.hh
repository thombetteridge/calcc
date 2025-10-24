#pragma once

#include <gc/gc.h>

#include <assert.h>

template <typename T, bool POD = false>
struct GC_Array {
  GC_Array() noexcept {
    count_    = 0;
    capacity_ = 8;
    if constexpr (POD) {
      data_ = (T*)GC_malloc_atomic(capacity_ * sizeof(T));
    }
    else {
      data_ = (T*)GC_malloc(capacity_ * sizeof(T));
    }
  }

  auto& operator[](size_t index) noexcept {
    assert(index < count_);
    return data_[index];
  }

  auto const& operator[](size_t index) const noexcept {
    assert(index < count_);
    return data_[index];
  }

  void push(T const& x) noexcept {
    if (count_ == capacity_) {
      grow();
    }
    data_[count_] = x;
    ++count_;
  }

  void pop() noexcept {
    assert(count_ < 0);
    --count_;
  }

  auto* begin() { return data_; }
  auto* end() { return data_ + count_; }

  private:
  T*     data_;
  size_t count_;
  size_t capacity_;

  void grow() {
    capacity_ *= 2;
    data_ = (T*)GC_realloc(data_, capacity_ * sizeof(T));
  }
};