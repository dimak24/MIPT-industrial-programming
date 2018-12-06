#pragma once

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <variant>
#include <vector>

#include "node.h"
#include "auxiliary.h"



struct derivative_exception : public std::exception {
private:
    const char* msg_;

public:
    derivative_exception(const char* msg)
        : msg_(msg) {}

    virtual const char* what() noexcept {
        return msg_;
    }
};



Node derivative(const Node& node, const char* var = "x") {
#define ARG(n) *node.children[n]
#define L ARG(0)
#define R ARG(1)
#define dARG(n) *node,children[n]
#define dL derivative(L, var)
#define dR derivative(R, var)
#define cL Node(L)
#define cR Node(R)

    if (IS_CONST(node))
        return MAKE_CONST(0);
    if (IS_VAR(node))
        return MAKE_CONST(!strcmp(var, NAME(node)));
    switch (OP(node)) {
        case OP_PLUS:
            return dL + dR;
        case OP_MINUS:
            return dL - dR;
        case OP_MUL:
            return dL * cR + cL * dR;
        case OP_DIV:
            return (dL * cR - cL * dR) / (cR * cR);
        case OP_POW:
            return IS_CONST(R) ? cR * (cL ^ MAKE_CONST(VALUE(R) - 1)) * dL
                               : (cL ^ cR) * (dR * ln(cL) + (cR * dL) / cL);
        case OP_LN:
            return dL / cL;
        default:
            throw derivative_exception("unknown operator");
    }

#undef cR
#undef cL
#undef dR
#undef dL
#undef dARG
#undef R
#undef L
#undef ARG
}


void append_dump(Node* root, std::string& ans, unsigned indent = 0) {
    if (!root)
        return;

    std::string value = "";
    if (IS_OP(*root))
        switch (OP(*root)) {
#define DEF_OP(name, type, mnemonic, arg_num, f) \
            case OP_##name: \
                value = mnemonic; \
                break;
#include "operators.h"
#undef DEF_OP
            default:
                throw derivative_exception("unknown operator");
        }
    else if (IS_CONST(*root))
        value = std::to_string(VALUE(*root));
    else
        value = NAME(*root);

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
#define DEF_OP(name, type, mnemonic, arg_num, f) \
            if (strlen(ch) >= strlen(mnemonic) && !strncmp(ch, mnemonic, strlen(mnemonic))) { \
                node->data = {NODE_OP, OP_##name}; \
                ch += strlen(mnemonic) - 1; \
                continue; \
            }
#include "operators.h"
#undef DEF_OP
            throw derivative_exception("could not parse expr");
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
#define DEF_OP(name, type, mnemonic, arg_num, f) \
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
                for (auto i = 0; i < arg_num; ++i) \
                    ans += ", " + serialize(*root.children[i]); \
                ans += ")"; \
                return ans;
#include "operators.h"
#undef DEF_OP
            default:
                throw derivative_exception("unknown operator");
        }
    }
    else if (IS_VAR(root))
        ans += NAME(root);
    else /* IS_CONST(root) */ 
        ans += eat_extra_zeros(std::to_string(VALUE(root)));

    return ans + " )";
}
