#pragma once
#include <boost/preprocessor/cat.hpp>
#include <utility>

#define SCOPE_EXIT                                                \
  auto SCOPE_GUARD_PP_UNIQUE(_scope_exit_local) =                 \
    ::scope_guard_detail::scope_guard_helper() << [&]

#define SCOPE_GUARD_PP_UNIQUE(x)                \
  BOOST_PP_CAT(x, __LINE__)

template <class F> class scope_guard {
  F f;
 public:
  inline scope_guard(scope_guard<F> &&o): f(std::move(o.f)) {}
  inline scope_guard(F f): f(std::move(f)) {}
  inline ~scope_guard() { f(); }
};

namespace scope_guard_detail {

struct scope_guard_helper {
  template <class F> inline scope_guard<F> operator<<(F &&f) {
    return scope_guard<F>(std::forward<F>(f));
  }
};

} // namespace scope_guard_detail
