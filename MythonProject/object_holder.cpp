#include "object_holder.h"

#include "object.h"

namespace Runtime {

ObjectHolder ObjectHolder::Share(Object& object) {
  return ObjectHolder(
      std::shared_ptr<Object>(&object, [](auto*) { /* do nothing */ }));
}

ObjectHolder ObjectHolder::None() { return ObjectHolder(); }

Object& ObjectHolder::operator*() { return *Get(); }

const Object& ObjectHolder::operator*() const { return *Get(); }

Object* ObjectHolder::operator->() { return Get(); }

const Object* ObjectHolder::operator->() const { return Get(); }

Object* ObjectHolder::Get() { return data.get(); }

const Object* ObjectHolder::Get() const { return data.get(); }

ObjectHolder::operator bool() const { return Get(); }

bool IsTrue(ObjectHolder object) {
  if (!object) {
    return false;
  }

	if (const auto& numeric = object.TryAs<Number>(); numeric) {
		return numeric->GetValue() != 0;
  }

	if (const auto& str = object.TryAs<String>(); str) {
		return !str->GetValue().empty();
	}

	if (const auto& boolean = object.TryAs<Bool>(); boolean) {
		return boolean->GetValue();
	}

  return false;
}

}  // namespace Runtime
