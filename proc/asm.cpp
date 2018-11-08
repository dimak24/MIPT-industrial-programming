
#include "processor.h"
#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <exception>
#include <string.h>
#include <map>
#include <string>


void get_labels(FILE* file, auto& labels) {
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
            if (labels[buf])
                throw asm_exception(line, get_string("label \"%s\" redefinition", buf));
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
    funcs.clear();

    char* buf = nullptr;
    size_t nbuf = 0;
    int ip_shift = 1;
    std::string cur_func_name("");
    bool balance = true;
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
            
            if (!balance)
                throw asm_exception(line, "function definition inside another function is permitted");
            if (!*buf)
                throw asm_exception(line, "function's name not found");
            if (funcs[buf].first)
                throw asm_exception(line, get_string("function \"%s\" redefinition", buf));

            ip_shift += sizeof(unsigned char) + sizeof(double) * ARGS_NUMBERS[CMD_FUNC];
            funcs[buf].first = ip_shift;
            cur_func_name = buf;
            balance = false;
        }
        else if (!strcmp(buf, "ENDFUNC")) {
            if (balance)
                throw asm_exception(line, "no function to terminate here");

            ip_shift += sizeof(unsigned char) + sizeof(double) * ARGS_NUMBERS[CMD_ENDFUNC];
            funcs[cur_func_name].second = ip_shift;
            balance = true;
        }
        else
            for (unsigned char i = 0; i < COMMANDS_NAMES.size(); ++i)
                if (!strcmp(buf, COMMANDS_NAMES[i])) {
                    ip_shift += sizeof(unsigned char) + sizeof(double) * ARGS_NUMBERS[i];
                    break;
                }
    }

    if (!balance)
        throw asm_exception(++line, "some functions are not terminated");
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
        } catch (asm_exception& e) {
            fprintf(stderr, STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n"
                        "    line %zu: " STYLE("0") "%s\n", 
                    e.line, e.what());
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
                if (buf[0] == ':' || buf[0] == '#' || buf[0] == '\n')
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
                    else if (i == COMMANDS_NAMES.size() - 1)
                        throw asm_exception(line, get_string("command \"%s\" not found", buf));
            }
        } catch (asm_exception& e) {
            fprintf(stderr, STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n"
                        "    line %zu: " STYLE("0") "%s\n", 
                    e.line, e.what());
            remove(filename);
            exit(1);
        } 
    }
}
