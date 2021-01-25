#include "statement.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <sstream>

#include "object.h"

using namespace std;

namespace Ast {

using Runtime::Closure;

ObjectHolder Assignment::Execute(Closure& closure) {
  return closure[var_name] = right_value->Execute(closure);
}

Assignment::Assignment(string var, unique_ptr<Statement> rv)
    : var_name(move(var)), right_value(move(rv)) {}

VariableValue::VariableValue(string var_name) {
  dotted_ids.push_back(move(var_name));
}

VariableValue::VariableValue(vector<string> dotted_ids)
    : dotted_ids(move(dotted_ids)) {}

ObjectHolder VariableValue::Execute(Closure& closure) {
  const string& var_name = dotted_ids.front();
  if (auto it = closure.find(var_name); it == closure.end()) {
    throw runtime_error("Variable value: Unexpected variable");
  }

  return accumulate(
      next(begin(dotted_ids)), end(dotted_ids), closure.at(var_name),
      [](ObjectHolder parent, const string& name) {
        return parent.TryAs<Runtime::ClassInstance>()->Fields().at(name);
      });
}

unique_ptr<Print> Print::Variable(string var) {
  return make_unique<Print>(make_unique<VariableValue>(var));
}

Print::Print(unique_ptr<Statement> argument) { args.push_back(move(argument)); }

Print::Print(vector<unique_ptr<Statement>> args) : args(move(args)) {}

ObjectHolder Print::Execute(Closure& closure) {
  bool is_first = true;
  for (const auto& argument : args) {
    if (!is_first) {
      *output << " ";
    }
    is_first = false;
    if (ObjectHolder object = argument->Execute(closure)) {
      object->Print(*output);
    } else {
      *output << "None";
    }
  }
  *output << '\n';
  return {};
}

ostream* Print::output = &cout;

void Print::SetOutputStream(ostream& output_stream) { output = &output_stream; }

MethodCall::MethodCall(unique_ptr<Statement> object, string method,
                       vector<unique_ptr<Statement>> args)
    : object(move(object)), method(move(method)), args(move(args)) {}

ObjectHolder MethodCall::Execute(Closure& closure) {
  auto executed_obj = object->Execute(closure);
  auto inst = executed_obj.TryAs<Runtime::ClassInstance>();

  if (!inst) {
    throw runtime_error("MethodCall works only with ClassInstance");
  }

  vector<ObjectHolder> method_args;
  method.reserve(args.size());
  transform(args.begin(), args.end(), back_inserter(method_args),
            [&closure](auto& arg) { return arg->Execute(closure); });

  return inst->Call(method, method_args);
}

ObjectHolder Stringify::ToString(ObjectHolder object) {
  using Runtime::Number, Runtime::String, Runtime::Bool;

  if (const auto& inst = object.TryAs<String>(); inst) {
    return object;
  }

  if (const auto& inst = object.TryAs<Number>(); inst) {
    return Runtime::ObjectHolder::Own(String(to_string(inst->GetValue())));
  }

  if (const auto& inst = object.TryAs<Bool>(); inst) {
    return Runtime::ObjectHolder::Own(
        String((inst->GetValue() ? "True" : "False")));
  }

  throw runtime_error("Stringify: Unexpected value");
}

ObjectHolder Stringify::Execute(Closure& closure) {
  using Runtime::ClassInstance;
  auto object = argument->Execute(closure);
  const auto& inst = object.TryAs<ClassInstance>();

  if (inst) {
    return ToString(inst->Call("__str__", {}));
  }

  return ToString(object);
}

ObjectHolder Add::Execute(Closure& closure) {
  using Runtime::Number, Runtime::String, Runtime::ClassInstance;

  auto [left_obj, right_obj] =
      make_tuple(lhs->Execute(closure), rhs->Execute(closure));

  if (const auto& [left_inst, right_inst] =
          make_tuple(left_obj.TryAs<Number>(), right_obj.TryAs<Number>());
      left_inst && right_inst) {
    return ObjectHolder::Own(
        Number(left_inst->GetValue() + right_inst->GetValue()));
  }

  if (const auto& [left_inst, right_inst] =
          make_tuple(left_obj.TryAs<String>(), right_obj.TryAs<String>());
      left_inst && right_inst) {
    return ObjectHolder::Own(
        String(left_inst->GetValue() + right_inst->GetValue()));
  }

  // left_inst не const, потому что Call не константный метод
  if (auto left_inst = left_obj.TryAs<ClassInstance>(); left_inst) {
    return left_inst->Call("__add__", {move(right_obj)});
  }

  // right_inst не const, потому что Call не константный метод
  if (auto right_inst = right_obj.TryAs<ClassInstance>(); right_inst) {
    return right_inst->Call("__add__", {move(left_obj)});
  }

  throw runtime_error("Add: Wrong values");
}

ObjectHolder Sub::Execute(Closure& closure) {
  return ObjectHolder::Own(Runtime::Number(
      lhs->Execute(closure).TryAs<Runtime::Number>()->GetValue() -
      rhs->Execute(closure).TryAs<Runtime::Number>()->GetValue()));
}

ObjectHolder Mult::Execute(Runtime::Closure& closure) {
  return ObjectHolder::Own(Runtime::Number(
      lhs->Execute(closure).TryAs<Runtime::Number>()->GetValue() *
      rhs->Execute(closure).TryAs<Runtime::Number>()->GetValue()));
}

ObjectHolder Div::Execute(Runtime::Closure& closure) {
  return ObjectHolder::Own(Runtime::Number(
      lhs->Execute(closure).TryAs<Runtime::Number>()->GetValue() /
      rhs->Execute(closure).TryAs<Runtime::Number>()->GetValue()));
}

ObjectHolder Compound::Execute(Closure& closure) {
  for (auto& statement : statements) {
    statement->Execute(closure);
  }

  return ObjectHolder::None();
}

ObjectHolder Return::Execute(Closure& closure) {
  throw statement->Execute(closure);
}

ClassDefinition::ClassDefinition(ObjectHolder class_)
    : cls(move(class_)), class_name(cls.TryAs<Runtime::Class>()->GetName()) {}

ObjectHolder ClassDefinition::Execute(Runtime::Closure& closure) { return cls; }

FieldAssignment::FieldAssignment(VariableValue object, string field_name,
                                 unique_ptr<Statement> rv)
    : object(move(object)),
      field_name(move(field_name)),
      right_value(move(rv)) {}

ObjectHolder FieldAssignment::Execute(Runtime::Closure& closure) {
  ObjectHolder cur_obj_holder = closure[object.dotted_ids.front()];
  for (size_t i = 1; i < object.dotted_ids.size(); ++i) {
    cur_obj_holder = cur_obj_holder.TryAs<Runtime::ClassInstance>()
                         ->Fields()[object.dotted_ids[i]];
  }
  return cur_obj_holder.TryAs<Runtime::ClassInstance>()->Fields()[field_name] =
             right_value->Execute(closure);
}

IfElse::IfElse(unique_ptr<Statement> condition, unique_ptr<Statement> if_body,
               unique_ptr<Statement> else_body)
    : condition(move(condition)),
      if_body(move(if_body)),
      else_body(move(else_body)) {}

ObjectHolder IfElse::Execute(Runtime::Closure& closure) {
  if (const auto& as_bool = condition->Execute(closure).TryAs<Runtime::Bool>();
      as_bool) {
    return (as_bool->GetValue() ? if_body->Execute(closure)
                               : else_body->Execute(closure));
  }
  return ObjectHolder::None();
}

ObjectHolder Or::Execute(Runtime::Closure& closure) {
  return ObjectHolder::Own(
      Runtime::Bool(lhs->Execute(closure).TryAs<Runtime::Bool>()->GetValue() ||
                    rhs->Execute(closure).TryAs<Runtime::Bool>()->GetValue()));
}

ObjectHolder And::Execute(Runtime::Closure& closure) {
  return ObjectHolder::Own(
      Runtime::Bool(lhs->Execute(closure).TryAs<Runtime::Bool>()->GetValue() &&
                    rhs->Execute(closure).TryAs<Runtime::Bool>()->GetValue()));
}

ObjectHolder Not::Execute(Runtime::Closure& closure) {
  return ObjectHolder::Own(Runtime::Bool(
      argument->Execute(closure).TryAs<Runtime::Bool>()->GetValue()));
}

Comparison::Comparison(Comparator cmp, unique_ptr<Statement> lhs,
                       unique_ptr<Statement> rhs) : comparator(std::move(cmp)), left(std::move(lhs)), right(std::move(rhs)) {}

ObjectHolder Comparison::Execute(Runtime::Closure& closure) {
  return ObjectHolder::Own(Runtime::Bool(comparator(left->Execute(closure), right->Execute(closure))));
}

NewInstance::NewInstance(const Runtime::Class& class_,
                         vector<unique_ptr<Statement>> args)
    : class_(class_), args(move(args)) {}

NewInstance::NewInstance(const Runtime::Class& class_)
    : NewInstance(class_, {}) {}

ObjectHolder NewInstance::Execute(Runtime::Closure& closure) {
  Runtime::ClassInstance result(class_);
  if (auto* m = class_.GetMethod("__init__"); m) {
    vector<ObjectHolder> actual_args;
    for (auto& stmt : args) {
      actual_args.push_back(stmt->Execute(closure));
    }

    result.Call("__init__", std::move(actual_args));
  }
  return ObjectHolder::Own(std::move(result));
}

} /* namespace Ast */
