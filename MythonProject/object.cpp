#include "object.h"
#include "statement.h"

#include <sstream>
#include <string_view>

using namespace std;

namespace Runtime {

	void ClassInstance::Print(std::ostream& os) {
	}

	bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
		//auto it = class_.GetMethod()
		return {};
	}

	const Closure& ClassInstance::Fields() const {
		return fields_;
	}

	Closure& ClassInstance::Fields() {
		return fields_;
	}

	ClassInstance::ClassInstance(const Class& cls) : class_(cls) {}

	ObjectHolder ClassInstance::Call(const std::string& method, const std::vector<ObjectHolder>& actual_args) {
		auto class_method = class_.GetMethod(method);
		if (class_method->name == "__str__") {
			return {};
		} else {
			for (size_t i = 0; i < actual_args.size(); ++i) {
				fields_[class_method->formal_params[i]] = actual_args[i];
			}
			return class_method->body->Execute(fields_);
		}
		//Ast::MethodCall(std::move(class_method), method, )
		//return Ast::MethodCall(class_.GetMethod(method)->body->Execute(actual_args);
		return {};
	}

	Class::Class(std::string name, std::vector<Method> methods, const Class* parent) {
		methods_.reserve(methods.size());
		for (auto& method : methods) {
			methods_[method.name] = std::move(method);
		}
	}

	const Method* Class::GetMethod(const std::string& name) const {
		return &methods_.at(name);
	}

	void Class::Print(ostream& os) {
	}

	const std::string& Class::GetName() const {
		return name_;
	}

	void Bool::Print(std::ostream& os) {
	}

} /* namespace Runtime */
