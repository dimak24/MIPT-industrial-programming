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
#include "../stack/stack.h"
#include "../onegin/onegin.h"


struct proc_exception : public my_exception {
    proc_exception(const char* msg) : my_exception(msg) {}
};


int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, STYLE("1") "proc: " STYLE("31") "error: " STYLE("0") "need 1 input file\n");
        exit(1);
    }

    const char* prog = nullptr;
    size_t size = 0;

    std::tie(prog, size) = read_text(argv[1]);

    if (strncmp(prog, SGN, strlen(SGN))) {\
        fprintf(stderr, STYLE("1") "proc: " STYLE("31") "error: " STYLE("0") "%s: wrong signature\n", argv[1]);
        exit(1);
    }

    const char* cur = prog + strlen(SGN);
    const char* fin = prog + size;

    Stack<double> stack;
    double args[2];
    bool end = false;

    try {
        while (cur != fin && !end) {
            size_t command = *(unsigned char*)cur;
            if (command >= __COMMANDS_NUMBER__) {
                char msg[1024];
                snprintf(msg, sizeof(msg), 
                    STYLE("1") "dasm: " STYLE("31") "error:" STYLE("39") "\n    byte %zu:" STYLE("0")
                    " wrong command's code\n", prog - cur + strlen(SGN));
                throw proc_exception(msg);
            }
            ++cur;
            for (size_t i = 0; i < ARGS_NUMBERS[command]; ++i) {
                if (fin < cur + sizeof(double)) {
                    char msg[1024];
                    snprintf(msg, sizeof(msg), 
                        STYLE("1") "dasm: " STYLE("31") "error:" STYLE("39") "\n    byte %zu:"
                        STYLE("0") " wrong arguments for command %s (code: %zu)\n", 
                        prog - cur + strlen(SGN), COMMANDS_NAMES[command], command);
                    throw proc_exception(msg);
                }
                args[i] = *(double*)cur;
                cur += sizeof(double);
            }

            switch (command) {
                case PUSH:
                    stack.push(args[0]);
                    break;
                case POP:
                    stack.pop();
                    break;
                case IN:
                    scanf("%lf", &args[0]);
                    stack.push(args[0]);
                    break;
                case ADD:
                    args[0] = stack.pop(), args[1] = stack.pop();
                    stack.push(args[0] + args[1]);
                    break;
                case MUL:
                    args[0] = stack.pop(), args[1] = stack.pop();
                    stack.push(args[0] * args[1]);
                    break;
                case SUB:
                    args[0] = stack.pop(), args[1] = stack.pop();
                    stack.push(args[1] - args[0]);
                    break;
                case DIV:
                    args[0] = stack.pop(), args[1] = stack.pop();
                    stack.push(args[1] / args[0]);
                    break;
                case OUT:
                    printf("%lf\n", stack.pop());
                    break;
                case END:
                    end = true;
                    break;
                default: __builtin_unreachable();
            }
        }
    } catch (proc_exception& e) {
        fprintf(stderr, "%s", e.what());
    }
}
