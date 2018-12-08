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
    Value:                     VAL >> N|ID

    CMPExpression:             CE >> E{[><]E}*
    Expression:                E >> S{[+-]S}*
    Summand:                   S >> M{[/ *]M}*
    Multiplier:                M >> P|P^P
    Paranthesis:               P >> (CE)|VAL|C
    
    Operation:                 OP >> ID [BINARY OP] CE | C
    Call:                      C >> {ID|[*+]}({CE,}*CE)

    If:                        IF >> if CE then G end;
    While:                     W >> till the cows come home repeat G end;

    Instruction:               I >> OP|IF|W|F
    Grammar:                   G >> {I;}+\0

    Math function declaration: F >> ID(ID)[=]E'
//////////////////////

    Array:                     A >> \[{CE|{CE,}*CE}\]



    

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

        while (lexeme.type != LT_EOF && lexeme.type != LT_END) {
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

        auto argnum = -2;

        if (false);
#define DEF_MATH_OP(name, _type, mnemonic, latex_command, arg_num) \
        else if (!strcmp(#_type, "FUNC") && lexeme.type == LT_##name) { \
            node = MAKE_OP<OP_##name>(); \
            argnum = arg_num; \
        }
#include "operators.h"
#undef DEF_MATH_OP
        else {
            lexer_.reset(p0);
            node = MAKE_VAR(NAME(getID_()));
        }
        if (lexer_.next_lexeme().type != LT_L_PARANTHESIS) {
            lexer_.reset(p0);
            throw parser_exception("expected (");
        }
        
        p0 = lexer_.mark();
        if (lexer_.next_lexeme().type == LT_R_PARANTHESIS) {
            if (argnum)
                throw parser_exception("expected more, than 0 arguments");
            return node;
        }

        lexer_.reset(p0);
        node.adopt(getCE_());

        auto args = 1;
        while (true) {
            p0 = lexer_.mark();
            if (lexer_.next_lexeme().type == LT_R_PARANTHESIS) {
                if (argnum != -2 && argnum != args)
                    throw parser_exception("wrong number of arguments");
                return node;
            }

            lexer_.reset(p0);
            if (lexer_.next_lexeme().type == LT_COMMA) {
                node.adopt(getCE_());
                ++args;
            }
            else
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
        auto cond = getCE_();

        if (lexer_.next_lexeme().type != LT_THEN)
            throw parser_exception("then expected");

        auto instr = getG_();

        if (lexer_.next_lexeme().type != LT_END)
            throw parser_exception("end expected");

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

        auto expr = getCE_();

        if (lexer_.next_lexeme().type != LT_DERIVATIVE)
            throw parser_exception("\' expected");

        return derivative(expr, NAME(var));                                                             // TODO: make it a function
    }

    Node getCE_() {
        auto node = getE_();

        auto p0 = lexer_.mark();
        auto lexeme = lexer_.next_lexeme();

        while (lexeme.type == LT_LT || lexeme.type == LT_LE ||
               lexeme.type == LT_GT || lexeme.type == LT_GE ||
               lexeme.type == LT_EQ || lexeme.type == LT_NE) {
            auto node1 = getE_();

            switch (lexeme.type) {
#define DEF_OP(name) \
                case LT_##name: \
                    node = MAKE_OP<OP_##name>(node, node1); \
                    break;
#define DEF_MATH_OP(name, type, mnemonic, latex_command, arg_num) DEF_OP(name)
// #define DEF_ASSIGN_OP(name, mnemonic) DEF_OP(name)
#include "operators.h"
// #undef DEF_ASSIGN_OP
#undef DEF_MATH_OP
#undef DEF_OP
                default: __builtin_unreachable();
            }

            p0 = lexer_.mark();
            lexeme = lexer_.next_lexeme();
        }

        lexer_.reset(p0);
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
            return getVAL_();
        }
        catch (parser_exception&) {
            try {
                return getC_();
            } catch (parser_exception&) {
                if (lexer_.next_lexeme().type != LT_L_PARANTHESIS)
                    throw parser_exception("expected '('");

                auto node = getCE_();

                if (lexer_.next_lexeme().type != LT_R_PARANTHESIS)
                    throw parser_exception("expected ')'");

                return node;
            }
        }
    }

    Node getN_() {
        auto p0 = lexer_.mark();
        auto lexeme = lexer_.next_lexeme();

        if (lexeme.type != LT_NUMBER) {
            lexer_.reset(p0);
            throw parser_exception("expected number");
        }

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
        try {
            return getC_();
        } catch (parser_exception&) {
            auto p0 = lexer_.mark();
            auto node = getID_();
            auto lexeme = lexer_.next_lexeme();
            auto node2 = getCE_();

            switch (lexeme.type) {                                                                      // LOL
#define DEF_OP(name) \
                case LT_##name: \
                    return MAKE_OP<OP_##name>(node, node2);
#define DEF_MATH_OP(name, type, mnemonic, latex_command, arg_num) DEF_OP(name)
#define DEF_ASSIGN_OP(name, mnemonic) DEF_OP(name)
#include "operators.h"
#undef DEF_ASSIGN_OP
#undef DEF_MATH_OP
#undef DEF_OP
                default:
                    lexer_.reset(p0);
                    throw parser_exception("operator expected");
            }
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


void generate_asm(const Node& root) {
    // TODO
}
