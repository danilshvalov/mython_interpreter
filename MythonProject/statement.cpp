#include "statement.h"
#include "object.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace Ast {

	using Runtime::Closure;

	ObjectHolder Assignment::Execute(Closure& closure) {
		auto object = right_value.release()->Execute(closure);
		closure[var_name] = object;
		return object;
	}

	Assignment::Assignment(string var, unique_ptr<Statement> rv) : var_name(move(var)), right_value(move(rv)) {
	}

	VariableValue::VariableValue(string var_name) {
		dotted_ids.push_back(move(var_name));
	}

	VariableValue::VariableValue(vector<string> dotted_ids) : dotted_ids(move(dotted_ids)) {}

	ObjectHolder VariableValue::Execute(Closure& closure) {
		for (const auto& name : dotted_ids) {
			if (auto it = closure.find(name); it != closure.end()) {
				return it->second;
			}
		}
		throw runtime_error("Variable value: Unexpected variable");
	}

	unique_ptr<Print> Print::Variable(string var) {
		return make_unique<Print>(make_unique<VariableValue>(var));
	}

	Print::Print(unique_ptr<Statement> argument) {
		args.push_back(move(argument));
	}

	Print::Print(vector<unique_ptr<Statement>> args) : args(move(args)) {}

	ObjectHolder Print::Execute(Closure& closure) {
		bool is_first = true;
		for (const auto& argument : args) {
			if (!is_first) {
				*output << " ";
			}
			is_first = false;
			if (auto object = argument->Execute(closure)) {
				object->Print(*output);
			} else {
				*output << "None";
			}
		}
		*output << '\n';
		return {};
	}

	ostream* Print::output = &cout;

	void Print::SetOutputStream(ostream& output_stream) {
		output = &output_stream;
	}

	MethodCall::MethodCall(
		unique_ptr<Statement> object
		, string method
		, vector<unique_ptr<Statement>> args
	) {
	}

	ObjectHolder MethodCall::Execute(Closure& closure) {
		return {};
	}

	ObjectHolder Stringify::Execute(Closure& closure) {
		auto object = argument.release()->Execute(closure);
		if (const auto& instance = object.TryAs<Runtime::Number>()) {
			return ObjectHolder::Own(Runtime::String(to_string(instance->GetValue())));
		}
		if (const auto& instance = object.TryAs<Runtime::ClassInstance>()) {
			return instance->Call("__str__", {});
		}
		if (const auto& instance = object.TryAs<Runtime::String>()) {
			return object;
		}
		throw runtime_error("Stringify: Unexpected value");
	}

	ObjectHolder Add::Execute(Closure& closure) {
		using Runtime::Number, Runtime::String, Runtime::ClassInstance;

		auto [left_obj, right_obj] = make_tuple(lhs->Execute(closure), rhs->Execute(closure));

		if (const auto& [left_inst, right_inst] = make_tuple(left_obj.TryAs<Number>(), right_obj.TryAs<Number>()); left_inst && right_inst) {
			return ObjectHolder::Own(Number(left_inst->GetValue() + right_inst->GetValue()));
		}

		if (const auto& [left_inst, right_inst] = make_tuple(left_obj.TryAs<String>(), right_obj.TryAs<String>()); left_inst && right_inst) {
			return ObjectHolder::Own(String(left_inst->GetValue() + right_inst->GetValue()));
		}

		// left_inst не const, потому что Call не константный метод
		if (auto left_inst = left_obj.TryAs<ClassInstance>(); left_inst) {
			return left_inst->Call("__add__", {right_obj});
		}

		// right_inst не const, потому что Call не константный метод
		if (auto right_inst = right_obj.TryAs<ClassInstance>(); right_inst) {
			return right_inst->Call("__add__", { left_obj });
		}

		throw runtime_error("Add: Wrong values");
	}

	ObjectHolder Sub::Execute(Closure& closure) {
		return ObjectHolder::Own(Runtime::Number(lhs->Execute(closure).TryAs<Runtime::Number>()->GetValue() - rhs->Execute(closure).TryAs<Runtime::Number>()->GetValue()));
	}

	ObjectHolder Mult::Execute(Runtime::Closure& closure) {
		return ObjectHolder::Own(Runtime::Number(lhs->Execute(closure).TryAs<Runtime::Number>()->GetValue() * rhs->Execute(closure).TryAs<Runtime::Number>()->GetValue()));
	}

	ObjectHolder Div::Execute(Runtime::Closure& closure) {
		return ObjectHolder::Own(Runtime::Number(lhs->Execute(closure).TryAs<Runtime::Number>()->GetValue() / rhs->Execute(closure).TryAs<Runtime::Number>()->GetValue()));
	}

	ObjectHolder Compound::Execute(Closure& closure) {
		return {};
	}

	ObjectHolder Return::Execute(Closure& closure) {
		return {};
	}

	ClassDefinition::ClassDefinition(ObjectHolder class_) : cls(move(class_)), class_name(cls.TryAs<Runtime::Class>()->GetName()) {}

	ObjectHolder ClassDefinition::Execute(Runtime::Closure& closure) {
		return {};
	}

	FieldAssignment::FieldAssignment(
		VariableValue object, string field_name, unique_ptr<Statement> rv
	)
		: object(move(object))
		, field_name(move(field_name))
		, right_value(move(rv)) {
	}

	ObjectHolder FieldAssignment::Execute(Runtime::Closure& closure) {
		auto value = right_value->Execute(closure);
		for (const auto& idx : object.dotted_ids) {
			if (auto it = closure.find(idx); it != closure.end()) {
				it->second.TryAs<Runtime::ClassInstance>()->Fields().insert({ field_name, value });
			}
		}
		closure[field_name] = value;
		return value;
	}

	IfElse::IfElse(
		unique_ptr<Statement> condition,
		unique_ptr<Statement> if_body,
		unique_ptr<Statement> else_body
	) {
	}

	ObjectHolder IfElse::Execute(Runtime::Closure& closure) {
		return {};
	}

	ObjectHolder Or::Execute(Runtime::Closure& closure) {
		return {};
	}

	ObjectHolder And::Execute(Runtime::Closure& closure) {
		return {};
	}

	ObjectHolder Not::Execute(Runtime::Closure& closure) {
		return {};
	}

	Comparison::Comparison(
		Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs
	) {
	}

	ObjectHolder Comparison::Execute(Runtime::Closure& closure) {
		return {};
	}

	NewInstance::NewInstance(
		const Runtime::Class& class_, vector<unique_ptr<Statement>> args
	)
		: class_(class_)
		, args(move(args)) {
	}

	NewInstance::NewInstance(const Runtime::Class& class_) : NewInstance(class_, {}) {
	}

	ObjectHolder NewInstance::Execute(Runtime::Closure& closure) {
		return ObjectHolder::Own(Runtime::ClassInstance(class_));
	}


} /* namespace Ast */
