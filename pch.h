#pragma once
#include <unknwn.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>

template<typename T>
struct Property {
  T m_value{};
  T operator()() const noexcept { return m_value; }
  void operator()(const T& v) noexcept { m_value = v; }
  Property<T>& operator=(const T& v) noexcept { m_value = v; return *this; }
  Property() = default;
  Property(const T& v) noexcept : m_value(v) {}
};
