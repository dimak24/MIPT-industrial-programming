#pragma once

#include <exception>

#include "lexer.h"
#include "node.h"


class parser_exception : std::exception {
private:
    const char* msg_;

public:
    parser_exception(const char* msg)
        : msg_(msg) {}

    virtual const char* what() noexcept {
        return msg_;
    }
};



class Parser {
private:
    Lexer lexer_;

    Node getG_() {
        return getN_();
    }

    Node getN_() {
        auto lexeme = lexer_.next_lexeme();
        if (lexeme.type != LT_NUMBER)
            throw parser_exception("expected number");

        double ans = 0, p = 1;
        bool point = false;
        for (auto j = lexeme.data.data(); j != lexeme.data.data() + lexeme.data.size(); ++j) {
            if (*j == '.')
                point = true;
            else if (!point)
                ans = ans * 10 + (*j - '0');
            else
                ans += (*j - '0') / (p *= 10);
        }

        return MAKE_CONST(ans);
    }

public:
    Parser() = delete;
    ~Parser() {}

    Parser(Lexer lexer)
        : lexer_(lexer) {}


    Node parse() {
        return getG_();
    }
};
