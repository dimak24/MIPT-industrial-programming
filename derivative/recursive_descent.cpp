#include <assert.h>
#include <stdio.h>
#include "derivative.h"
#include "to_latex.h"

/*
    Grammar:     G >> E\0
    Expression:  E >> S{[+-]S}*
    Summand:     S >> M{[/*]M}*
    Multiplier:  M >> P|P^P
    Paranthesis: P >> (E)|N
    Number:      N >> [0-9]+
*/



int p = 0;
const char* s = nullptr;

Node getE();


auto getN() {
    int val = 0;

    int p0 = p;
    while ('0' <= s[p] && s[p] <= '9') {
        val = val * 10 + (s[p] - '0');   
        ++p;
    }

    assert(p0 != p);

    return MAKE_CONST(val);
}

auto getP() {
    Node node;

    if (s[p] == '(') {
        ++p;
        node = getE();

        assert(s[p] == ')');
        ++p;
        return node;
    }

    node = getN();

    return node;
}

auto getM() {
    auto node = getP();

    if (s[p] == '^') {
        ++p;
        auto node2 = getP();

        return node ^ node2;
    }

    return node;
}

auto getS() {
    auto node = getM();

    while (s[p] == '*' || s[p] == '/') {
        char op = s[p];
        ++p;
        auto node2 = getM();

        if (op == '*')
            node *= node2;
        else
            node /= node2;
    }

    return node;
}

Node getE() {
    auto node = getS();

    while (s[p] == '-' || s[p] == '+') {
        char op = s[p];
        ++p;
        auto node2 = getS();

        if (op == '-')
            node -= node2;
        else
            node += node2;
    }

    return node;
}

auto getG(const char* str) {
    s = str;
    p = 0;

    Node node = getE();
    
    assert(s[p] == '\0');
    ++p;

    return node;
}


int main() {
    auto node = getG("2+2^(0-2+11)");
    printf("%s\n", make_latex_document(to_latex(derivative(node))).c_str());
    // dump(&node);
}
