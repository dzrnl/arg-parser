#include "arg_parser.h"

#include <iostream>
#include <sstream>
#include <algorithm>

using namespace ArgumentParser;

ArgParser::ArgParser(std::string name) : name_(std::move(name)) {}

Argument<bool>& ArgParser::AddHelp(const std::string& description) {
    if (help_argument_ != nullptr) {
        PrintError(HelpArgumentAlreadyExists);
    }

    help_argument_ = &AddArgument<bool>('h', "help", "Display this help and exit").Default(false);
    description_ = description;

    return *help_argument_;
}

std::string ArgParser::HelpDescription() const {
    std::stringstream ss;
    ss << name_ << '\n';
    if (!description_.empty()) {
        ss << description_ << '\n';
    }
    ss << '\n';

    if (!argument_map_.empty()) {
        for (const auto& [name, arg] : argument_map_) {
            ss << *arg << '\n';
        }
        ss << '\n';
    }

    if (help_argument_ != nullptr) {
        ss << *help_argument_ << '\n';
    }

    return ss.str();
}

bool ArgParser::Help() const {
    if (help_argument_ == nullptr) {
        return false;
    }

    return help_argument_->GetValue();
}

Argument<bool>& ArgParser::AddFlag(const std::string& long_name, const std::string& description) {
    return AddArgument<bool>(long_name, description).Default(false);
}

Argument<bool>& ArgParser::AddFlag(char short_name, const std::string& long_name, const std::string& description) {
    return AddArgument<bool>(short_name, long_name, description).Default(false);
}

bool ArgParser::GetFlagValue(const std::string& long_name) {
    return GetArgumentValue<bool>(long_name);
}

bool ArgParser::GetFlagValue(char short_name) {
    return GetArgumentValue<bool>(short_name);
}

bool ArgParser::Parse(const std::vector<std::string>& vec) {
    for (auto str = vec.begin() + 1; str != vec.end(); ++str) {
        if (str->starts_with("--")) {
            auto border = str->find('=');
            std::string long_name = str->substr(2, border - 2);
            std::string value = str->substr(border + 1);

            auto iter = argument_map_.find(long_name);

            if (iter == argument_map_.end()) {
                PrintError(UnknownArgument, long_name);
            }

            ArgumentBase* argument = iter->second.get();

            if (border == std::string::npos) {
                if (argument->GetType() == typeid(bool)) {
                    argument->SetValueFromString("1");
                    continue;
                }
                argument->PrintError(NoArgumentValue);
            }

            if (argument->GetType() != typeid(bool) && value.empty()) {
                argument->PrintError(NoArgumentValue);
            }

            argument->SetValueFromString(value);

        } else if (str->starts_with("-")) {
            for (int i = 1; i < str->length(); ++i) {
                char c = (*str)[i];

                auto iter_name = long_name_map_.find(c);

                if (iter_name == long_name_map_.end()) {
                    PrintError(UnknownArgument, c);
                }

                auto iter_arg = argument_map_.find(iter_name->second);

                ArgumentBase* argument = iter_arg->second.get();

                if (argument->GetType() == typeid(bool)) {
                    argument->SetValueFromString("1");
                } else {
                    if (i != str->length() - 1) {
                        PrintError(UnknownArgument, *str);
                    }
                    if (++str == vec.end()) {
                        argument->PrintError(NoArgumentValue);
                    }
                    argument->SetValueFromString(*str);
                    break;
                }
            }

        } else {
            if (!positional_argument_) {
                for (const auto& [long_name, arg] : argument_map_) {
                    if (arg->IsPositional()) {
                        positional_argument_ = arg.get();
                        break;
                    }
                }
                if (!positional_argument_) {
                    PrintError(NoPositionalArgument, *str);
                }
            }

            positional_argument_->SetValueFromString(*str);
        }
    }

    return std::all_of(argument_map_.cbegin(), argument_map_.cend(),
                       [](const auto& pair) {
                           return pair.second->HasValue();
                       });
}

bool ArgParser::Parse(int argc, char** argv) {
    std::vector<std::string> vec(argc);
    for (int i = 0; i < argc; ++i) {
        vec[i] = *(argv + i);
    }
    return Parse(vec);
}

void ArgParser::PrintError(const ArgParserError& error) {
    std::cerr << "error: ";
    switch (error) {
        case HelpArgumentAlreadyExists:
            std::cerr << "help argument already exists" << std::endl;
            break;
        default:
            std::cerr << "unknown error" << std::endl;
    }
    exit(EXIT_FAILURE);
}

void ArgParser::PrintError(const ArgParserError& error, const std::string& long_name) {
    std::cerr << "error: ";
    switch (error) {
        case ArgumentAlreadyExists:
            std::cerr << "argument --" << long_name << " already exists" << std::endl;
            break;
        case UnknownArgument:
            std::cerr << "unknown argument: --" << long_name << std::endl;
            break;
        case NoPositionalArgument:
            std::cerr << "no positional argument for the value " << long_name << std::endl;
            break;
        default:
            std::cerr << "unknown error" << std::endl;
    }
    exit(EXIT_FAILURE);
}

void ArgParser::PrintError(const ArgParserError& error, char short_name) {
    std::cerr << "error: ";
    switch (error) {
        case ArgumentAlreadyExists:
            std::cerr << "argument -" << short_name << " already exists" << std::endl;
            break;
        case UnknownArgument:
            std::cerr << "unknown argument: -" << short_name << std::endl;
            break;
        default:
            std::cerr << "unknown error" << std::endl;
    }
    exit(EXIT_FAILURE);
}
