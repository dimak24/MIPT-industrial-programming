#pragma once

#include <exception>
#include <string>
#include <string_view>
#include <stdio.h>

#include "derivative.h"
#include "lexer.h"
#include "node.h"
#include "../proc/processor.h"


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

    Variable declaration:      VD >> ID:=CE
    
    Operation:                 OP >> ID [BINARY OP] CE | C
    Call:                      C >> {ID|[*+]}({CE,}*CE)

    If:                        IF >> if CE then G end;
    While:                     W >> till the cows come home repeat G end;

    Instruction:               I >> OP|IF|W|F|VD
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
#define DEF_MATH_OP(name, _type, mnemonic, latex_command, arg_num, proc_command) \
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
                    try {
                        lexer_.reset(p0);
                        return getC_();
                    } catch (parser_exception&) {
                        try {
                            lexer_.reset(p0);
                            return getVD_();
                        } catch (parser_exception&) {
                            lexer_.reset(p0);
                            return getOP_();
                        }
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
#define DEF_MATH_OP(name, type, mnemonic, latex_command, arg_num, proc_command) DEF_OP(name)
// #define DEF_ASSIGN_OP(name, mnemonic, proc_command) DEF_OP(name)
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
#define DEF_MATH_OP(name, type, mnemonic, latex_command, arg_num, proc_command) DEF_OP(name)
#define DEF_ASSIGN_OP(name, mnemonic, proc_command) DEF_OP(name)
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

    Node getVD_() {
        auto p0 = lexer_.mark();
        auto node = getID_();

        if (lexer_.next_lexeme().type != LT_DECLARE) {
            lexer_.reset(p0);
            throw parser_exception(":= expected");
        }
        return MAKE_OP<OP_DECLARE>(node, getCE_());
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


std::vector<std::map<std::string, int>> locals;


void generate_asm_CALL(Node*, FILE*);
int generate_block(Node*, FILE*);
void generate_asm_EXPR(Node*, FILE*);
void generate_asm_OP(Node*, FILE*);


void generate_asm_EXPR(Node* expr, FILE* memstream) {
    if (IS_OP(*expr))
        switch (OP(*expr)) {
#define DEF_MATH_OP(name, type, mnemonic, latex_mnemonic, arg_num, proc_command) \
            case OP_##name: { \
                for (auto i = 0; i < arg_num; ++i) \
                    if (IS_VAR(*expr->children[i])) { \
                        fwrite(new unsigned char(CMD_GET_LOCAL), 1, 1, memstream); \
                        fwrite(new double(locals.back()[std::string(NAME(*expr->children[i]))]), \
                               1, sizeof(double), memstream); \
                    } \
                    else if (IS_CONST(*expr->children[i])) { \
                        fwrite(new unsigned char(CMD_PUSH), 1, 1, memstream); \
                        fwrite(&VALUE(*expr->children[i]), 1, sizeof(double), memstream); \
                    } \
                    else if (IS_CALL(*expr->children[i])) \
                        generate_asm_CALL(expr->children[i], memstream); \
                    else \
                        generate_asm_OP(expr->children[i], memstream); \
                fwrite(new unsigned char(CMD_##proc_command), 1, 1, memstream); \
                break; \
            }
#include "operators.h"
#undef DEF_MATH_OP
            default: __builtin_unreachable();
        }
    else if (IS_CALL(*expr))
        generate_asm_CALL(expr, memstream);
    else if (IS_CONST(*expr))
        fwrite(&VALUE(*expr), 1, sizeof(double), memstream);
    else
        throw parser_exception("wrong operator");
}

void generate_asm_OP(Node* op, FILE* memstream) {
    std::string name = NAME(*op->children[0]);
    if (locals.back().find(name) == locals.back().end())
        throw parser_exception("variable has not been declared yet");

    fwrite(new unsigned char(CMD_GET_LOCAL), 1, 1, memstream);
    fwrite(new double(locals.back()[name]), 1, sizeof(double), memstream);

    generate_asm_EXPR(op->children[1], memstream);

    switch (OP(*op)) {
#define DEF_ASSIGN_OP(name, mnemonic, proc_command) \
            case OP_##name: \
                fwrite(new unsigned char(CMD_##proc_command), 1, 1, memstream); \
                break;
#include "operators.h"
#undef DEF_ASSIGN_OP
            default: __builtin_unreachable();
        }

    fwrite(new unsigned char(CMD_SET_LOCAL), 1, 1, memstream);
    fwrite(new double(locals.back()[name]), 1, sizeof(double), memstream);
}

void generate_asm_CALL(Node* call, FILE* memstream) {
    std::string name = NAME(*call->children[0]);
    if (locals.back().find(name) != locals.back().end())
        throw parser_exception("function has already been declared");
    for (auto child : call->children)
        if (IS_VAR(*child)) {
            fwrite(new unsigned char(CMD_GET_LOCAL), 1, 1, memstream);
            fwrite(new double(locals.back()[std::string(NAME(*child))]), 1, sizeof(double), memstream);
        }
        else if (IS_CONST(*child)) {
            fwrite(new unsigned char(CMD_PUSH), 1, 1, memstream);
            fwrite(new double(VALUE(*child)), 1, sizeof(double), memstream);
        }
        else if (IS_CALL(*child))
            generate_asm_CALL(child, memstream);
        else
            generate_asm_EXPR(child, memstream);
    
    fwrite(new unsigned char(CMD_CALL), 1, 1, memstream);
    fwrite(name.c_str(), name.size() + 1, 1, memstream);
}

void generate_asm_FD(Node* fd, FILE* memstream) {
    std::string name = NAME(*fd->children[0]);
    if (locals.back().find(name) != locals.back().end())
        throw parser_exception("function has already been declared");
    locals.back()[name] = locals.back().size();
    // fd name fd->children[1]->children.size() nlocals nskip
    locals.emplace_back();
    for (auto i = 0; i < fd->children[1]->children.size(); ++i)
        locals.back()[NAME(*fd->children[1]->children[i])] = -i - 1;
    auto nlocals = generate_block(fd, memstream);
    // update nlocals
    // calculate and update nskip
}

void generate_asm_VD(Node* vd, FILE* memstream) {
    std::string name = NAME(*vd->children[0]);
    if (locals.back().find(name) != locals.back().end())
        throw parser_exception("variable has already been declared");
    locals.back()[name] = locals.back().size();

    generate_asm_EXPR(vd->children[1], memstream);
    
    fwrite(new unsigned char(CMD_SET_LOCAL), 1, 1, memstream);
    fwrite(new double(locals.back()[name]), 1, sizeof(double), memstream);
}

int generate_block(Node* root, FILE* memstream) {
    for (auto child : root->children)
        if (IS_OP(*child)) {
            if (OP(*child) == OP_DECLARE) {
                if (IS_UNDEFINED(*child->children[1]))
                    generate_asm_FD(child, memstream);
                else
                    generate_asm_VD(child, memstream);
            }
            else
                generate_asm_OP(child, memstream);
        }
        else if (IS_CALL(*child))
            generate_asm_CALL(child, memstream);
        else
            throw parser_exception("WTF");
    return locals.back().size();
}

void generate_asm(Node* root) {
    char* buf = nullptr;
    size_t nbuf = 0;
    FILE* memstream = open_memstream(&buf, &nbuf);

    locals.emplace_back();
    
    auto name = "___main_dk_";
    fwrite(new unsigned char(CMD_FD), 1, 1, memstream);
    fwrite(name, 1, strlen(name) + 1, memstream);
    fwrite(new double(0), 1, sizeof(double), memstream);

    auto offset = ftell(memstream);

    fwrite(new double(0), 1, sizeof(double), memstream);
    fwrite(new double(0), 1, sizeof(double), memstream);

    auto nlocals = generate_block(root, memstream);

    auto nskip = ftell(memstream) - offset - 2 * sizeof(double);

    fseek(memstream, offset, SEEK_SET);

    fwrite(&nlocals, 1, sizeof(double), memstream);
    fwrite(&nskip, 1, sizeof(double), memstream);

    fseek(memstream, nskip, SEEK_CUR);

    fwrite(new unsigned char(CMD_CALL), 1, 1, memstream);
    fwrite(&name, 1, strlen(name) + 1, memstream);

    fclose(memstream);

    FILE* out = fopen("a.dk", "w");
    if (!out) {
        perror("a.dk");
        exit(1);
    }

    fwrite(buf, nbuf, 1, out);

    fclose(out);
    free(buf);
}
