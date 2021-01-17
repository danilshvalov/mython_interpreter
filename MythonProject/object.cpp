#include "object.h"
#include "statement.h"

#include <sstream>
#include <string_view>

using namespace std;

namespace Runtime {

	void ClassInstance::Print(std::ostream& os) {
	}

	bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
		const auto& method_ptr = class_.GetMethod(method);
		return method_ptr && method_ptr->formal_params.size() == argument_count;
	}

	const Closure& ClassInstance::Fields() const {
		return fields_;
	}

	Closure& ClassInstance::Fields() {
		return fields_;
	}

	ClassInstance::ClassInstance(const Class& cls) : class_(cls) {}

	ObjectHolder ClassInstance::Call(const std::string& method, const std::vector<ObjectHolder>& actual_args) {
		const auto& class_method = class_.GetMethod(method);
		for (size_t i = 0; i < actual_args.size(); ++i) {
			fields_[class_method->formal_params[i]] = actual_args[i];
		}
		return class_method->body->Execute(fields_);
	}

	Class::Class(std::string name, std::vector<Method> methods, const Class* parent) {
		methods_.reserve(methods.size());
		for (auto& method : methods) {
			methods_[method.name] = std::move(method);
		}
	}

	const Method* Class::GetMethod(const std::string& name) const {
		if (auto it = methods_.find(name); it != methods_.end()) {
			return &it->second;
		}
		return nullptr;
	}

	void Class::Print(ostream& os) {
	}

	const std::string& Class::GetName() const {
		return name_;
	}

	void Bool::Print(std::ostream& os) {
	}

} /* namespace Runtime */
