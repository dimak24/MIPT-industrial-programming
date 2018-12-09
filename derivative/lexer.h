#pragma once


#include <string_view>
#include <exception>
#include <string>
#include <stdlib.h>
#include <string.h>

#include "trie.h"


enum LexemeType {
    __LT_UNDEFINED__,

    LT_NUMBER,
    LT_IDENTIFICATOR,

    LT_COMMA,

    LT_DERIVATIVE,

    LT_R_PARANTHESIS,
    LT_L_PARANTHESIS,
    LT_L_BRACKET,
    LT_R_BRACKET,

#define DEF_OP(name, mnemonic) LT_##name,
#define DEF_MATH_OP(name, type, mnemonic, latex_command, arg_num, proc_command) DEF_OP(name, mnemonic)
#define DEF_ASSIGN_OP(name, mnemonic, proc_command) DEF_OP(name, mnemonic)
#define DEF_KEYWORD(name, mnemonic) DEF_OP(name, mnemonic)
#include "operators.h"
#undef DEF_KEYWORD
#undef DEF_ASSIGN_OP
#undef DEF_MATH_OP
#undef DEF_OP

    LT_EOF
};


struct Lexeme {
    LexemeType type;
    std::string_view data = {};
};


class lexer_exception : std::exception {
private:
    const char* msg_;

public:
    lexer_exception(const char* msg)
        : msg_(msg) {}

    virtual const char* what() noexcept {
        return msg_;
    }
};


class Lexer {
private:
    const char* start_;
    const char* end_;
    const char* p_;

    Trie trie;

public:
    Lexer(std::string_view prog)
        : start_(prog.data()),
          end_(prog.data() + prog.size()),
          p_(prog.data()) {
#define DEF_OP(name, mnemonic) trie.append(mnemonic, LT_##name);
#define DEF_MATH_OP(name, type, mnemonic, latex_command, arg_num, proc_command) DEF_OP(name, mnemonic)
#define DEF_ASSIGN_OP(name, mnemonic, proc_command) DEF_OP(name, mnemonic)
#define DEF_KEYWORD(name, mnemonic) DEF_OP(name, mnemonic)
#include "operators.h"
#undef DEF_KEYWORD
#undef DEF_ASSIGN_OP
#undef DEF_MATH_OP
#undef DEF_OP
          }


    ~Lexer() {}


    const char* mark() const {
        return p_;
    }

    void reset(const char* p) {
        p_ = p;
    }

    Lexeme next_lexeme() {
        while (p_ != end_ && *p_ == ' ')
            ++p_;

        if (p_ == end_)
            return {LT_EOF};
        
        switch (*p_) {
            case ',':
                ++p_;
                return {LT_COMMA};
            case '(':
                ++p_;
                return {LT_L_PARANTHESIS};
            case ')':
                ++p_;
                return {LT_R_PARANTHESIS};
            case '[':
                return {LT_L_BRACKET};
            case ']':
                return {LT_R_BRACKET};
            case '\'':
                ++p_;
                return {LT_DERIVATIVE};
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                bool point = false;
                const char* p0 = p_;
                while (p_ != end_ && (('0' <= *p_ && *p_ <= '9') || *p_ == '.')) {
                    if (*p_ == '.') {
                        if (point)
                            throw lexer_exception("2 points in number");
                        point = true;
                    }
                    ++p_;
                }
                return {LT_NUMBER, {p0, size_t(p_ - p0)}};
            }
            default: {
                int ans = 0, len = 0;
                std::tie(ans, len) = trie.find_longest_prefix(p_, end_);
                if (ans != __LT_UNDEFINED__) {
                    p_ += len;
                    return {(LexemeType)ans};
                }
                
                auto p0 = p_;
                while (p_ != end_ && (('a' <= *p_ && *p_ <= 'z') ||
                                      ('A' <= *p_ && *p_ <= 'Z')))
                    ++p_;
                
                if (p0 == p_)
                    throw lexer_exception("wrong operator");


                return {LT_IDENTIFICATOR, {p0, (size_t)(p_ - p0)}};
            }
        }
    }
};
