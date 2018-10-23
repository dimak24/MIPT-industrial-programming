
#include "processor.h"
#include "reader.h"
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
#include <map>


int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, STYLE("1") "dasm: " STYLE("31") "error: " STYLE("0") "no input files\n");
        exit(1);
    }

    for (int f = 1; f < argc; ++f) {
        const char* prog = nullptr;
        size_t size = 0;

        std::tie(prog, size) = read_text(argv[f]);

        // try {
        //     verify(prog, size, "dasm");
        // } catch (proc_exception& e) {
        //     fprintf(stderr, "%s", e.what());
        //     exit(1);
        // }

        char filename[1024];
        snprintf(filename, sizeof(filename), "%s.dasm", argv[f]);
        FILE* raw = fopen(filename, "w");
        if (!raw)
            PANIC();

        std::map<int, int> lines_begin;
        int line = 0;

        const char* cur = prog + strlen(SGN);
        const char* fin = prog + size;
        
        double tmp[2];
        while (cur != fin) {
            lines_begin[cur - prog] = line++;
            unsigned char index = *(unsigned char*)cur;
            fprintf(raw, "%s", COMMANDS_NAMES[index]);
            ++cur;
            for (size_t i = 0; i < ARGS_NUMBERS[index]; ++i) {
                tmp[i] = *(double*)cur;
                if (index == CMD_PUSH) {
                    if (i == 1) {
                        switch ((int)tmp[0]) {
                            case 0:
                                fprintf(raw, " %lf", *(double*)cur);
                                break;
                            case 1:
                                fprintf(raw, " %s", REGISTERS_NAMES[*(double*)cur]);
                                break;
                            case 2:
                                size_t shift = (size_t)tmp[1] / (__REGISTERS_NUMBER__ + 1),
                                       reg = (size_t)tmp[1] % (__REGISTERS_NUMBER__ + 1);
                                if (!reg)
                                    fprintf(raw, " [%zu]", shift);
                                else if (!shift)
                                    fprintf(raw, " [%s]", REGISTERS_NAMES[reg - 1]);
                                else
                                    fprintf(raw, " [%s+%zu]", REGISTERS_NAMES[reg - 1], shift);
                                break;
                        }
                    }
                }
                else if (index == CMD_POP) {
                    if (i == 1) {
                        switch ((int)tmp[0]) {
                            case 0:
                                break;
                            case 1:
                                fprintf(raw, " %s", REGISTERS_NAMES[tmp[1]]);
                                break;
                            case 2:
                                size_t shift = (size_t)tmp[1] / (__REGISTERS_NUMBER__ + 1),
                                       reg = (size_t)tmp[1] % (__REGISTERS_NUMBER__ + 1);
                                if (!reg)
                                    fprintf(raw, " [%zu]", shift);
                                else if (!shift)
                                    fprintf(raw, " [%s]", REGISTERS_NAMES[reg - 1]);
                                else
                                    fprintf(raw, " [%s+%zu]", REGISTERS_NAMES[reg - 1], shift);
                                break;
                        }
                    }
                }
                else if (index == CMD_JMP || index == CMD_JA || index == CMD_JB || index == CMD_JE || 
                         index == CMD_JNE || index == CMD_JAE || index == CMD_JBE)
                    fprintf(raw, " %d", lines_begin[(int)tmp[0]]);
                else
                    fprintf(raw, " %lf", *(double*)cur);
                cur += sizeof(double);
            }
            fprintf(raw, "\n");
        }
    }
}
