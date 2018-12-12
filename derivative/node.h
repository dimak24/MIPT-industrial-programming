#pragma once

#include <array>
#include <cmath>
#include <functional>
#include <string.h>
#include <variant>
#include <vector>

#include "auxiliary.h"


struct serialization_deserialization_exception : public std::exception {
private:
    const char* msg_;

public:
    serialization_deserialization_exception(const char* msg)
        : msg_(msg) {}

    virtual const char* what() noexcept {
        return msg_;
    }
};



enum Operator {
#define DEF_OP(name, mnemonic) OP_##name,
#define DEF_MATH_OP(name, type, mnemonic, latex_command, arg_num, proc_command) DEF_OP(name, mnemonic)
#define DEF_ASSIGN_OP(name, mnemonic, proc_command) DEF_OP(name, mnemonic)
#define DEF_KEYWORD(name, mnemonic) DEF_OP(name, mnemonic)
#include "operators.h"
#undef DEF_KEYWORD
#undef DEF_ASSIGN_OP
#undef DEF_MATH_OP
#undef DEF_OP
    
    __OP_LAST__,
};

enum OperatorType {
    OT_BINARY,
    OT_UNARY,
    OT_FUNC
};

constexpr auto _get_operators_types() {
    std::array<OperatorType, __OP_LAST__> ans = {};

#define DEF_MATH_OP(name, type, mnemonic, latex_command, arg_num, proc_command) ans[OP_##name] = OT_##type;
#include "operators.h"
#undef DEF_MATH_OP

    return ans;
}

constexpr auto _OPERATORS_TYPES = _get_operators_types();

#define IS_BINARY(op) (_OPERATORS_TYPES[op] == OT_BINARY)
#define IS_UNARY(op) (_OPERATORS_TYPES[op] == OT_UNARY)


enum NodeType {
    NODE_UNDEFINED,
    NODE_VAR,
    NODE_OP,
    NODE_CONST,
    NODE_CALL,
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
        : data(data), children(std::initializer_list<Node*>({args...})), parent(nullptr) {
            for (auto& child : children)
                child->parent = this;
        }

    Node(const Node& another)
        : data(another.data), parent(nullptr) {
            for (const auto& child : another.children)
                children.push_back(new Node(*child));
        }

