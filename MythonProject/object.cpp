#include "object.h"

#include <algorithm>
#include <sstream>
#include <string_view>

#include "statement.h"

using namespace std;

namespace Runtime {

void ClassInstance::Print(std::ostream& os) {
  auto res = Call("__str__", {});
  res->Print(os);
}

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

  auto& instance_method = *class_.GetMethod(method);
  Closure argument_closure = {{"self", ObjectHolder::Share(*this)}};

  transform(begin(instance_method.formal_params),
            end(instance_method.formal_params), begin(actual_args),
            inserter(argument_closure, end(argument_closure)),
            [](string formal, ObjectHolder actual) {
              return make_pair(formal, actual);
            });
  try {
    return instance_method.body->Execute(argument_closure);
  } catch (ObjectHolder& returned_value) {
    return returned_value;
  }
}

Class::Class(std::string name, std::vector<Method> methods, const Class* parent)
    : name_(std::move(name)), parent_(parent) {
  InitParentMethods(parent);

  methods_impl_.reserve(methods_impl_.size() + methods.size() + 1);
  methods_.reserve(methods_.size() + methods.size() + 1);

  for (auto&& method : methods) {
    methods_impl_.push_back(move(method));
    methods_[methods_impl_.back().name] = &methods_impl_.back();
  }

  if (static const string init_method = "__init__"; !GetMethod(init_method)) {
    methods_impl_.push_back(Method{init_method, {}, make_unique<Ast::None>()});
    methods_[init_method] = &methods_impl_.back();
  }
}

const Method* Class::GetMethod(const std::string& name) const {
  if (auto it = methods_.find(name); it != methods_.end()) {
    return it->second;
  }
  return nullptr;
}

void Class::Print(ostream& os) {
  Closure empty;
  GetMethod("__str__")->body->Execute(empty).TryAs<String>()->Print(os);
}

const std::string& Class::GetName() const { return name_; }

void Bool::Print(std::ostream& os) { os << (GetValue() ? "True" : "False"); }

} /* namespace Runtime */
