#pragma once

#include <exception>
#include <string>

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

/*
    Grammar:      G >> OP+\0
    Expression:   E >> S{[+-]S}*
    Summand:      S >> M{[/*]M}*
    Multiplier:   M >> P|P^P
    Paranthesis:  P >> (E)|N
    Number:       N >> [0-9]+
    Identificator:ID >> [a-z,A-Z]+
    Operation:    OP >> ID[=]VAL
    Value:        VAL >> N|ID
    If:           IF >> if(E)OP
*/

class Parser {
private:
    Lexer lexer_;

    Node getG_() {
        auto p0 = lexer_.mark();
        auto lexeme = lexer_.next_lexeme();
        if (lexeme.type == LT_EOF)
            throw parser_exception("empty program");

        lexer_.reset(p0);
        auto node = getOP_();                                                                           // TODO

        while (true) {
            p0 = lexer_.mark();
            lexeme = lexer_.next_lexeme();
            lexer_.reset(p0);

            if (lexeme.type == LT_EOF)
                break;    
            
            node = getOP_();
        }
        return node;
    }

    Node getE_() {
        auto node = getS_();

        auto p0 = lexer_.mark();
        auto lexeme = lexer_.next_lexeme();

        while (lexeme.type == LT_PLUS || lexeme.type == LT_MINUS) {
            auto node1 = getS_();
            if (lexeme.type == LT_PLUS)
                node += node1;
            else if (lexeme.type == LT_MINUS)
                node -= node1;

            p0 = lexer_.mark();
            lexeme = lexer_.next_lexeme();
        }

        lexer_.reset(p0);
        return node;
    }

    Node getS_() {
        auto node = getM_();

        auto p0 = lexer_.mark();
        auto lexeme = lexer_.next_lexeme();

        while (lexeme.type == LT_MUL || lexeme.type == LT_DIV) {
            auto node1 = getM_();
            if (lexeme.type == LT_MUL)
                node *= node1;
            else if (lexeme.type == LT_DIV)
                node /= node1;

            p0 = lexer_.mark();
            lexeme = lexer_.next_lexeme();
        }

        lexer_.reset(p0);
        return node;
    }

    Node getM_() {
        auto node = getP_();

        auto p0 = lexer_.mark();

        while (lexer_.next_lexeme().type == LT_POW) {
            node ^= getP_();

            p0 = lexer_.mark();
        }

        lexer_.reset(p0);
        return node;
    }

    Node getP_() {
        try {
            return getN_();
        }
        catch (parser_exception& e) {
            if (lexer_.next_lexeme().type != LT_L_PARANTHESIS)
                throw parser_exception("expected '('");

            auto node = getE_();

            if (lexer_.next_lexeme().type != LT_R_PARANTHESIS)
                throw parser_exception("expected ')'");

            return node;
        }
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

    Node getID_() {
        auto lexeme = lexer_.next_lexeme();
        if (lexeme.type != LT_IDENTIFICATOR)
            throw parser_exception("expected identificator");

        return MAKE_VAR(std::string(lexeme.data.data(), lexeme.data.data()).c_str());
    }

    Node getVAL_() {
        try {
            return getID_();
        }
        catch (parser_exception&) {
            return getN_();
        }
    }

    Node getOP_() {
        auto node = getID_();
        auto lexeme = lexer_.next_lexeme();

        if (lexeme.type != LT_ASSIGN && lexeme.type != LT_PLUS_ASSIGN &&
            lexeme.type != LT_MINUS_ASSIGN && lexeme.type != LT_MUL_ASSIGN &&
            lexeme.type != LT_DIV_ASSIGN && lexeme.type != LT_POW_ASSIGN)
                throw parser_exception("expected operator");

        auto node2 = getVAL_();

        switch (lexeme.type) {
            case LT_ASSIGN:
                return MAKE_OP(node, OP_ASSIGN, node2);
            default:
                return MAKE_OP(node, OP_ASSIGN, node2);                                                 // TODO
        }
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