    Node& operator=(const Node& another) {
        // fix it
        children.clear();

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

    void adopt(const Node& node) {
        children.push_back(new Node(node));
        children.back()->parent = this;
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
#define IS_CALL(node) ((node).data.type == NODE_CALL)
#define MAKE_CONST(value) Node({NODE_CONST, (value)})
#define MAKE_VAR(name) Node({NODE_VAR, name})

template <Operator op, typename... Args>
auto MAKE_OP(Args... args) {
    return Node({NODE_OP, op}, new Node(args)...);
}

template <typename... Args>
auto MAKE_CALL(const char* name, const Node& func, Args... args) {
    return Node({NODE_CALL, name}, new Node({NODE_UNDEFINED}, new Node(args)...), new Node(func));
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

Node operator-(const Node& node) {
    return MAKE_OP<OP_MINUS>(MAKE_CONST(0), node);
}


void append_dump(Node* root, std::string& ans, unsigned indent = 0) {
    if (!root)
        return;

    std::string value = "";
    if (IS_OP(*root))
        switch (OP(*root)) {
#define DEF_OP(name, mnemonic) \
            case OP_##name: \
                value = mnemonic; \
                break;
#define DEF_MATH_OP(name, type, mnemonic, latex_command, arg_num, proc_command) DEF_OP(name, mnemonic)
#define DEF_ASSIGN_OP(name, mnemonic, proc_command) DEF_OP(name, mnemonic)
#define DEF_KEYWORD(name, mnemonic) DEF_OP(name, mnemonic)
#include "operators.h"
#undef DEF_KEYWORD
#undef DEF_ASSIGN_OP
#undef DEF_MATH_OP
#undef DEF_OP
            default:
                throw serialization_deserialization_exception("unknown operator");
        }
    else if (IS_CONST(*root))
        value = std::to_string(VALUE(*root));
    else if (IS_VAR(*root) || IS_CALL(*root))
        value = NAME(*root);
    else /* UNDEFINED */
        value = "_";

    std::string indent_str(indent, ' ');

    ans += indent_str + std::to_string((size_t)root) + " [label=\"" + value + "\""
                                                         "color=\"" + (IS_OP(*root)  ? "red" :
                                                                       IS_VAR(*root) ? "blue" :
                                                                                       "green") + "\"];\n";
    ans += indent_str + std::to_string((size_t)root) + "-> {";
    
    auto argnum = root->children.size();
    if (argnum) {
        ans += std::to_string((size_t)root->children[0]);
        for (auto i = 1u; i < argnum; ++i)
            ans += ", " + std::to_string((size_t)root->children[i]);
    }
    
    ans +=  "};\n";

    for (auto child : root->children)
        append_dump(child, ans, indent);
}


/* 
    prints dump in dot format
    run ./a.out > dump && dot -Tps dump -o graph.ps && xdg-open graph.ps
    to see it
*/
void dump(Node* root) {
    std::string to_dot;
    to_dot += "digraph G {";
    append_dump(root, to_dot, 4);
    to_dot += "}";
    printf("%s\n", to_dot.c_str());
}



Node deserialize(const char* expression, char var = 'x') {
    Node* root = new Node();
    Node* node = root;

    const char* end = expression + strlen(expression);

    for (const char* ch = expression; ch != end; ++ch) {
        if (*ch == ' ')
            continue;
        if (*ch == '(')
            node = node->init_new_child();
        else if (*ch == ')')
            node = node->parent;
        else if (*ch == var /* is var */)                                                                           // TODO
            node->data = {NodeType::NODE_VAR, "x"};
        else if (*ch == '.' || ('0' <= *ch && *ch <= '9')) {
            char* double_end = nullptr;
            double value = strtod(ch, &double_end);

            node->data = {NodeType::NODE_CONST, value};

            ch = --double_end;
        }
        else {
#define DEF_MATH_OP(name, type, mnemonic, latex_command, arg_num, proc_command) \
            if (strlen(ch) >= strlen(mnemonic) && !strncmp(ch, mnemonic, strlen(mnemonic))) { \
                node->data = {NODE_OP, OP_##name}; \
                ch += strlen(mnemonic) - 1; \
                continue; \
            }
#include "operators.h"
#undef DEF_MATH_OP
            throw serialization_deserialization_exception("could not parse expr");
        }
    }

    return *root->children[0];
}

Node deserialize(std::string expression, char var = 'x') {
    return deserialize(expression.c_str(), var);
}


std::string serialize(const Node& root) {
    std::string ans = "( ";
    if (IS_OP(root)) {
        switch (OP(root)) {
#define DEF_MATH_OP(name, type, mnemonic, latex_command, arg_num, proc_command) \
            case OP_##name: \
                if (IS_UNARY(OP_##name)) { \
                    ans += std::string(mnemonic) + serialize(*root.children[0]); \
                    break; \
                } \
                if (IS_BINARY(OP_##name)) { \
                    ans += serialize(*root.children[0]) + " " \
                                                          mnemonic \
                                                               " " + serialize(*root.children[1]); \
                    break; \
                } \
                ans += std::string(mnemonic) + "("; \
                if (arg_num) \
                    ans += serialize(*root.children[0]); \
                for (auto i = 1; i < arg_num; ++i) \
                    ans += ", " + serialize(*root.children[i]); \
                ans += ")"; \
                return ans;
#include "operators.h"
#undef DEF_MATH_OP
            default:
                throw serialization_deserialization_exception("unknown operator");
        }
    }
    else if (IS_VAR(root))
        ans += NAME(root);
    else /* IS_CONST(root) */ 
        ans += eat_extra_zeros(std::to_string(VALUE(root)));

    return ans + " )";
}
