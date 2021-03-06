#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tuple>
#include <map>
#include "processor.h"
#include "reader.h"
#include "verificator.h"


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
            verify(prog, size);
        } catch (verificator_exception& e) {
            fprintf(stderr, STYLE("1") "proc: " STYLE("31") "error:" STYLE("39") "\n"
                            "    byte %zu: " STYLE("0") "%s\n", 
                    e.byte, e.what());
            exit(1);
        }

        char filename[1024];
        snprintf(filename, sizeof(filename), "%s.dasm", argv[f]);
        FILE* raw = fopen(filename, "w");
        if (!raw)
            PANIC();

        std::map<int, int> lines_begin;
        int line = 0;
        int indent = 0;

        const char* cur = prog + strlen(SGN);
        const char* fin = prog + size;
        
        double tmp[__MAXIMAL_ARGS_NUMBER__];
        while (cur != fin) {
            lines_begin[cur - prog] = ++line;
            unsigned char index = *(unsigned char*)cur;
            
            if (index == CMD_RET || index == CMD_LEAVE)
                --indent;
            
            fprintf(raw, "%*s%s", indent * 2, "", COMMANDS_NAMES[index]);
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
                // else if (index == CMD_JMP || index == CMD_JA || index == CMD_JB || index == CMD_JE || 
                //          index == CMD_JNE || index == CMD_JAE || index == CMD_JBE)
                //     fprintf(raw, " %d", lines_begin[(int)tmp[0]]);
                // else if (index == CMD_CALL)
                //     fprintf(raw, " f%d", lines_begin[(int)tmp[0]]);
                else
                    fprintf(raw, " %lf", *(double*)cur);
                // if (i == 1 && index == CMD_FD)
                //     fprintf(raw, " (f%d)", lines_begin[(int)tmp[0]]);
                cur += sizeof(double);
            }

            if (index == CMD_FD) 
                ++indent;
            
            fprintf(raw, "\n");
        }
    }
}
