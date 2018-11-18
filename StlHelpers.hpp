#ifndef STEINER_STL_HELPERS_H_DEFINED__
#define STEINER_STL_HELPERS_H_DEFINED__

#include <utility>

// Special remove if.
template<typename It, typename Predicate>
It remove_if_with_index(It first, It last, Predicate Pred) {
  size_t Idx = 0;
  for (; first != last; ++first, ++Idx) {
    if (Pred(*first, Idx))
      break;
  }
  if (first == last)
    return first;

  It result = first;
  ++first;
  ++Idx;
  for (; first != last; ++first, ++Idx) {
    if (!Pred(*first, Idx)) {
      *result = std::move(*first);
      ++result;
    }
  }
  return result;
}

#endif
