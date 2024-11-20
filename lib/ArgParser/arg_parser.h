#pragma once

#include "argument.h"

#include <unordered_map>
#include <memory>

namespace ArgumentParser {

enum ArgParserError {
    ArgumentAlreadyExists,
    HelpArgumentAlreadyExists,
    UnknownArgument,
    NoPositionalArgument,
};

class ArgParser {
  public:
    explicit ArgParser(std::string name);

    Argument<bool>& AddHelp(const std::string& description = "");

    [[nodiscard]] std::string HelpDescription() const;

    [[nodiscard]] bool Help() const;

    template<typename T>
    Argument<T>& AddArgument(const std::string& long_name, const std::string& description = "") {
        auto argument = std::make_unique<Argument<T>>(long_name, description);
        auto [it, inserted] = argument_map_.emplace(long_name, std::move(argument));

        if (!inserted) {
            PrintError(ArgumentAlreadyExists, long_name);
        }

        return *dynamic_cast<Argument<T>*>(it->second.get());
    }

    template<typename T>
    Argument<T>& AddArgument(char short_name, const std::string& long_name, const std::string& description = "") {
        auto [it_name, inserted_name] = long_name_map_.emplace(short_name, long_name);

        if (!inserted_name) {
            PrintError(ArgumentAlreadyExists, short_name);
        }

        auto argument = std::make_unique<Argument<T>>(short_name, long_name, description);
        auto [it_arg, inserted_arg] = argument_map_.emplace(long_name, std::move(argument));

        if (!inserted_arg) {
            PrintError(ArgumentAlreadyExists, long_name);
        }

        return *dynamic_cast<Argument<T>*>(it_arg->second.get());
    }

    Argument<bool>& AddFlag(const std::string& long_name, const std::string& description = "");

    Argument<bool>& AddFlag(char short_name, const std::string& long_name, const std::string& description = "");

    template<typename T, size_t min_size>
    Argument<T, true>& AddArgument(const std::string& long_name, const std::string& description = "") {
        auto argument = std::make_unique<Argument<T, true>>(long_name, min_size, description);
        auto [it, inserted] = argument_map_.emplace(long_name, std::move(argument));

        if (!inserted) {
            PrintError(ArgumentAlreadyExists, long_name);
        }

        return *dynamic_cast<Argument<T, true>*>(it->second.get());
    }

    template<typename T, size_t min_size>
    Argument<T, true>& AddArgument(char short_name, const std::string& long_name, const std::string& description = "") {
        auto [it_name, inserted_name] = long_name_map_.emplace(short_name, long_name);

        if (!inserted_name) {
            PrintError(ArgumentAlreadyExists, short_name);
        }

        auto argument = std::make_unique<Argument<T, true>>(short_name, long_name, min_size, description);
        auto [it_arg, inserted_arg] = argument_map_.emplace(long_name, std::move(argument));

        if (!inserted_arg) {
            PrintError(ArgumentAlreadyExists, long_name);
        }

        return *dynamic_cast<Argument<T, true>*>(it_arg->second.get());
    }

    template<typename T>
    T GetArgumentValue(const std::string& long_name) {
        auto iter = argument_map_.find(long_name);

        if (iter == argument_map_.end()) {
            PrintError(UnknownArgument, long_name);
        }

        ArgumentBase* argument = iter->second.get();

        if (argument->GetType() != typeid(T)) {
            argument->PrintError(InvalidArgumentType);
        }

        return dynamic_cast<Argument<T>*>(argument)->GetValue();
    }

    template<typename T>
    T GetArgumentValue(char short_name) {
        auto iter_short = long_name_map_.find(short_name);

        if (iter_short == long_name_map_.end()) {
            PrintError(UnknownArgument, short_name);
        }

        const std::string long_name = iter_short->second;

        const auto iter_arg = argument_map_.find(long_name);

        if (iter_arg == argument_map_.end()) {
            PrintError(UnknownArgument, long_name);
        }

        ArgumentBase* argument = iter_arg->second.get();

        if (argument->GetType() != typeid(T)) {
            argument->PrintError(InvalidArgumentType);
        }

        return dynamic_cast<Argument<T>*>(argument)->GetValue();
    }

    bool GetFlagValue(const std::string& long_name);

    bool GetFlagValue(char short_name);

    template<typename T>
    T GetArgumentValue(const std::string& long_name, size_t index) {
        auto iter = argument_map_.find(long_name);

        if (iter == argument_map_.end()) {
            PrintError(UnknownArgument, long_name);
        }

        ArgumentBase* argument = iter->second.get();

        if (argument->GetType() != typeid(T)) {
            argument->PrintError(InvalidArgumentType);
        }

        return dynamic_cast<Argument<T, true>*>(argument)->GetValue(index);
    }

    template<typename T>
    T GetArgumentValue(char short_name, size_t index) {
        auto iter_short = long_name_map_.find(short_name);

        if (iter_short == long_name_map_.end()) {
            PrintError(UnknownArgument, short_name);
        }

        const std::string long_name = iter_short->second;

        const auto iter_arg = argument_map_.find(long_name);

        if (iter_arg == argument_map_.end()) {
            PrintError(UnknownArgument, long_name);
        }

        ArgumentBase* argument = iter_arg->second.get();

        if (argument->GetType() != typeid(T)) {
            argument->PrintError(InvalidArgumentType);
        }

        return dynamic_cast<Argument<T, true>*>(argument)->GetValue(index);
    }

    bool Parse(const std::vector<std::string>& vec);

    bool Parse(int argc, char** argv);

  private:
    static void PrintError(const ArgParserError& error);

    static void PrintError(const ArgParserError& error, const std::string& long_name);

    static void PrintError(const ArgParserError& error, char short_name);

    const std::string name_;
    std::string description_;

    Argument<bool>* help_argument_ = nullptr;
    ArgumentBase* positional_argument_ = nullptr;

    std::unordered_map<std::string, std::unique_ptr<ArgumentBase>> argument_map_;
    std::unordered_map<char, std::string> long_name_map_;
};

} // ArgumentParser
