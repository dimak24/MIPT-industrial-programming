#pragma once

#include <array>
#include <cmath>
#include <functional>
#include <string.h>
#include <variant>
#include <vector>


enum Operator {
#define DEF_OP(name, type, mnemonic, arg_num, f) OP_##name,
#include "operators.h"
#undef DEF_OP

    __OP_LAST__,
};

enum OperatorType {
    OT_OTHER,
    OT_UNARY,
    OT_BINARY
};

constexpr auto _get_operators_types() {
    std::array<OperatorType, __OP_LAST__> ans = {};

#define DEF_OP(name, type, mnemonic, arg_num, f) ans[OP_##name] = OT_##type;
#include "operators.h"
#undef DEF_OP

    return ans;
}

// constexpr auto _get_operators_mnemonics() {
//     std::array<const char*, __OP_LAST__> ans = {};

// #define DEF_OP(name, type, mnemonic, arg_num, f) ans[OP_##name] = mnemonic;
// #include "operators.h"
// #undef DEF_OP
    
//     return ans;
// }

// constexpr auto _get_operators_arg_num() {
//     std::array<unsigned, __OP_LAST__> ans = {};

// #define DEF_OP(name, type, mnemonic, arg_num, f) ans[OP_##name] = arg_num;
// #include "operators.h"
// #undef DEF_OP
    
//     return ans;
// }

// constexpr auto _get_operators_func() {
//     std::array<std::, __OP_LAST__> ans = {};

// #define DEF_OP(name, type, mnemonic, arg_num, f) ans[OP_##name] = arg_num;
// #include "operators.h"
// #undef DEF_OP
    
//     return ans;
// }


constexpr auto _OPERATORS_TYPES = _get_operators_types();

#define IS_BINARY(op) (_OPERATORS_TYPES[op] == OT_BINARY)
#define IS_UNARY(op) (_OPERATORS_TYPES[op] == OT_UNARY)


enum NodeType {
    NODE_UNDEFINED,
    NODE_VAR,
    NODE_OP,
    NODE_CONST
};


struct NodeData {
    NodeType type;
    std::variant<const char*, Operator, double> value = 0;
};


struct Node {
    NodeData data;
    std::vector<Node*> children;

    Node* parent;


    template <typename... Args>
    Node(NodeData data, Args... args)
        : data(data), parent(nullptr), children(std::initializer_list<Node*>({args...})) {
            for (auto& child : children)
                child->parent = this;
        }

    Node(const Node& another)
        : data(another.data), parent(nullptr) {
            for (auto& child : another.children)
                children.push_back(new Node(*child));
        }

    Node& operator=(const Node& another) {
        data = another.data;
        parent = nullptr;

        for (auto& child : another.children)
            children.push_back(new Node(*child));
        
        return *this;
    }

    Node() : parent(nullptr) {}

    Node* init_new_child() {
        children.push_back(new Node());
        children.back()->parent = this;
        return children.back();
    }

    ~Node() {
        for (auto& child : children)
            delete child;
    }
};


#define IS_OP(node) ((node).data.type == NODE_OP)
#define IS_VAR(node) ((node).data.type == NODE_VAR)
#define IS_CONST(node) ((node).data.type == NODE_CONST)
#define IS_UNDEFINED(node) ((node).data.type == NODE_UNDEFINED)
#define MAKE_CONST(value) Node({NODE_CONST, (value)})
#define MAKE_VAR(name) Node({NODE_VAR, name})

template <Operator op, typename... Args>
auto MAKE_OP(Args... args) {
    // static_assert(sizeof...(Args) == get_arg_num(op));
    return Node({NODE_OP, op}, new Node(args)...);
}

#define OP(node) (std::get<Operator>((node).data.value))
#define VALUE(node) (std::get<double>((node).data.value))
#define NAME(node) (std::get<const char*>((node).data.value))
#define EPS 1e-5
#define EQUAL(value1, value2) (fabs((value1) - (value2)) < EPS)



Node operator+(const Node& left, const Node& right) {
    if (IS_CONST(left) && IS_CONST(right))
        return MAKE_CONST(VALUE(left) + VALUE(right));

    if (IS_CONST(left) && EQUAL(VALUE(left), 0))
        return Node(right);

    if (IS_CONST(right) && EQUAL(VALUE(right), 0))
        return Node(left);
    
    return MAKE_OP<OP_PLUS>(left, right);
}


Node operator-(const Node& left, const Node& right) {
    if (IS_CONST(left) && IS_CONST(right))
        return MAKE_CONST(VALUE(left) - VALUE(right));

    if (IS_CONST(right) && EQUAL(VALUE(right), 0))
        return Node(left);
    
    return MAKE_OP<OP_MINUS>(left, right);
}


Node operator*(const Node& left, const Node& right) {
    if (IS_CONST(left) && IS_CONST(right))
        return MAKE_CONST(VALUE(left) * VALUE(right));

    if ((IS_CONST(left) && EQUAL(VALUE(left), 0)) || (IS_CONST(right) && EQUAL(VALUE(right), 0)))
        return MAKE_CONST(0);

    if (IS_CONST(left) && EQUAL(VALUE(left), 1))
        return Node(right);

    if (IS_CONST(right) && EQUAL(VALUE(right), 1))
        return Node(left);
    
    return MAKE_OP<OP_MUL>(left, right);
}


Node operator/(const Node& left, const Node& right) {
    if (IS_CONST(left) && IS_CONST(right))
        return MAKE_CONST(VALUE(left) / VALUE(right));

    if (IS_CONST(left) && EQUAL(VALUE(left), 0))
        return MAKE_CONST(0);

    if (IS_CONST(right) && EQUAL(VALUE(right), 1))
        return Node(left);

    if (IS_VAR(left) && IS_VAR(right) && !strcmp(NAME(left), NAME(right)))
        return MAKE_CONST(1);
    
    return MAKE_OP<OP_DIV>(left, right);
}


Node operator^(const Node& left, const Node& right) {
    if (IS_CONST(left) && IS_CONST(right))
        return MAKE_CONST(pow(VALUE(left), VALUE(right)));

    if (IS_CONST(left) && EQUAL(VALUE(left), 0))
        return MAKE_CONST(0);

    if ((IS_CONST(right) && EQUAL(VALUE(right), 0)) || (IS_CONST(left) && EQUAL(VALUE(left), 1)))
        return MAKE_CONST(1);

    return MAKE_OP<OP_POW>(left, right);
}


Node ln(const Node& node) {
    if (IS_CONST(node))
        return MAKE_CONST(log(VALUE(node)));

    return MAKE_OP<OP_LN>(node);
}


Node& operator+=(Node& left, const Node& right) {
    return left = left + right;
}

Node& operator-=(Node& left, const Node& right) {
    return left = left - right;
}

Node& operator*=(Node& left, const Node& right) {
    return left = left * right;
}

Node& operator/=(Node& left, const Node& right) {
    return left = left / right;
}

Node& operator^=(Node& left, const Node& right) {
    return left = left ^ right;
}
