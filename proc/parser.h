#pragma once


#include <string.h>
#include <map>
#include "processor.h"



static inline void shift(char*& ch) {
    while (*ch == ' ')
        ++ch;
}

static inline void make_fin(char* buf) {
    char* comment = strchr(buf, '#');
    char* endl = strchr(buf, '\n');
    char* last = (comment ? comment : endl ? endl : buf + strlen(buf));
    *last = 0;
    --last;
    while (last != buf && *last == ' ') {
        *last = 0;
        --last;
    }
}


static void parse_push_pop(size_t command, char* args_buf, FILE* out, size_t line) {
    char* st = args_buf;
    shift(st);

    if (!*st) 
        if (command == CMD_POP) {
            fwrite(new double(0.0), sizeof(double), 1, out);
            fwrite(new double(0.0), sizeof(double), 1, out);
        }
        else
            throw asm_exception(line, "PUSH: no arguments (need 1)");
    else if (*st == 'r') {
        for (double k = 0; k < REGISTERS_NAMES.size(); ++k)
            if (!strcmp(st, REGISTERS_NAMES[k])) {
                fwrite(new double(1.0), sizeof(double), 1, out);
                fwrite(&k, sizeof(double), 1, out);
                break;
            }
            else if (k == REGISTERS_NAMES.size() - 1)
                throw asm_exception(line, get_string("register \"%s\" not found", st));
    }
    else if (*st == '[') {
        size_t index = 0;
        ++st;
        shift(st);
        
        if (*st == 'r') {
            for (unsigned char k = 0; k < REGISTERS_NAMES.size(); ++k)
                if (!strncmp(st, REGISTERS_NAMES[k], 3)) {
                    index = k + 1;
                    st += 3;
                    break;
                }
                else if (k == REGISTERS_NAMES.size() - 1)
                    throw asm_exception(line, get_string("register \"%s\" not found", st));

            shift(st);

            if (*st == ']') {
                double double_index = index;
                fwrite(new double(2.0), sizeof(double), 1, out);
                fwrite(&double_index, sizeof(double), 1, out);
                return;
            }
            else if (*st != '+')
                throw asm_exception(line, "wrong argument (access to RAM must be of form [rax+1])");
            ++st;
        }

        shift(st);
        char* end = nullptr;
        unsigned long int arg = strtoul(st, &end, 10);

        shift(end);
        if (*end != ']')
            throw asm_exception(line, "wrong argument (access to RAM must be of form [rax+1])");

        index += arg * (__REGISTERS_NUMBER__ + 1);
        
        double double_index = index;
        fwrite(new double(2.0), sizeof(double), 1, out);
        fwrite(&double_index, sizeof(double), 1, out);
    }
    else if (command == CMD_PUSH) {
        char* end = nullptr;
        double arg = strtod(st, &end);
        shift(end);
        if (*end)
            throw asm_exception(line, "wrong argument (expected double)");

        fwrite(new double(0.0), sizeof(double), 1, out);
        fwrite(&arg, sizeof(double), 1, out);
    }
    else
        throw asm_exception(line, "POP: wrong argument format");
}


static void parse_jump(char* args_buf, FILE* out, size_t line, auto& labels) {
    char* st = args_buf;
    shift(st);

    if (*st != ':')
        throw asm_exception(line, "wrong arument (label has to begin with \":\")");

    ++st;
    shift(st);
    if (!labels[st])
        throw asm_exception(line, get_string("label %s not found", st));

    double tmp = labels[st] - 1 + strlen(SGN);
    fwrite(&tmp, sizeof(double), 1, out);
}


static void parse_func(char* args_buf, FILE* out, auto& funcs) noexcept {
    char* st = args_buf;
    shift(st);

    double tmp = funcs[st].second - 1 + strlen(SGN);
    fwrite(&tmp, sizeof(double), 1, out);
}


static void parse_call(char* args_buf, FILE* out, size_t line, auto& funcs) {
    char* st = args_buf;
    shift(st);

    if (!funcs[st].first)
        throw asm_exception(line, get_string("function %s not found", st));

    double tmp = funcs[st].first - 1 + strlen(SGN);
    fwrite(&tmp, sizeof(double), 1, out);
}


static void parse(unsigned char command, char* args_buf, FILE* out, size_t line, auto& labels, auto& funcs) {
    make_fin(args_buf);

    if (command == CMD_PUSH || command == CMD_POP)
        parse_push_pop(command, args_buf, out, line);
    else if (command == CMD_JMP || 
             command == CMD_JA || command == CMD_JB || command == CMD_JNE ||
             command == CMD_JAE || command == CMD_JBE || command == CMD_JE)
        parse_jump(args_buf, out, line, labels);
    else if (command == CMD_FUNC)
        parse_func(args_buf, out, funcs);
    else if (command == CMD_CALL)
        parse_call(args_buf, out, line, funcs);
    else {
        char* cur = args_buf;

        for (size_t i = 0; i < ARGS_NUMBERS[command]; ++i) {
            shift(cur);

            char* end = nullptr;
            double arg = strtod(cur, &end);

            if (*end && *end != ' ')
                throw asm_exception(line, "wrong arument (expected double)");
            
            fwrite(&arg, sizeof(double), 1, out);
            cur = end;
        }
    }
}
