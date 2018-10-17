#include "commands.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <exception>


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

        fprintf(compiled, SGN);

        char* buf = nullptr;
        size_t nbuf = 0;
        size_t line = 0;

        try {
            while (getline(&buf, &nbuf, raw) > 0) {
                ++line;
                char* ch = buf;
                char* st = buf;
                char* fin = buf + strlen(buf) - 1;
                while (ch != fin && *ch != ' ')
                    ++ch;
                *ch = 0;
                for (unsigned char i = 0; i < COMMANDS_NAMES.size(); ++i)
                    if (!strcmp(buf, COMMANDS_NAMES[i])) {
                        fwrite(&i, sizeof(unsigned char), 1, compiled);
                        for (size_t j = 0; j < ARGS_NUMBERS[i]; ++j) {
                            if (ch != fin)
                                st = ch + 1;
                            while (st != fin && *ch == ' ')
                                ++st;
                            if (st == fin && j < ARGS_NUMBERS[i] - 1) {
                                char msg[1024];
                                snprintf(msg, sizeof(msg), 
                                    STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:" STYLE("0") " %s: wrong number of aruments (expected %zu)\n",
                                    line, COMMANDS_NAMES[i], ARGS_NUMBERS[i]);
                                throw proc_exception(msg);
                            }
                            ch = st;
                            while (ch != fin && *ch != ' ')
                                ++ch;
                            *ch = 0;

                            char* end = nullptr;
                            double arg = strtod(st, &end);
                            if (end != fin && *end != ' ') {
                                char msg[1024];
                                snprintf(msg, sizeof(msg), 
                                    STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:" STYLE("0") " wrong arument\n", line);
                                throw proc_exception(msg);
                            }
                            fwrite(&arg, sizeof(double), 1, compiled);
                        }
                        break;
                    }
                    else if (i == COMMANDS_NAMES.size() - 1) {
                        char msg[1024];
                        snprintf(msg, sizeof(msg), 
                            STYLE("1") "asm: " STYLE("31") "error:" STYLE("39") "\n    line %zu:" STYLE("0") " command \"%s\" not found\n", line, buf);
                        throw proc_exception(msg);
                    };
            }
        } catch (proc_exception& e) {
            fprintf(stderr, "%s", e.what());
            remove(filename);
            exit(1);
        } 
    }
}
