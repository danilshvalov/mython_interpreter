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

	Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv) : var_name(std::move(var)), right_value(std::move(rv)) {
	}

	VariableValue::VariableValue(std::string var_name) {
		dotted_ids.push_back(std::move(var_name));
	}

	VariableValue::VariableValue(std::vector<std::string> dotted_ids) : dotted_ids(std::move(dotted_ids)) {}

	ObjectHolder VariableValue::Execute(Closure& closure) {
		for (const auto& name : dotted_ids) {
			if (auto it = closure.find(name); it != closure.end()) {
				return it->second;
			}
		}
		throw std::runtime_error("Variable value: Unexpected variable");
	}

	unique_ptr<Print> Print::Variable(std::string var) {
		return std::make_unique<Print>(std::make_unique<VariableValue>(var));
	}

	Print::Print(unique_ptr<Statement> argument) {
		args.push_back(std::move(argument));
	}

	Print::Print(vector<unique_ptr<Statement>> args) : args(std::move(args)) {}

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
		std::unique_ptr<Statement> object
		, std::string method
		, std::vector<std::unique_ptr<Statement>> args
	) {
	}

	ObjectHolder MethodCall::Execute(Closure& closure) {
		return {};
	}

	ObjectHolder Stringify::Execute(Closure& closure) {
		auto object = argument.release()->Execute(closure);
		if (const auto& instance = object.TryAs<Runtime::Number>()) {
			return ObjectHolder::Own(Runtime::String(std::to_string(instance->GetValue())));
		}
		if (const auto& instance = object.TryAs<Runtime::ClassInstance>()) {
			return instance->Call("__str__", {});
		}
		if (const auto& instance = object.TryAs<Runtime::String>()) {
			return object;
		}
		throw std::runtime_error("Stringify: Unexpected value");
	}

	ObjectHolder Add::Execute(Closure& closure) {
		return ObjectHolder::Own(Runtime::Number(lhs->Execute(closure).TryAs<Runtime::Number>()->GetValue() + rhs->Execute(closure).TryAs<Runtime::Number>()->GetValue()));
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

	ClassDefinition::ClassDefinition(ObjectHolder class_) : cls(std::move(class_)), class_name(cls.TryAs<Runtime::Class>()->GetName()) {}

	ObjectHolder ClassDefinition::Execute(Runtime::Closure& closure) {
		return {};
	}

	FieldAssignment::FieldAssignment(
		VariableValue object, std::string field_name, std::unique_ptr<Statement> rv
	)
		: object(std::move(object))
		, field_name(std::move(field_name))
		, right_value(std::move(rv)) {
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
		std::unique_ptr<Statement> condition,
		std::unique_ptr<Statement> if_body,
		std::unique_ptr<Statement> else_body
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
		const Runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args
	)
		: class_(class_)
		, args(std::move(args)) {
	}

	NewInstance::NewInstance(const Runtime::Class& class_) : NewInstance(class_, {}) {
	}

	ObjectHolder NewInstance::Execute(Runtime::Closure& closure) {
		return ObjectHolder::Own(Runtime::ClassInstance(class_));
	}


} /* namespace Ast */
