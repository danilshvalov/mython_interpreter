#include "comparators.h"

#include <functional>
#include <iostream>
#include <optional>

#include "object.h"
#include "object_holder.h"

using namespace std;

namespace Runtime {

bool Equal(ObjectHolder lhs, ObjectHolder rhs) {
  if (const auto& [left, right] =
          make_tuple(lhs.TryAs<String>(), rhs.TryAs<String>());
      left && right) {
    return left->GetValue() == right->GetValue();
  }

  if (const auto& [left, right] =
          make_tuple(lhs.TryAs<Number>(), rhs.TryAs<Number>());
      left && right) {
    return left->GetValue() == right->GetValue();
  }

  return false;
}

bool Less(ObjectHolder lhs, ObjectHolder rhs) {
  if (const auto& [left, right] =
          make_tuple(lhs.TryAs<String>(), rhs.TryAs<String>());
      left && right) {
    return left->GetValue() < right->GetValue();
  }

  if (const auto& [left, right] =
          make_tuple(lhs.TryAs<Number>(), rhs.TryAs<Number>());
      left && right) {
    return left->GetValue() < right->GetValue();
  }

  return false;
}

} /* namespace Runtime */
