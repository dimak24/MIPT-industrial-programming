#pragma once

#include <exception>
#include <string>
#include <string_view>

#include "derivative.h"
#include "lexer.h"
#include "node.h"


class parser_exception : std::exception {
private:
    const char* msg_;

public:
    parser_exception(const char* msg)
        : msg_(msg) {}

    virtual const char* what() {
        return msg_;
    }
};

/*
    Number:                    N >> [0-9]+
    Identificator:             ID >> [a-z,A-Z]+

    Array:                     A >> \[{E|{E,}*E}\]

    Expression:                E >> S{[+-]S}*
    Summand:                   S >> M{[/ *]M}*
    Multiplier:                M >> P|P^P
    Paranthesis:               P >> (E)|VAL

    Value:                     VAL >> N|ID

    Math function declaration: F >> ID(ID)[=]E'
    
    Call:                      C >> {ID|[*+]}({E,}*E)

    Operation:                 OP >> ID[=]E
    If:                        IF >> if(E) then G end;
    While:                     W >> till the cows come home repeat G end;

    Instruction:               I >> OP|IF|W|C|F
    Grammar:                   G >> {I;}+\0
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
        Node node;

        while (lexeme.type != LT_EOF) {
            node.adopt(getI_());

            p0 = lexer_.mark();
            lexeme = lexer_.next_lexeme();
            lexer_.reset(p0);
        }
        return node;
    }

    Node getC_() {                                                                                      // TODO
        auto p0 = lexer_.mark();
        auto lexeme = lexer_.next_lexeme();
        Node node;

        if (lexeme.type == LT_PLUS)
            node = MAKE_OP<OP_PLUS>();
        else if (lexeme.type == LT_MUL)
            node = MAKE_OP<OP_MUL>();
        else {
            lexer_.reset(p0);
            node = MAKE_VAR(NAME(getID_()));
        }

        if (lexer_.next_lexeme().type != LT_L_PARANTHESIS) {
            lexer_.reset(p0);
            throw parser_exception("expected (");
        }
        
        p0 = lexer_.mark();
        if (lexer_.next_lexeme().type == LT_R_PARANTHESIS)
            return node;
        lexer_.reset(p0);
        node.adopt(getE_());

        while (true) {
            p0 = lexer_.mark();
            if (lexer_.next_lexeme().type == LT_R_PARANTHESIS)
                return node;
            lexer_.reset(p0);
            if (lexer_.next_lexeme().type == LT_COMMA)
                node.adopt(getE_());
            throw parser_exception("wrong function's arguments");     
        }
    }

    Node getI_() {
        auto p0 = lexer_.mark();
        try {
            return getIF_();
        } catch (parser_exception&) {
            try {
                lexer_.reset(p0);
                return getW_();
            } catch (parser_exception&) {
                try {
                    lexer_.reset(p0);
                    return getF_();
                } catch (parser_exception&) {
                    lexer_.reset(p0);
                    try {
                        return getC_();
                    } catch (parser_exception&) {
                        lexer_.reset(p0);
                        return getOP_();
                    }
                }
            }
        }
    }

    Node getIF_() {
        if (lexer_.next_lexeme().type != LT_IF)
            throw parser_exception("if expected");

        auto cond = getE_();

        if (lexer_.next_lexeme().type != LT_THEN)
            throw parser_exception("then expected");

        auto instr = getG_();

        if (lexer_.next_lexeme().type != LT_END)
            throw parser_exception("end eapected");

        return MAKE_OP<OP_IF>(cond, instr);
    }

    Node getW_() {
        if (lexer_.next_lexeme().type != LT_WHILE)
            throw parser_exception("while expected");

        if (lexer_.next_lexeme().type != LT_REPEAT)
            throw parser_exception("repeat expected");

        auto instr = getG_();

        if (lexer_.next_lexeme().type != LT_END)
            throw parser_exception("end eapected");

        return MAKE_OP<OP_WHILE>(instr);
    }

    Node getF_() {
        auto name = getID_();

        if (lexer_.next_lexeme().type != LT_L_PARANTHESIS)
            throw parser_exception("( expected");

        auto var = getID_();

        if (lexer_.next_lexeme().type != LT_R_PARANTHESIS)
            throw parser_exception(") expected");

        if (lexer_.next_lexeme().type != LT_ASSIGN)
            throw parser_exception("= expected");

        auto expr = getE_();

        if (lexer_.next_lexeme().type != LT_DERIVATIVE)
            throw parser_exception("\' expected");

        return derivative(expr, NAME(var));                                                             // TODO: make it a function
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
            return getVAL_();
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
        auto p0 = lexer_.mark();
        auto lexeme = lexer_.next_lexeme();
        if (lexeme.type != LT_IDENTIFICATOR) {
            lexer_.reset(p0);
            throw parser_exception("expected identificator");
        }

        return MAKE_VAR(strdup(std::string(lexeme.data.data(), 
                                           lexeme.data.size()).c_str()));
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

        auto node2 = getE_();

        switch (lexeme.type) {
            case LT_ASSIGN:
                return MAKE_OP<OP_ASSIGN>(node, node2);
            default:
                return MAKE_OP<OP_ASSIGN>(node, node2);                                                 // TODO
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
