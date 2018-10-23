
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
    labels.clear();

    char* buf = nullptr;
    size_t nbuf = 0;
    int ip_shift = 1;

    while (getline(&buf, &nbuf, file) > 0) {
        if (buf[0] == ':') {
            ++buf;
            buf[strlen(buf) - 1] = 0;
            if (labels[buf]) {
                throw proc_exception("Repeated label");
            }
            labels[buf] = ip_shift;
        }
        else {
            char* ch = buf;
            while (*ch && *ch != '\n' && *ch != ' ')
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

        try {
            get_labels(raw, labels);
        } catch (proc_exception& e) {
            fprintf(stderr, "%s", e.what());
            remove(filename);
            exit(1);   
        }

        fprintf(compiled, SGN);

        fseek(raw, 0, SEEK_SET);

        char* buf = nullptr;
        size_t nbuf = 0;
        size_t line = 0;

        try {
            while (getline(&buf, &nbuf, raw) > 0) {
                ++line;
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
                        parse(i, ch + 1, compiled, line, labels);
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
