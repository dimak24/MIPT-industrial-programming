#pragma once

#include <string_view>
#include <exception>
#include <string>
#include <stdlib.h>
#include <string.h>


enum LexemeType {
    LT_NUMBER,
    LT_IDENTIFICATOR,

    LT_KEYWORD,
    LT_THEN,
    LT_END,

    LT_DERIVATIVE,

    LT_IF,
    LT_WHILE,

    LT_R_PARANTHESIS,
    LT_L_PARANTHESIS,

    LT_PLUS,
    LT_MINUS,
    LT_MUL,
    LT_DIV,
    LT_POW,

    LT_ASSIGN,
    LT_PLUS_ASSIGN,
    LT_MINUS_ASSIGN,
    LT_MUL_ASSIGN,
    LT_DIV_ASSIGN,
    LT_POW_ASSIGN,

    LT_EQ,
    LT_NE,
    LT_LT,
    LT_LE,
    LT_GT,
    LT_GE,

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


public:
    Lexer(std::string_view prog)
        : start_(prog.data()),
          end_(prog.data() + prog.size()),
          p_(prog.data()) {}

    ~Lexer() {}
    Lexer() {}


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
            case '=':
                ++p_;
                if (p_ != end_ && *p_ == '=') {
                    ++p_;
                    return {LT_EQ};
                }
                return {LT_ASSIGN};
            case '+':
                ++p_;
                if (p_ != end_ && *p_ == '=') {
                    ++p_;
                    return {LT_PLUS_ASSIGN};    
                }
                return {LT_PLUS};
            case '-':
                ++p_;
                if (p_ != end_ && *p_ == '=') {
                    ++p_;
                    return {LT_MINUS_ASSIGN};
                }
                return {LT_MINUS};
            case '*':
                ++p_;
                if (p_ != end_ && *p_ == '=') {
                    ++p_;
                    return {LT_MUL_ASSIGN};
                }
                return {LT_MUL};
            case '/':
                ++p_;
                if (p_ != end_ && *p_ == '=') {
                    ++p_;
                    return {LT_DIV_ASSIGN};
                }
                return {LT_DIV};
            case '^':
                ++p_;
                if (p_ != end_ && *p_ == '=') {
                    ++p_;
                    return {LT_POW_ASSIGN};
                }
                return {LT_POW};
            case '>':
                ++p_;
                if (p_ != end_ && *p_ == '=') {
                    ++p_;
                    return {LT_GE};
                }
                return {LT_GT};
            case '<':
                ++p_;
                if (p_ != end_ && *p_ == '=') {
                    ++p_;
                    return {LT_LE};
                }
                return {LT_LT};
            case '!':
                ++p_;
                if (p_ != end_ && *p_ == '=') {
                    ++p_;
                    return {LT_NE};
                }
                throw lexer_exception("after '!' should be '='");
            case '(':
                ++p_;
                return {LT_L_PARANTHESIS};
            case ')':
                ++p_;
                return {LT_R_PARANTHESIS};
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
                if (end_ - p_ >= 23 && !memcmp(p_, "till the cows come home", 23)) {
                    p_ += 23;
                    return {LT_WHILE};
                }
                if (end_ - p_ >= 2 && !memcmp(p_, "if", 2)) {
                    p_ += 2;
                    return {LT_IF};
                }

                std::string name;
                while (p_ != end_ && (('a' <= *p_ && *p_ <= 'z') ||
                                      ('A' <= *p_ && *p_ <= 'Z'))) {
                    name += *p_;
                    ++p_;
                }
                return {LT_IDENTIFICATOR, {name.c_str(), name.size()}};
            }
        }
    }
};
