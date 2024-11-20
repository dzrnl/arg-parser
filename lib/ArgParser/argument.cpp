#include "argument.h"

using namespace ArgumentParser;

ArgumentBase::ArgumentBase(std::string long_name, std::string description)
        : long_name_(std::move(long_name)), description_(std::move(description)) {
    if (long_name_.empty()) {
        PrintError(EmptyArgumentLongName);
    }
}

ArgumentBase::ArgumentBase(char short_name, std::string long_name, std::string description)
        : short_name_(short_name), long_name_(std::move(long_name)), description_(std::move(description)) {
    if (short_name_.value() == ' ') {
        PrintError(EmptyArgumentShortName);
    }
    if (long_name_.empty()) {
        PrintError(EmptyArgumentLongName);
    }
}

std::string ArgumentBase::GetLongName() const {
    return long_name_;
}

std::string ArgumentBase::GetTypeName() const {
    const auto& type = GetType();
    if (type == typeid(bool)) {
        return "bool";
    }
    if (type == typeid(int)) {
        return "int";
    }
    if (type == typeid(std::string)) {
        return "string";
    }
    return type.name();
}

bool ArgumentBase::IsPositional() const {
    return is_positional_;
}

void ArgumentBase::PrintError(const ArgumentError& error) const {
    std::cerr << "error: ";
    switch (error) {
        case EmptyArgumentLongName:
            std::cerr << "argument long name cannot be empty string" << std::endl;
            break;
        case EmptyArgumentShortName:
            std::cerr << "argument short name cannot be whitespace char" << std::endl;
            break;
        case NoArgumentValue:
            std::cerr << "no value was passed for the argument --" << long_name_ << std::endl;
            break;
        case InvalidArgumentType:
            std::cerr << "argument --" << GetLongName() << " has value type <" << GetTypeName() << ">" << std::endl;
            break;
        default:
            std::cerr << "unknown error" << std::endl;
    }
    exit(EXIT_FAILURE);
}
