#include "commands.h"
#include "../onegin/onegin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tuple>
#include <unistd.h>
#include <sys/stat.h>
#include <exception>
#include <system_error>
#include <errno.h>
#include <fcntl.h>


struct dasm_exception : public my_exception {
    dasm_exception(const char* msg) : my_exception(msg) {} 
};


int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, STYLE("1") "dasm: " STYLE("31") "error: " STYLE("0") "no input files\n");
        exit(1);
    }

    for (int f = 1; f < argc; ++f) {

        char filename[1024];
        snprintf(filename, sizeof(filename), "%s.dasm", argv[f]);

        FILE* raw = fopen(filename, "w");
        if (!raw)
            PANIC();

        const char* prog = nullptr;
        size_t size = 0;

        std::tie(prog, size) = read_text(argv[f]);

        if (strncmp(prog, SGN, strlen(SGN))) {
            fprintf(stderr, STYLE("1") "dasm: " STYLE("31") "error: " STYLE("0") "%s: wrong signature\n", argv[f]);
            exit(1);
        }

        const char* cur = prog + strlen(SGN);
        const char* fin = prog + size;
        try {
            while (cur != fin) {
                auto index = *(unsigned char*)cur;
                if (index >= __COMMANDS_NUMBER__) {
                    char msg[1024];
                    snprintf(msg, sizeof(msg), 
                        STYLE("1") "dasm: " STYLE("31") "error:" STYLE("39") "\n    byte %zu:" STYLE("0")
                        " wrong command's code\n", prog - cur + strlen(SGN));
                    throw dasm_exception(msg);
                }
                fprintf(raw, "%s", COMMANDS_NAMES[index]);
                ++cur;
                for (size_t i = 0; i < ARGS_NUMBERS[index]; ++i) {
                    if (fin < cur + sizeof(double)) {
                        char msg[1024];
                        snprintf(msg, sizeof(msg), 
                            STYLE("1") "dasm: " STYLE("31") "error:" STYLE("39") "\n    byte %zu:"
                            STYLE("0") " wrong arguments for command %s (code: %zu)\n", 
                            prog - cur + strlen(SGN), COMMANDS_NAMES[index], index);
                        throw dasm_exception(msg);
                    }
                    fprintf(raw, " %lf", *(double*)cur);
                    cur += sizeof(double);
                }
                fprintf(raw, "\n");
            }
        } catch (dasm_exception& e) {
            fprintf(stderr, "%s", e.what());
            remove(filename);
        }
    }
}
