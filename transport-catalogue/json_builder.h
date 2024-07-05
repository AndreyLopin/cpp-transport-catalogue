#pragma once

#include <string>
#include <utility>
#include <vector>
#include "json.h"

namespace json {

class Builder {
    class BaseContext;
    class KeyItemContext;
    class ArrayItemContext;
    class DictItemContext;
public:
    Builder();

    Builder& Value(Node::Value value) {
        AddObject(std::move(value), /* one_shot */ true);
        return *this;
    }

    Builder& EndDict();
    Builder& EndArray();
    Node Build();

    Builder::KeyItemContext Key(std::string key) {
        using namespace std::string_literals;
        Node::Value& host_value = GetCurrentValue();
        
        if (!std::holds_alternative<Dict>(host_value)) {
            throw std::logic_error("Key() outside a dict"s);
        }
        
        nodes_stack_.emplace_back(
            &std::get<Dict>(host_value)[std::move(key)]
        );
        return *this;
    }

    Builder::DictItemContext StartDict() {
        AddObject(Dict{}, /* one_shot */ false);
        return BaseContext{*this};
    }

    Builder::ArrayItemContext StartArray() {
        AddObject(Array{}, /* one_shot */ false);
        return BaseContext{*this};
    }

private:
    Node root_;
    std::vector<Node*> nodes_stack_;

    Node::Value& GetCurrentValue();
    const Node::Value& GetCurrentValue() const;
    
    void AssertNewObjectContext() const;
    void AddObject(Node::Value value, bool one_shot);

    class BaseContext {
    public:
        BaseContext(Builder& builder) : builder_(builder) {}
        Node Build();

        KeyItemContext Key(std::string key) {
            return builder_.Key(std::move(key));
        }

        BaseContext Value(Node::Value value) {
            return builder_.Value(std::move(value));
        }

        DictItemContext StartDict() {
            return builder_.StartDict();
        }

        ArrayItemContext StartArray() {
            return builder_.StartArray();
        }

        Builder& EndDict() {
            return builder_.EndDict();
        }

        Builder& EndArray() {
            return builder_.EndArray();
        }
    private:
        Builder& builder_;
    };

    class KeyItemContext : public BaseContext {
    public:
        KeyItemContext(Builder& builder) : BaseContext(builder) {}
        KeyItemContext Key(std::string key) = delete;
        DictItemContext Value(Node::Value value) {
            return BaseContext::Value(std::move(value));
        }
        Builder& EndArray() = delete;
        Builder& EndDict() = delete;
        Node Build() = delete;
    };

    class ArrayItemContext : public BaseContext {
    public:
        ArrayItemContext(BaseContext base) : BaseContext(base) {}
        KeyItemContext Key(std::string key) = delete;
        ArrayItemContext Value(Node::Value value) {
            return BaseContext::Value(std::move(value));
        }
        Builder& EndDict() = delete;
        Node Build() = delete;
    };

    class DictItemContext : public BaseContext {
    public:
        DictItemContext(BaseContext base) : BaseContext(base) {}
        Builder& Value(Node::Value value) = delete;
        ArrayItemContext StartArray() = delete;
        Builder& EndArray() = delete;
        DictItemContext StartDict() = delete;
        Node Build() = delete;
    };
};

}  // namespace json