#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
   /* Реализуйте Node, используя std::variant */
    Node() : value_(nullptr) {};
    

    template <typename Type>
    Node(Type value)
        : value_(std::move(value)) {
    }

    const Value& GetValue() const {
         return value_;
    }

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    bool operator== (const Node& other) const {
        if (IsNull() && other.IsNull()) {
            return true;
        } else if (IsInt() && other.IsInt()) {
            return AsInt() == other.AsInt();
        } else if (IsPureDouble() && other.IsPureDouble()) {
            return AsDouble() == other.AsDouble();
        } else if (IsBool() && other.IsBool()) {
            return AsBool() == other.AsBool();
        } else if (IsString() && other.IsString()) {
            return AsString() == other.AsString();
        } else if (IsArray() && other.IsArray()) {
            return AsArray() == other.AsArray();
        } else if (IsMap() && other.IsMap()) {
            return AsMap() == other.AsMap();
        } else {
            return false;
        }
    }

private:
    Value value_;
};

inline bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

inline bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

inline bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json