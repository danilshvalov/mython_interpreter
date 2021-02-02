#include "object.h"

#include <algorithm>
#include <sstream>
#include <string_view>

#include "statement.h"

using namespace std;

namespace Runtime {

void ClassInstance::Print(std::ostream& os) { Call("__str__", {})->Print(os); }

bool ClassInstance::HasMethod(const std::string& method,
                              size_t argument_count) const {
  const auto& method_ptr = class_.GetMethod(method);
  return method_ptr && method_ptr->formal_params.size() == argument_count;
}

const Closure& ClassInstance::Fields() const { return fields_; }

Closure& ClassInstance::Fields() { return fields_; }

ClassInstance::ClassInstance(const Class& cls) : class_(cls) {}

ObjectHolder ClassInstance::Call(const std::string& method,
                                 const std::vector<ObjectHolder>& actual_args) {
  if (!HasMethod(method, actual_args.size())) {
    if (method == "__str__") {
      stringstream ptr;
      ptr << this;
      return ObjectHolder::Own(String(ptr.str()));
    }
    throw runtime_error("Class: method \"" + method + "\" not found");
  }

  try {
    auto* method_ptr = class_.GetMethod(method);
    Closure closure = {{"self", ObjectHolder::Share(*this)}};
    for (size_t i = 0; i < actual_args.size(); ++i) {
      closure[method_ptr->formal_params[i]] = actual_args[i];
    }
    return method_ptr->body->Execute(closure);
  } catch (ObjectHolder& returned_value) {
    return returned_value;
  }
}

Class::Class(std::string name, std::vector<Method> methods, const Class* parent)
    : name_(std::move(name)), parent_(parent) {
  methods_.reserve(methods_.size() + methods.size() + 1);

  for (auto&& method : methods) {
    if (auto it = methods_.find(method.name); it != methods_.end()) {
      throw runtime_error("Class " + name_ +
                          " has duplicate methods with name " + method.name);
    }
    methods_.insert({method.name, move(method)});
  }
}

const Method* Class::GetMethod(const std::string& name) const {
  if (auto it = methods_.find(name); it != methods_.end()) {
    return &it->second;
  } else if (parent_) {
    return parent_->GetMethod(name);
  }
  return nullptr;
}

void Class::Print(ostream& os) { os << "Class " << name_; }

const std::string& Class::GetName() const { return name_; }

void Bool::Print(std::ostream& os) { os << (GetValue() ? "True" : "False"); }

} /* namespace Runtime */
