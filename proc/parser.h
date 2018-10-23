#pragma once


#include <string.h>
#include <map>
#include "processor.h"



void shift(char*& ch) {
    while (*ch == ' ')
        ++ch;
}


void parse_push(char* args_buf, FILE* out, size_t line) {
    static char msg[1024];
    char* st = args_buf;
    shift(st);

    if (*st == 'r') {
        for (double k = 0; k < REGISTERS_NAMES.size(); ++k)
            if (!strcmp(st, REGISTERS_NAMES[k])) {
                fwrite(new double(1.0), sizeof(double), 1, out);
                fwrite(&k, sizeof(double), 1, out);
                break;
            }
            else if (k == REGISTERS_NAMES.size() - 1) {
                snprintf(msg, sizeof(msg), 
                    STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:" 
                    STYLE("0") " register \"%s\" not found\n", line, st);
                throw proc_exception(msg);
            }
    }
    else if (*st == '[') {
        size_t index = 0;
        ++st;
        
        if (*st == 'r') {
            for (unsigned char k = 0; k < REGISTERS_NAMES.size(); ++k) {
                if (!strncmp(st, REGISTERS_NAMES[k], 3)) {
                    index = k + 1;
                    st += 3;
                    break;
                }
                else if (k == REGISTERS_NAMES.size() - 1) {
                    snprintf(msg, sizeof(msg), 
                        STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:" 
                        STYLE("0") " register \"%s\" not found\n", line, st);
                    throw proc_exception(msg);
                }
            }

            if (*st == ']') {
                double double_index = index;
                fwrite(new double(2.0), sizeof(double), 1, out);
                fwrite(&double_index, sizeof(double), 1, out);
                return;
            }
            else if (*st != '+') {
                snprintf(msg, sizeof(msg), 
                    STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:" 
                    STYLE("0") " wrong argument\n", line);
                throw proc_exception(st);
            }
            ++st;
        }

        char* end = nullptr;
        unsigned long int arg = strtoul(st, &end, 10);

        if (*end != ']') {
            snprintf(msg, sizeof(msg), 
                STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:" 
                STYLE("0") " wrong argument\n", line);
            throw proc_exception(st);
        }

        index += arg * (__REGISTERS_NUMBER__ + 1);
        
        double double_index = index;
        fwrite(new double(2.0), sizeof(double), 1, out);
        fwrite(&double_index, sizeof(double), 1, out);
    }
    else {
        char* end = nullptr;
        double arg = strtod(st, &end);
        if ((end != args_buf + strlen(args_buf)) && *end != ' ') {
            snprintf(msg, sizeof(msg), 
                STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:" 
                STYLE("0") " wrong arument\n", line);
            throw proc_exception(msg);
        }

        fwrite(new double(0.0), sizeof(double), 1, out);
        fwrite(&arg, sizeof(double), 1, out);
    }
}


void parse_pop(char* args_buf, FILE* out, size_t line) {
    static char msg[1024];
    char* st = args_buf;
    shift(st);

    if (st == args_buf + strlen(args_buf)) {
        fwrite(new double(0.0), sizeof(double), 1, out);
        fwrite(new double(0.0), sizeof(double), 1, out);
    }
    else if (*st == 'r') {
        for (double k = 0; k < REGISTERS_NAMES.size(); ++k)
            if (!strcmp(st, REGISTERS_NAMES[k])) {
                fwrite(new double(1.0), sizeof(double), 1, out);
                fwrite(&k, sizeof(double), 1, out);
                break;
            }
            else if (k == REGISTERS_NAMES.size() - 1) {
                snprintf(msg, sizeof(msg), 
                    STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:" 
                    STYLE("0") " register \"%s\" not found\n", line, st);
                throw proc_exception(msg);
            }
    }
    else if (*st == '[') {
        size_t index = 0;

        ++st;
        
        if (*st == 'r') {
            for (unsigned char k = 0; k < REGISTERS_NAMES.size(); ++k) {
                if (!strncmp(st, REGISTERS_NAMES[k], 3)) {
                    index = k + 1;
                    st += 3;
                    break;
                }
                else if (k == REGISTERS_NAMES.size() - 1) {
                    snprintf(msg, sizeof(msg), 
                        STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:" 
                        STYLE("0") " register \"%s\" not found\n", line, st);
                    throw proc_exception(msg);
                }
            }

            if (*st == ']') {
                double double_index = index;
                fwrite(new double(2.0), sizeof(double), 1, out);
                fwrite(&double_index, sizeof(double), 1, out);
                return;
            }
            else if (*st != '+') {
                snprintf(msg, sizeof(msg), 
                    STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:" 
                    STYLE("0") " wrong argument\n", line);
                throw proc_exception(st);
            }
            ++st;
        }

        char* end = nullptr;
        unsigned long int arg = strtoul(st, &end, 10);

        if (*end != ']') {
            snprintf(msg, sizeof(msg), 
                STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:" 
                STYLE("0") " wrong argument\n", line);
            throw proc_exception(st);
        }

        index += arg * (__REGISTERS_NUMBER__ + 1);
        
        double double_index = index;
        fwrite(new double(2.0), sizeof(double), 1, out);
        fwrite(&double_index, sizeof(double), 1, out);
    }
}


void parse_jump(char* args_buf, FILE* out, size_t line, auto& labels) {
    static char msg[1024];
    char* st = args_buf;
    shift(st);

    if (*st != ':') {
        snprintf(msg, sizeof(msg), 
            STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:" 
            STYLE("0") " wrong arument\n", line);
        throw proc_exception(msg);
    }

    ++st;
    if (!labels[st]) {
        snprintf(msg, sizeof(msg), 
            STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:" 
            STYLE("0") " label %s not found\n", line, st);
        throw proc_exception(msg);
    }

    double tmp = labels[st] - 1 + strlen(SGN);
    fwrite(&tmp, sizeof(double), 1, out);
}


void parse(unsigned char command, char* args_buf, FILE* out, size_t line, auto& labels) {
    args_buf[strlen(args_buf) - 1] = 0;
    if (command == CMD_PUSH)
        parse_push(args_buf, out, line);
    else if (command == CMD_POP)
        parse_pop(args_buf, out, line);
    else if (command == CMD_JMP || 
             command == CMD_JA || command == CMD_JB || command == CMD_JNE ||
             command == CMD_JAE || command == CMD_JBE || command == CMD_JE)
        parse_jump(args_buf, out, line, labels);
    else {
        char* cur = args_buf;
        char* fin = args_buf + strlen(args_buf);
        
        for (size_t i = 0; i < ARGS_NUMBERS[command]; ++i) {
            shift(cur);

            char* end = nullptr;
            double arg = strtod(cur, &end);
            
            if (end != fin && *end != ' ') {
                char msg[1024];
                snprintf(msg, sizeof(msg), 
                    STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:" 
                    STYLE("0") " wrong arument (expected double)\n", line);
                throw proc_exception(msg);
            }
            
            fwrite(&arg, sizeof(double), 1, out);
            cur += sizeof(double);
        }
    }
}
