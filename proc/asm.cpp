
#include "processor.h"
#include "reader.h"
#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <exception>
#include <string.h>
#include <map>
#include <string>


void get_labels(FILE* file, auto& labels) {
    static char msg[1024];
    labels.clear();

    char* buf = nullptr;
    size_t nbuf = 0;
    int ip_shift = 1;
    size_t line = 0;

    while (getline(&buf, &nbuf, file) > 0) {
        make_fin(buf);
        shift(buf);
        ++line;

        if (buf[0] == ':') {
            ++buf;
            if (labels[buf]) {
                snprintf(msg, sizeof(msg), 
                    STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:"
                    STYLE("0") " label \"%s\" redefinition\n", line, buf);
                throw proc_exception(msg);
            }
            labels[buf] = ip_shift;
        }
        else {
            char* ch = buf;
            while (*ch && *ch != ' ')
                ++ch;
            *ch = 0;
            for (unsigned char i = 0; i < COMMANDS_NAMES.size(); ++i)
                if (!strcmp(buf, COMMANDS_NAMES[i])) {
                    ip_shift += sizeof(unsigned char) + sizeof(double) * ARGS_NUMBERS[i];
                    break;
                }
        }
    }
}

void get_funcs(FILE* file, auto& funcs) {
    static char msg[1024];
    funcs.clear();

    char* buf = nullptr;
    size_t nbuf = 0;
    int ip_shift = 1;
    std::string cur_func_name("");
    size_t balance = 0;
    size_t line = 0;

    while (getline(&buf, &nbuf, file) > 0) {
        ++line;
        make_fin(buf);
        shift(buf);

        char* ch = buf;
        while (*ch && *ch != ' ')
            ++ch;
        *ch = 0;
        
        if (!strcmp(buf, "FUNC")) {
            buf = ch + 1;
            shift(buf);
            if (!*buf) {
                snprintf(msg, sizeof(msg), 
                    STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:"
                    STYLE("0") " function name not found\n", line);
                throw proc_exception(msg);
            }
            if (funcs[buf].first) {
                snprintf(msg, sizeof(msg), 
                    STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:"
                    STYLE("0") " function \"%s\" redefinition\n", line, buf);
                throw proc_exception(msg);
            }
            ip_shift += sizeof(unsigned char) + sizeof(double) * ARGS_NUMBERS[CMD_FUNC];
            funcs[buf].first = ip_shift;
            cur_func_name = buf;
            ++balance;
        }
        else if (!strcmp(buf, "ENDFUNC")) {
            if (!balance) {
                snprintf(msg, sizeof(msg), 
                    STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:"
                    STYLE("0") " no one function could be finished here\n", line);
                throw proc_exception(msg);
            }
            ip_shift += sizeof(unsigned char) + sizeof(double) * ARGS_NUMBERS[CMD_ENDFUNC];
            funcs[cur_func_name].second = ip_shift;
            --balance;
        }
        else
            for (unsigned char i = 0; i < COMMANDS_NAMES.size(); ++i)
                if (!strcmp(buf, COMMANDS_NAMES[i])) {
                    ip_shift += sizeof(unsigned char) + sizeof(double) * ARGS_NUMBERS[i];
                    break;
            }
    }

    if (balance) {
        snprintf(msg, sizeof(msg), 
            STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:"
            STYLE("0") " some functions are not finished\n", ++line);
        throw proc_exception(msg);
    }
}



int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, STYLE("1") "asm: " STYLE("31") "error: " STYLE("0") "no input files\n");
        exit(1);
    }

    for (int f = 1; f < argc; ++f) {

        char filename[1024];
        snprintf(filename, sizeof(filename), "%s.dk", argv[f]);

        FILE* raw = fopen(argv[f], "r");
        if (!raw)
            PANIC();

        FILE* compiled = fopen(filename, "w");
        if (!compiled)
            PANIC();

        std::map<std::string, uintptr_t> labels;
        std::map<std::string, std::pair<uintptr_t, uintptr_t>> funcs;

        try {
            get_labels(raw, labels);
            fseek(raw, 0, SEEK_SET);

            get_funcs(raw, funcs);
            fseek(raw, 0, SEEK_SET);
        } catch (proc_exception& e) {
            fprintf(stderr, "%s", e.what());
            remove(filename);
            exit(1);   
        }

        fprintf(compiled, SGN);

        char* buf = nullptr;
        size_t nbuf = 0;
        size_t line = 0;

        try {
            while (getline(&buf, &nbuf, raw) > 0) {
                ++line;
                shift(buf);
                if (buf[0] == ':')
                    continue;
                char* ch = buf;
                char* fin = buf + strlen(buf) - 1;
                while (ch != fin && *ch != ' ')
                    ++ch;
                *ch = 0;
                for (unsigned char i = 0; i < COMMANDS_NAMES.size(); ++i)
                    if (!strcmp(buf, COMMANDS_NAMES[i])) {
                        fwrite(&i, sizeof(unsigned char), 1, compiled);
                        parse(i, ch + 1, compiled, line, labels, funcs);
                        break;
                    }
                    else if (i == COMMANDS_NAMES.size() - 1) {
                        char msg[1024];
                        snprintf(msg, sizeof(msg), 
                            STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:"
                            STYLE("0") " command \"%s\" not found\n", line, buf);
                        throw proc_exception(msg);
                    }
            }
        } catch (proc_exception& e) {
            fprintf(stderr, "%s", e.what());
            remove(filename);
            exit(1);
        } 
    }
}
