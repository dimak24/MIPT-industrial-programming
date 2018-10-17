#include "commands.h"
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


int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, STYLE("1") "dasm: " STYLE("31") "error: " STYLE("0") "no input files\n");
        exit(1);
    }

    for (int f = 1; f < argc; ++f) {
        const char* prog = nullptr;
        size_t size = 0;

        std::tie(prog, size) = read_text(argv[f]);

        try {
            verify(prog, size, "dasm");
        } catch (proc_exception& e) {
            fprintf(stderr, "%s", e.what());
            exit(1);
        }

        char filename[1024];
        snprintf(filename, sizeof(filename), "%s.dasm", argv[f]);
        FILE* raw = fopen(filename, "w");
        if (!raw)
            PANIC();

        const char* cur = prog + strlen(SGN);
        const char* fin = prog + size;
        
        while (cur != fin) {
            size_t index = *(unsigned char*)cur;
            fprintf(raw, "%s", COMMANDS_NAMES[index]);
            ++cur;
            for (size_t i = 0; i < ARGS_NUMBERS[index]; ++i) {
                fprintf(raw, " %lf", *(double*)cur);
                cur += sizeof(double);
            }
            fprintf(raw, "\n");
        }
    }
}
