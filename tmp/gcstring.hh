#pragma once

#include <gc/gc.h>

#include <assert.h>
#include <string.h>

struct GC_String {
  GC_String() noexcept {
  }

  GC_String(const char* str) noexcept {
    auto n = strlen(str);
    init(n);
    memcpy(data_, str, n);
    data_[length_] = '\0';
  }

  GC_String(const char* a, const char* b) noexcept {
    auto a_len = strlen(a);
    auto b_len = strlen(b);
    auto n     = a_len + b_len;
    init(n);
    memcpy(data_, a, a_len);
    memcpy(data_ + a_len, b, b_len);
    data_[length_] = '\0';
  }

  GC_String(GC_String const& a, GC_String const& b) noexcept {
    auto a_len = a.length_;
    auto b_len = b.length_;
    auto n     = a_len + b_len;
    init(n);
    memcpy(data_, a.data_, a_len);
    memcpy(data_ + a_len, b.data_, b_len);
    data_[length_] = '\0';
  }

  auto& operator[](size_t index) noexcept {
    assert(index < length_);
    return data_[index];
  }

  auto const& operator[](size_t index) const noexcept {
    assert(index < length_);
    return data_[index];
  }

  auto operator+(GC_String const& other) {
    return GC_String(*this, other);
  }

  auto* begin() { return data_; }
  auto* end() { return data_ + length_; }

  auto ptr() { return data_; }

  private:
  size_t length_;
  char*  data_;

  void init(size_t n) {
    length_ = n;
    data_   = (char*)GC_malloc_atomic((length_ * sizeof(char)) + 1);
  }
};