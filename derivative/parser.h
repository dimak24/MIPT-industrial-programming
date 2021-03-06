#pragma once

#include <exception>
#include <string>
#include <string_view>
#include <stdio.h>

#include "derivative.h"
#include "lexer.h"
#include "node.h"
#include "../proc/processor.h"
#include "../proc/parser.h"


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
    Number:                        N >> [0-9]+
    Identificator:                 ID >> [a-z,A-Z]+
    Value:                         VAL >> N|ID|ID\[_|N{,N}*\]

    CMPExpression:                 CE >> E{[><]E}*
    Expression:                    E >> S{[+-]S}*
    Summand:                       S >> M{[/ *]M}*
    Multiplier:                    M >> P|P^P
    Paranthesis:                   P >> (CE)|VAL|C

    Variable declaration:          VD >> ID:=CE
    Math function declaration:     F >> ID(_|ID{,ID}*):=E'

    Ordinary function declaration: DEF >> def ID(_|ID{,ID}*) as G end;
    Return:                        ret CE | leave 
    
    Operation:                     OP >> ID [+ - / * = +=] CE | C
    Call:                          C >> ID(_|{CE,}*CE)

    If:                            IF >> if CE then G end;
    While:                         W >> till the cows come home repeat G end;

    Instruction:                   I >> OP|IF|W|F|VD
    Grammar:                       G >> {I;}+\0

    Vector:                        V >> \[{CE|{CE,}*CE}\]

*/

class Parser {
private:
    Lexer& lexer_;
    std::map<std::string, Node*> funcs;

    Node getG_() {
        auto p0 = lexer_.mark();
        auto lexeme = lexer_.next_lexeme();
        if (lexeme.type == LT_EOF)
            throw parser_exception("empty program");

        lexer_.reset(p0);
        Node node;

        while (lexeme.type != LT_EOF && lexeme.type != LT_END) {
            node.adopt(getI_());
            if (IS_OP(*node.children.back()) &&
                OP(*node.children.back()) == OP_DECLARE &&
                IS_UNDEFINED(*node.children.back()->children[1]))
                    funcs[std::string(NAME(*node.children.back()->children[0]))] = node.children.back();

            p0 = lexer_.mark();
            lexeme = lexer_.next_lexeme();
            lexer_.reset(p0);
        }
        return node;
    }

