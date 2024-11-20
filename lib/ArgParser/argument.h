#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <optional>

namespace ArgumentParser {

class ArgParser;

enum ArgumentError {
    EmptyArgumentLongName,
    EmptyArgumentShortName,
    NoArgumentValue,
    InvalidArgumentType,
};

class ArgumentBase {
  public:
    explicit ArgumentBase(std::string long_name, std::string description = "");

    explicit ArgumentBase(char short_name, std::string long_name, std::string description = "");

    virtual ~ArgumentBase() = default;

    [[nodiscard]] std::string GetLongName() const;

    virtual void SetValueFromString(const std::string& value) = 0;

    [[nodiscard]] virtual bool HasValue() const = 0;

    [[nodiscard]] virtual const std::type_info& GetType() const = 0;

    [[nodiscard]] std::string GetTypeName() const;

    [[nodiscard]] virtual bool IsMultivalued() const = 0;

    [[nodiscard]] bool IsPositional() const;

    friend std::ostream& operator<<(std::ostream& os, const ArgumentBase& arg) {
        if (arg.short_name_.has_value()) {
            os << '-' << arg.short_name_.value() << ",  ";
        } else {
            os << "     ";
        }
        os << "--" << arg.long_name_;
        if (arg.GetType() != typeid(bool)) {
            os << "=<" << arg.GetTypeName() << ">";
        }
        if (!arg.description_.empty()) {
            os << ", " << arg.description_;
        }
        if (arg.IsPositional() || arg.IsMultivalued() || arg.HasDefaultValue()) {
            os << " [";
            bool space = false;
            if (arg.IsPositional()) {
                os << "positional";
                space = true;
            }
            if (arg.IsMultivalued()) {
                if (space) {
                    os << ", ";
                }
                os << "multivalued (min = " << arg.MinSize() << ")";
                space = true;
            }
            if (arg.HasDefaultValue()) {
                if (space) {
                    os << ", ";
                }
                os << "default = " << arg.DefaultValueString();
            }
            os << "]";
        }

        return os;
    }

    friend ArgParser;

  protected:
    [[nodiscard]] virtual bool HasDefaultValue() const = 0;

    [[nodiscard]] virtual size_t MinSize() const = 0;

    [[nodiscard]] virtual std::string DefaultValueString() const = 0;

    void PrintError(const ArgumentError& error) const;

    const std::string long_name_;
    const std::optional<char> short_name_;
    std::string description_;
    bool is_positional_ = false;
};

template<typename T, bool Multivalued = false>
class Argument : public ArgumentBase {
  public:
    explicit Argument(std::string long_name, const std::string& description = "")
            : ArgumentBase(std::move(long_name), std::move(description)) {}

    explicit Argument(char short_name, std::string long_name, std::string description = "")
            : ArgumentBase(short_name, std::move(long_name), std::move(description)) {}

    void SetValueFromString(const std::string& value) override {
        T val;

        std::istringstream stream(value);
        stream >> val;

        value_ = std::move(val);

        if (stream.fail()) {
            PrintError(InvalidArgumentType);
        }
    }

    Argument& SetValue(const T& value) {
        value_ = value;

        return *this;
    }

    [[nodiscard]] bool HasValue() const override {
        return value_.has_value() || default_value_.has_value();
    }

    T GetValue() const {
        if (!HasValue()) {
            PrintError(NoArgumentValue);
        }

        return value_.has_value() ? value_.value() : default_value_.value();
    }

    Argument& Default(const T& value) {
        default_value_ = value;

        return *this;
    }

    Argument& Positional() {
        is_positional_ = true;

        return *this;
    }

    T& GetStorage() {
        if (!value_.has_value()) {
            value_ = default_value_.has_value() ? default_value_.value() : T();
        }

        return value_.value();
    }

    const T& GetStorage() const {
        if (!value_.has_value()) {
            value_ = default_value_.has_value() ? default_value_.value() : T();
        }

        return value_.value();
    }

    [[nodiscard]] const std::type_info& GetType() const override {
        return typeid(T);
    }

    [[nodiscard]] bool IsMultivalued() const override {
        return false;
    }

  private:
    [[nodiscard]] bool HasDefaultValue() const override {
        return default_value_.has_value();
    }

    [[nodiscard]] size_t MinSize() const override {
        return 1;
    }

    [[nodiscard]] std::string DefaultValueString() const override {
        std::ostringstream ss;
        ss << std::boolalpha;

        ss << default_value_.value();

        return ss.str();
    }

    std::optional<T> value_;
    std::optional<T> default_value_;
};

template<typename T>
class Argument<T, true> : public ArgumentBase {
  public:
    explicit Argument(std::string long_name, size_t min_size, const std::string& description = "")
            : ArgumentBase(std::move(long_name), std::move(description)), min_size_(min_size) {}

    explicit Argument(char short_name, std::string long_name, size_t min_size, std::string description = "")
            : ArgumentBase(short_name, std::move(long_name), std::move(description)), min_size_(min_size) {}

    void SetValueFromString(const std::string& value) override {
        T val;

        std::istringstream stream(value);
        stream >> val;

        value_.push_back(std::move(val));

        if (stream.fail()) {
            PrintError(InvalidArgumentType);
        }
    }

    Argument& SetValue(const T& value) {
        value_.push_back(value);

        return *this;
    }

    [[nodiscard]] bool HasValue() const override {
        return value_.size() >= min_size_ || default_value_.has_value();
    }

    T GetValue(size_t index = 0) const {
        if (!HasValue() || index >= value_.size()) {
            PrintError(NoArgumentValue);
        }

        return index < value_.size() ? value_.at(index) : default_value_.value();
    }

    Argument& Default(const T& val) {
        default_value_ = val;

        return *this;
    }

    Argument& Positional() {
        is_positional_ = true;

        return *this;
    }

    std::vector<T>& GetStorage() {
        return value_;
    }

    const std::vector<T>& GetStorage() const {
        return value_;
    }

    [[nodiscard]] const std::type_info& GetType() const override {
        return typeid(T);
    }

    [[nodiscard]] bool IsMultivalued() const override {
        return true;
    }

  private:
    [[nodiscard]] bool HasDefaultValue() const override {
        return default_value_.has_value();
    }

    [[nodiscard]] size_t MinSize() const override {
        return min_size_;
    }

    [[nodiscard]] std::string DefaultValueString() const override {
        std::ostringstream ss;
        ss << std::boolalpha;

        ss << default_value_.value();

        return ss.str();
    }

    const size_t min_size_;
    std::vector<T> value_;
    std::optional<T> default_value_;
};

} // ArgumentParser