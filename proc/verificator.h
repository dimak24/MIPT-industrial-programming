#pragma once
#include "processor.h"


static void verify_push_pop(unsigned char command, const char* cur, const char* begin) {
    int where = *(double*)cur;
    int what = *(double*)(cur + sizeof(double));

    if (where == 1 && (what < 0 || what >= __REGISTERS_NUMBER__))
        throw verificator_exception(begin - cur, 
                    get_string("%s: wrong register (expected: 0 <= reg < %d, received: %d)", 
                               COMMANDS_NAMES[command], __REGISTERS_NUMBER__, what));
    else if (where == 2 && (what < 0 || what >= (int)RAM_SIZE))
        throw verificator_exception(begin-cur, 
                    get_string("%s: RAM index (expected: 0 <= index < %zu, received: %d)",
                               COMMANDS_NAMES[command], RAM_SIZE, what));
}


static void verify_jump(unsigned char command, const char* cur, const char* beg, const char* fin) {
    int ip = *(double*)cur;
    if (ip < 0 || ip >= fin - beg)
        throw verificator_exception(beg - cur, 
                    get_string("%s: invalid pointer (expected: 0 <= ip < %zu, received: %d)", 
                               COMMANDS_NAMES[command], fin - beg, ip));
}


static void verify_draw(const char* cur, const char* beg) {
    double w = *(double*)cur;
    double h = *(double*)(cur + sizeof(double));
    double ndata = *(double*)(cur + 2  * sizeof(double));

    if (w < 0 || h < 0 || ndata < 0)
        throw verificator_exception(beg - cur,
                    "DRAW: all width, height and ndata have to be greater than 0");
    if (ndata >= RAM_SIZE)
        throw verificator_exception(beg - cur + 2 * sizeof(double),
                    get_string("DRAW: ndata greater than RAM_SIZE (RAM_SIZE = %zu)", RAM_SIZE));
}


static void verify_command(unsigned char command, const char* cur, const char* beg, const char* fin) {
    if ((size_t)(fin - cur) < sizeof(double) * ARGS_NUMBERS[command])
        throw verificator_exception(beg - cur, 
                    get_string("%s: wrong number of arguments (expected %zu)", 
                               COMMANDS_NAMES[command], ARGS_NUMBERS[command]));

    if (command == CMD_PUSH || command == CMD_POP)
        verify_push_pop(command, cur, beg);
    else if (command == CMD_JMP || 
             command == CMD_JA || command == CMD_JB || command == CMD_JNE ||
             command == CMD_JAE || command == CMD_JBE || command == CMD_JE ||
             command == CMD_CALL || command == CMD_FD)
        verify_jump(command, cur, beg, fin);
    else if (command == CMD_DRAW)
        verify_draw(cur, beg);
}


static void verify(const char* prog, size_t size) {
    if (strncmp(prog, SGN, strlen(SGN)))
        throw verificator_exception(-1, "wrong signature");

    const char* cur = prog + strlen(SGN);
    const char* fin = prog + size;
    while (cur != fin) {
        size_t command = *(unsigned char*)cur;
        if (command >= __COMMANDS_NUMBER__)
            throw verificator_exception(prog - cur, get_string("wrong command's code: %zu", command));
        ++cur;
        verify_command(command, cur, prog, fin);
        cur += sizeof(double) * ARGS_NUMBERS[command];
    }
}
