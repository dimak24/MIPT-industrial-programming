#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tuple>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#include <exception>
#include <system_error>
#include <errno.h>
#include <fcntl.h>
#include "../stack/stack.h"
#include "processor.h"
#include "reader.h"


std::array<double, __REGISTERS_NUMBER__> registers = {};
std::array<double, RAM_SIZE> RAM = {};


int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, STYLE("1") "proc: " STYLE("31") "error: " STYLE("0") "need 1 input file\n");
        exit(1);
    }

    const char* prog = nullptr;
    size_t size = 0;

    std::tie(prog, size) = read_text(argv[1]);

    // try {
    //     verify(prog, size, "proc");
    // } catch (proc_exception& e) {
    //     fprintf(stderr, "%s", e.what());
    //     exit(1);
    // }

    int ip = strlen(SGN);
    const char* fin = prog + size;

    Stack<double> stack;
    double args[2];
    bool end = false;
    while (prog + ip != fin && !end) {
        size_t command = *(unsigned char*)(prog + ip);
        ++ip;
        for (size_t i = 0; i < ARGS_NUMBERS[command]; ++i) {
            args[i] = *(double*)(prog + ip);
            ip += sizeof(double);
        }
        switch (command) {

#define DEF_CMD(cmd, args_number, code) \
            case CMD_##cmd: \
                code; \
                break;
#include "commands.h"
#undef DEF_CMD

            default: __builtin_unreachable();
        }
    }
}