    Node getC_() {
        auto p0 = lexer_.mark();
        auto lexeme = lexer_.next_lexeme();
        Node node;

        auto argnum = -1;

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
            node = Node({NODE_CALL, NAME(getID_())});

            auto name = std::string(NAME(node));

            if (false);
#define DEF_BUILTIN_FUNC(bf_name, arg_num, proc_command) \
            else if (!strcmp(name.c_str(), bf_name)) \
                argnum = arg_num;
#include "operators.h"
#undef DEF_BUILTIN_FUNC
            else if (!funcs.count(name))
                throw parser_exception("function not declared");
            else
                argnum = funcs[name]->children[1]->children.size();
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
                if (argnum != args)
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
                    return getDEF_();
                } catch (parser_exception&) {
                    try {
                        lexer_.reset(p0);
                        return getRET_();
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
        auto name_str = std::string(NAME(name));

        if (funcs.count(name_str))
            throw parser_exception("function redeclaration");

        if (lexer_.next_lexeme().type != LT_L_PARANTHESIS)
            throw parser_exception("( expected");

        std::vector<Node> args;

        auto p0 = lexer_.mark();
        while (lexer_.next_lexeme().type != LT_R_PARANTHESIS) {
            lexer_.reset(p0);
            args.push_back(getID_());

            p0 = lexer_.mark();
            if (lexer_.next_lexeme().type != LT_COMMA)
                lexer_.reset(p0);
            p0 = lexer_.mark();
        }

        if (lexer_.next_lexeme().type != LT_DECLARE)
            throw parser_exception(":= expected");

        auto expr = getCE_();

        // HARD MODE
        // if (lexer_.next_lexeme().type != LT_DERIVATIVE)
        //     throw parser_exception("\' expected");

        Node ans = expr;
        p0 = lexer_.mark();
        
        while (lexer_.next_lexeme().type == LT_DERIVATIVE) {
            p0 = lexer_.mark();
            if (lexer_.next_lexeme().type == LT_L_BRACE) {
                auto var = getID_();
                if (lexer_.next_lexeme().type != LT_R_BRACE)
                    throw parser_exception("} expected");
                ans = derivative(Node(ans), NAME(var));
            }
            else {
                lexer_.reset(p0);
                ans = derivative(Node(ans));
            }
            p0 = lexer_.mark();
        }
        lexer_.reset(p0);

        return MAKE_OP<OP_DECLARE>(name, Node({NODE_UNDEFINED}, args), ans);
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

        while (lexeme.type == LT_PLUS || lexeme.type == LT_MINUS || lexeme.type == LT_PM) {
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

        while (lexeme.type == LT_MUL || lexeme.type == LT_DIV || lexeme.type == LT_MD) {
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
        auto p0 = lexer_.mark();
        try {
            return getC_();
        }
        catch (parser_exception&) {
            lexer_.reset(p0);
            try {
                return getVAL_();
            } catch (parser_exception&) {
                lexer_.reset(p0);
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
            auto name = getID_();

            auto p0 = lexer_.mark();
            if (lexer_.next_lexeme().type == LT_L_BRACKET) {
                name.adopt(Node());

                p0 = lexer_.mark();
                while (lexer_.next_lexeme().type != LT_R_BRACKET) {
                    lexer_.reset(p0);
                    name.children[0]->adopt(getCE_());

                    p0 = lexer_.mark();
                    if (lexer_.next_lexeme().type != LT_COMMA)
                        lexer_.reset(p0);
                    p0 = lexer_.mark();
                }

                return name;
            }

            lexer_.reset(p0);
            return name;
        }
        catch (parser_exception&) {
            return getN_();
        }
    }

    Node getOP_() {
        auto p0 = lexer_.mark();
        try {
            return getC_();
        } catch (parser_exception&) {
            lexer_.reset(p0);
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

        p0 = lexer_.mark();

        // vector
        if (lexer_.next_lexeme().type == LT_L_BRACKET) {
            auto ans = MAKE_OP<OP_DECLARE>(node, Node());

            p0 = lexer_.mark();
            while (lexer_.next_lexeme().type != LT_R_BRACKET) {
                lexer_.reset(p0);
                ans.children[0]->adopt(getCE_());

                p0 = lexer_.mark();
                if (lexer_.next_lexeme().type != LT_COMMA)
                    lexer_.reset(p0);
                p0 = lexer_.mark();
            }

            return ans;
        }

        lexer_.reset(p0);
        return MAKE_OP<OP_DECLARE>(node, getCE_());
    }

    Node getRET_() {
        auto p0 = lexer_.mark();
        if (lexer_.next_lexeme().type == LT_LEAVE)
            return MAKE_OP<OP_LEAVE>();

        lexer_.reset(p0);
        if (lexer_.next_lexeme().type == LT_RET)
            return MAKE_OP<OP_RET>(getCE_());

        lexer_.reset(p0);
        throw parser_exception("ret or leave expected");
    }

    Node getDEF_() {
        auto p0 = lexer_.mark();
        if (lexer_.next_lexeme().type != LT_DEF) {
            lexer_.reset(p0);
            throw parser_exception("def expected");
        }

        auto name = getID_();
        auto name_str = std::string(NAME(name));

        if (funcs.count(name_str))
            throw parser_exception("function redeclaration");

        if (lexer_.next_lexeme().type != LT_L_PARANTHESIS)
            throw parser_exception("( expected");

        std::vector<Node> args;

        p0 = lexer_.mark();
        while (lexer_.next_lexeme().type != LT_R_PARANTHESIS) {
            lexer_.reset(p0);
            args.push_back(getID_());

            p0 = lexer_.mark();
            if (lexer_.next_lexeme().type != LT_COMMA)
                lexer_.reset(p0);
            p0 = lexer_.mark();
        }

        if (lexer_.next_lexeme().type != LT_AS)
            throw parser_exception("as expected");

        auto body = getG_();

        if (lexer_.next_lexeme().type != LT_END)
            throw parser_exception("end expected");

        return MAKE_OP<OP_DECLARE>(name, Node({NODE_UNDEFINED}, args), body);
    }

public:
    Parser() = delete;
    ~Parser() {}

    Parser(Lexer& lexer)
        : lexer_(lexer) {}

    Parser(const Parser&) = delete;
    Parser(Parser&&) = delete;


    Node parse() {
        return getG_();
    }
};


std::vector<std::map<std::string, int>> locals;
std::vector<size_t> cur_nargs;
std::map<std::string, int> call_start;


void generate_asm_CALL(Node*, FILE*);
void generate_block(Node*, FILE*, int call_from = 0);
void generate_asm_EXPR(Node*, FILE*);
void generate_asm_OP(Node*, FILE*);
void generate_asm_VD(Node*, FILE*);
void generate_asm_FD(Node*, FILE*);
void generate_asm_MFD(Node*, FILE*);


void generate_asm_EXPR(Node* expr, FILE* memstream) {
    if (IS_OP(*expr))
        switch (OP(*expr)) {
#define DEF_MATH_OP(name, type, mnemonic, latex_mnemonic, arg_num, proc_command) \
            case OP_##name: { \
                for (auto i = 0; i < arg_num; ++i) \
                    if (IS_VAR(*expr->children[i])) { \
                        fwrite(get_address(CMD_GET_LOCAL), 1, 1, memstream); \
                        fwrite(get_address<double>(locals.back()[std::string(NAME(*expr->children[i]))]), \
                               sizeof(double), 1, memstream); \
                    } \
                    else if (IS_CONST(*expr->children[i])) { \
                        fwrite(get_address(CMD_PUSH), 1, 1, memstream); \
                        fwrite(get_address<double>(0), sizeof(double), 1, memstream); \
                        fwrite(&VALUE(*expr->children[i]), sizeof(double), 1, memstream); \
                    } \
                    else if (IS_CALL(*expr->children[i])) \
                        generate_asm_CALL(expr->children[i], memstream); \
                    else \
                        generate_asm_EXPR(expr->children[i], memstream); \
                fwrite(get_address(CMD_##proc_command), 1, 1, memstream); \
                break; \
            }
#include "operators.h"
#undef DEF_MATH_OP
            default: __builtin_unreachable();
        }
    else if (IS_CALL(*expr))
        generate_asm_CALL(expr, memstream);
    else if (IS_CONST(*expr)) {
        fwrite(get_address(CMD_PUSH), 1, 1, memstream);
        fwrite(get_address<double>(0), sizeof(double), 1, memstream);
        fwrite(&VALUE(*expr), sizeof(double), 1, memstream);
    }
    else
        throw parser_exception("wrong operator");
}

void generate_asm_OP(Node* op, FILE* memstream) {
    switch (OP(*op)) {
#define DEF_ASSIGN_OP(name, mnemonic, proc_command) \
            case OP_##name: { \
                std::string name1 = NAME(*op->children[0]); \
                if (!locals.back().count(name1)) \
                    throw parser_exception("variable has not been declared yet"); \
                \
                fwrite(get_address(CMD_GET_LOCAL), 1, 1, memstream); \
                fwrite(get_address<double>(locals.back()[name1]), sizeof(double), 1, memstream); \
                \
                generate_asm_EXPR(op->children[1], memstream); \
                \
                fwrite(get_address(CMD_##proc_command), 1, 1, memstream); \
                \
                fwrite(get_address(CMD_SET_LOCAL), 1, 1, memstream); \
                fwrite(get_address<double>(locals.back()[name1]), sizeof(double), 1, memstream); \
                break; \
            }
#include "operators.h"
#undef DEF_ASSIGN_OP
            case OP_IF: {
                auto cond = op->children[0];
                generate_asm_EXPR(cond, memstream);

                fwrite(get_address(CMD_PUSH), 1, 1, memstream);
                fwrite(get_address<double>(0), sizeof(double), 1, memstream);
                fwrite(get_address<double>(0), sizeof(double), 1, memstream);
                fwrite(get_address(CMD_JE), 1, 1, memstream);
            
                auto label_place = ftell(memstream);
                fwrite(get_address<double>(0), sizeof(double), 1, memstream);
                
                generate_block(op->children[1], memstream);

                auto last = ftell(memstream);

                fseek(memstream, label_place, SEEK_SET);
                fwrite(get_address<double>(last + strlen(SGN)), sizeof(double), 1, memstream);

                fseek(memstream, last, SEEK_SET);
                break;
            }
            case OP_WHILE: {
                auto start = ftell(memstream);

                generate_block(op->children[0], memstream);

                fwrite(get_address(CMD_JMP), 1, 1, memstream);
                fwrite(get_address<double>(start + strlen(SGN)), sizeof(double), 1, memstream);
                break;
            }

            default: __builtin_unreachable();
        }
}

void generate_asm_CALL(Node* call, FILE* memstream) {
    std::string name = NAME(*call);
    // reverse!!
    for (auto it_child = call->children.rbegin(); it_child != call->children.rend(); ++it_child) {
        auto child = *it_child;
        if (IS_VAR(*child)) {
            fwrite(get_address(CMD_GET_LOCAL), 1, 1, memstream);
            fwrite(get_address<double>(locals.back()[std::string(NAME(*child))]), sizeof(double), 1, memstream);
        }
        else if (IS_CONST(*child)) {
            fwrite(get_address(CMD_PUSH), 1, 1, memstream);
            fwrite(get_address<double>(0), sizeof(double), 1, memstream);
            fwrite(get_address<double>(VALUE(*child)), sizeof(double), 1, memstream);
        }
        else if (IS_CALL(*child))
            generate_asm_CALL(child, memstream);
        else
            generate_asm_EXPR(child, memstream);
    }

    if (false);
#define DEF_BUILTIN_FUNC(mnemonic, nargs, proc_command) \
    else if (name == mnemonic) \
        fwrite(get_address(CMD_##proc_command), 1, 1, memstream);
#include "operators.h"
#undef DEF_BUILTIN_FUNC
    else if (!locals.back().count(name))
        throw parser_exception("function has not been declared yet");
    else {
        fwrite(get_address(CMD_CALL), 1, 1, memstream);
        fwrite(get_address<double>(call_start[name]), sizeof(double), 1, memstream);
    }
}

void generate_asm_MFD(Node* fd, FILE* memstream) {
    std::string name = NAME(*fd->children[0]);
    if (locals.back().count(name))
        throw parser_exception("function has already been declared");
    locals.back()[name] = locals.back().size() - cur_nargs.back();

    fwrite(get_address(CMD_FD), 1, 1, memstream);
    auto nskip_offset = ftell(memstream);

    fwrite(get_address<double>(0), sizeof(double), 1, memstream);
    call_start[name] = ftell(memstream) + strlen(SGN);
    fwrite(get_address<double>(0), sizeof(double), 1, memstream);
    fwrite(get_address<double>(0), sizeof(double), 1, memstream);

    locals.emplace_back();
    
    auto n = fd->children[1]->children.size();
    cur_nargs.push_back(n);
    for (auto i = 0u; i < n; ++i) 
        locals.back()[NAME(*fd->children[1]->children[i])] = -i-1;
    
    generate_asm_EXPR(fd->children[2], memstream);
    fwrite(get_address(CMD_RET), 1, 1, memstream);
    fwrite(get_address<double>(call_start[name]), sizeof(double), 1, memstream);

    auto from = ftell(memstream);

    fseek(memstream, nskip_offset, SEEK_SET);
    fwrite(get_address<double>(from + strlen(SGN)), sizeof(double), 1, memstream);
    fwrite(get_address<double>(n), sizeof(double), 1, memstream);
    fwrite(get_address<double>(locals.back().size() - n), sizeof(double), 1, memstream);
    fseek(memstream, from, SEEK_SET);

    cur_nargs.pop_back();
    locals.pop_back();  
}

void generate_asm_FD(Node* fd, FILE* memstream) {
    std::string name = NAME(*fd->children[0]);
    if (locals.back().count(name))
        throw parser_exception("function has already been declared");
    locals.back()[name] = locals.back().size() - cur_nargs.back();;

    fwrite(get_address(CMD_FD), 1, 1, memstream);
    auto nskip_offset = ftell(memstream);

    // endfunc
    fwrite(get_address<double>(0), sizeof(double), 1, memstream);
    call_start[name] = ftell(memstream) + strlen(SGN);
    
    // nargs
    fwrite(get_address<double>(0), sizeof(double), 1, memstream);
    // nlocals
    fwrite(get_address<double>(0), sizeof(double), 1, memstream);

    locals.emplace_back();
    
    auto n = fd->children[1]->children.size();
    cur_nargs.push_back(n);
    for (auto i = 0u; i < n; ++i)
        locals.back()[NAME(*fd->children[1]->children[i])] = -i-1;
    
    generate_block(fd->children[2], memstream);
    fwrite(get_address(CMD_LEAVE), 1, 1, memstream);
    fwrite(get_address<double>(call_start[name]), sizeof(double), 1, memstream);

    auto from = ftell(memstream);

    fseek(memstream, nskip_offset, SEEK_SET);
    fwrite(get_address<double>(from + strlen(SGN)), sizeof(double), 1, memstream);
    fwrite(get_address<double>(n), sizeof(double), 1, memstream);
    fwrite(get_address<double>(locals.back().size() - n), sizeof(double), 1, memstream);
    fseek(memstream, from, SEEK_SET);

    cur_nargs.pop_back();
    locals.pop_back();
}


void generate_asm_VD(Node* vd, FILE* memstream) {
    std::string name = NAME(*vd->children[0]);
    if (locals.back().find(name) != locals.back().end())
        throw parser_exception("variable has already been declared");
    locals.back()[name] = locals.back().size() - cur_nargs.back();

    generate_asm_EXPR(vd->children[1], memstream);
    
    fwrite(get_address(CMD_SET_LOCAL), 1, 1, memstream);
    fwrite(get_address<double>(locals.back()[name]), sizeof(double), 1, memstream);
}


void generate_block(Node* root, FILE* memstream, int call_from) {
    for (auto child : root->children)
        if (IS_OP(*child)) {
            if (OP(*child) == OP_DECLARE) {
                if (IS_UNDEFINED(*child->children[1])) {
                    if (IS_UNDEFINED(*child->children[2]))
                        generate_asm_FD(child, memstream);
                    else
                        generate_asm_MFD(child, memstream);
                }
                else
                    generate_asm_VD(child, memstream);
            }
            else if (OP(*child) == OP_LEAVE) {
                fwrite(get_address(CMD_LEAVE), 1, 1, memstream);
                fwrite(get_address<double>(call_from + strlen(SGN)), sizeof(double), 1, memstream);
            }
            else if (OP(*child) == OP_RET) {
                generate_asm_EXPR(child->children[0], memstream);
                fwrite(get_address(CMD_RET), 1, 1, memstream);
                fwrite(get_address<double>(call_from + strlen(SGN)), sizeof(double), 1, memstream);
            }
            else
                generate_asm_OP(child, memstream);
        }
        else if (IS_CALL(*child))
            generate_asm_CALL(child, memstream);
        else
            throw parser_exception("WTF");
}


void generate_asm(Node* root) {
    char* buf = nullptr;
    size_t nbuf = 0;
    FILE* memstream = open_memstream(&buf, &nbuf);

    auto begin = ftell(memstream);

    FILE* out = fopen("a.dk", "w");
    if (!out) {
        perror("a.dk");
        exit(1);
    }
    fprintf(out, SGN);

    locals.emplace_back();
    cur_nargs.push_back(0);

    // fd
    fwrite(get_address(CMD_FD), 1, 1, memstream);
    auto offset = ftell(memstream);

    // endfunc
    fwrite(get_address<double>(0), sizeof(double), 1, memstream);
    // nargs
    fwrite(get_address<double>(0), sizeof(double), 1, memstream);
    // nlocals
    fwrite(get_address<double>(0), sizeof(double), 1, memstream);

    generate_block(root, memstream);
    fwrite(get_address(CMD_LEAVE), 1, 1, memstream);
    fwrite(get_address<double>(strlen(SGN) + 1 + sizeof(double)), sizeof(double), 1, memstream);

    auto from = ftell(memstream);

    // update
    fseek(memstream, offset, SEEK_SET);
    fwrite(get_address<double>(from - begin + strlen(SGN)), sizeof(double), 1, memstream);
    fwrite(get_address<double>(0), sizeof(double), 1, memstream);
    fwrite(get_address<double>(locals.back().size()), sizeof(double), 1, memstream);

    fseek(memstream, from, SEEK_SET);

    fwrite(get_address(CMD_CALL), 1, 1, memstream);
    fwrite(get_address<double>(strlen(SGN) + 1 + sizeof(double)), sizeof(double), 1, memstream);

    fclose(memstream);
 
    fwrite(buf, nbuf, 1, out);

    fclose(out);
    free(buf);
}
