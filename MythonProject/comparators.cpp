#include "comparators.h"

#include <functional>
#include <optional>
#include <iostream>

#include "object.h"
#include "object_holder.h"

using namespace std;

namespace Runtime {

bool Equal(ObjectHolder lhs, ObjectHolder rhs) {
  //if (const auto& [left, right] =
  //        make_tuple(lhs.TryAs<ClassInstance>(), rhs.TryAs<ClassInstance>());
  //    left && right) {
  //  return left == right;
  //}

  if (const auto& [left, right] =
          make_tuple(lhs.TryAs<String>(), rhs.TryAs<String>());
      left && right) {
    return left->GetValue() == right->GetValue();
  }

  if (const auto& [left, right] =
          make_tuple(lhs.TryAs<Number>(), rhs.TryAs<Number>());
      left && right) {
    auto [left_value, right_value] =
        std::make_tuple(left->GetValue(), right->GetValue());
  	return left_value == right_value;
    /*return static_cast<int>(left->GetValue()) ==
           static_cast<int>(right->GetValue());*/
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
    auto [left_value, right_value] =
        std::make_tuple(left->GetValue(), right->GetValue());
    return left_value < right_value;
  }

	return false;
}

} /* namespace Runtime */
