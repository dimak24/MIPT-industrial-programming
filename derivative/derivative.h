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
        case OP_SIN:
            return dL * MAKE_OP<OP_COS>(L);
        case OP_COS:
            return -dL * MAKE_OP<OP_SIN>(L);
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
