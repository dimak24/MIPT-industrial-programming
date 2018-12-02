#pragma once

#include <variant>
#include <exception>
#include <math.h>
#include <cstdlib>
#include <algorithm>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

#include "node.h"
#include "auxiliary.h"



struct derivative_exception : public std::exception {

};



Node derivative(const Node& node) { 
#define L *node.left
#define R *node.right
#define dL derivative(L)
#define dR derivative(R)
#define cL Node(L)
#define cR Node(R)

    if (IS_CONST(node))
        return MAKE_CONST(0);
    if (IS_VAR(node))
        return MAKE_CONST(1);                                                                            // TODO
    switch (OP(node)) {
        case Operator::OP_PLUS:
            return dL + dR;
        case Operator::OP_MINUS:
            return dL - dR;
        case Operator::OP_MUL:
            return dL * cR + cL * dR;
        case Operator::OP_DIV:
            return (dL * cR - cL * dR) / (cR * cR);
        case Operator::OP_POW:
            return IS_CONST(R) ? cR * (cL ^ MAKE_CONST(VALUE(R) - 1)) * dL
                               : (cL ^ cR) * (dR * ln(cL) + (cR * dL) / cL);
        case Operator::OP_LN:
            return dR / cR;
        default:
            throw derivative_exception();
    }

#undef cR
#undef cL
#undef dR
#undef dL
#undef R
#undef L
}


void append_dump(Node* root, std::string& ans, unsigned indent = 0) {
    if (!root)
        return;

    std::string value = "";
    if (IS_OP(*root)) {
        switch (OP(*root)) {
            case OP_PLUS:
                value = "+";
                break;
            case OP_MINUS:
                value = "-";
                break;
            case OP_MUL:
                value = "*";
                break;
            case OP_DIV:
                value = "/";
                break;
            case OP_POW:
                value = "^";
                break;
            case OP_LN:
                value = "ln";
                break;
            default:
                throw derivative_exception();
        }
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
    
    unsigned argnum = (root->left != nullptr) + (root->right != nullptr);
    if (argnum == 2)
        ans += std::to_string((size_t)root->left) + ", " + std::to_string((size_t)root->right);
    else if (argnum)
        ans += std::to_string((size_t)(root->left ? root->left : root->right));
    
    ans +=  "};\n";

    append_dump(root->left, ans, indent);
    append_dump(root->right, ans, indent);
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
    Node* root = new Node(nullptr);
    Node* node = root;

    const char* end = expression + strlen(expression);

    for (const char* ch = expression; ch != end; ++ch) {
        if (*ch == ' ')
            continue;
        if (*ch == '(') {
            if (IS_UNDEFINED(*node)) {
                node->left = new Node(node);
                node = node->left;
            }
            else {
                node->right = new Node(node);
                node = node->right;
            }
        }
        else if (*ch == ')') {
            node = node->parent;
        }
        else if (*ch == var /* is var */)                                                                           // TODO
            node->data = {NodeType::NODE_VAR, "x"};
        else if (*ch == '+' || *ch == '-' || *ch == '*' || *ch == '/' || *ch == '^') {
            switch (*ch) {
                case '+':
                    node->data = NodeData{NodeType::NODE_OP, Operator::OP_PLUS};
                    break;
                case '-':
                    node->data = NodeData{NodeType::NODE_OP, Operator::OP_MINUS};
                    break;
                case '*':
                    node->data = NodeData{NodeType::NODE_OP, Operator::OP_MUL};
                    break;
                case '/':
                    node->data = NodeData{NodeType::NODE_OP, Operator::OP_DIV};
                    break;
                case '^':
                    node->data = NodeData{NodeType::NODE_OP, Operator::OP_POW};
                    break;
                default:
                    throw derivative_exception();
            }
        }
        else if (strlen(ch) >= 2 && *ch == 'l' && *(ch + 1) == 'n') {                   
            ++ch;
            node->data = NodeData{NodeType::NODE_OP, Operator::OP_LN};
        }
        else {
            char* double_end = nullptr;
            double value = strtod(ch, &double_end);

            node->data = NodeData{NodeType::NODE_CONST, value};

            ch = --double_end;
        }
    }

    return *root->left;
}

Node deserialize(std::string expression, char var = 'x') {
    return deserialize(expression.c_str(), var);
}


std::string serialize(const Node& root) {
    std::string ans = "( ";
    if (IS_OP(root)) {
        if (root.left)
            ans += serialize(*root.left);
        switch(OP(root)) {
            case Operator::OP_PLUS:
                ans += " + ";
                break;
            case Operator::OP_MINUS:
                ans += " - ";
                break;
            case Operator::OP_MUL:
                ans += " * ";
                break;
            case Operator::OP_DIV:
                ans += " / ";
                break;
            case Operator::OP_POW:
                ans += " ^ ";
                break;
            case Operator::OP_LN:
                ans += "ln";
                break;
            default:
                throw derivative_exception();
        }
        if (root.right)
            ans += serialize(*root.right);
    }
    else if (IS_VAR(root)) {
        ans += NAME(root);
    }
    else /* IS_CONST(root) */ { 
        ans += eat_extra_zeros(std::to_string(VALUE(root)));
    }
    return ans + " )";
}
