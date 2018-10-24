#pragma once
#include "processor.h"


static void verify_push_pop(unsigned char command, const char* cur, 
                            const char* begin, const char* from) {
    static char msg[1024];
    int where = *(double*)cur;
    int what = *(double*)(cur + sizeof(double));
    if (where == 1 && (what < 0 || what >= __REGISTERS_NUMBER__)) {
        snprintf(msg, sizeof(msg), 
            STYLE("1") "%s: " STYLE("31") "error:" STYLE("39") "\n    byte %zu:" STYLE("0")
            " %s: wrong register (expected: 0 <= reg < %d, received: %d)\n", 
            from, begin - cur, COMMANDS_NAMES[command], __REGISTERS_NUMBER__, what);
        throw proc_exception(msg);
    }
    else if (where == 2 && (what < 0 || what >= (int)RAM_SIZE)) {
        snprintf(msg, sizeof(msg), 
            STYLE("1") "%s: " STYLE("31") "error:" STYLE("39") "\n    byte %zu:" STYLE("0")
            " %s: RAM index (expected: 0 <= index < %zu, received: %d)\n", 
            from, begin - cur, COMMANDS_NAMES[command], RAM_SIZE, what);
        throw proc_exception(msg);
    }
}


static void verify_jump(unsigned char command, const char* cur,
                        const char* begin, const char* fin, const char* from) {
    static char msg[1024];
    int ip = *(double*)cur;
    if (ip < 0 || ip >= fin - begin) {
        snprintf(msg, sizeof(msg), 
            STYLE("1") "%s: " STYLE("31") "error:" STYLE("39") "\n    byte %zu:" STYLE("0")
            " %s: invalid pointer (expected: 0 <= ip < %zu, received: %d)\n", 
            from, begin - cur, COMMANDS_NAMES[command], fin - begin, ip);
        throw proc_exception(msg);
    }
}


static void verify_command(unsigned char command, const char* cur, 
                           const char* begin, const char* fin, const char* from) {
    static char msg[1024];
    if ((size_t)(fin - cur) < sizeof(double) * ARGS_NUMBERS[command]) {
        snprintf(msg, sizeof(msg), 
            STYLE("1") "%s: " STYLE("31") "error:" STYLE("39") "\n    byte %zu:" STYLE("0")
            " %s: wrong number of arguments (expected %zu)\n", 
            from, begin - cur, COMMANDS_NAMES[command], ARGS_NUMBERS[command]);
        throw proc_exception(msg);
    }

    if (command == CMD_PUSH || command == CMD_POP)
        verify_push_pop(command, cur, begin, from);
    else if (command == CMD_JMP || 
             command == CMD_JA || command == CMD_JB || command == CMD_JNE ||
             command == CMD_JAE || command == CMD_JBE || command == CMD_JE ||
             command == CMD_CALL || command == CMD_FUNC)
        verify_jump(command, cur, begin, fin, from);
}


static void verify(const char* prog, size_t size, const char* from) {
    static char msg[1024];
    if (strncmp(prog, SGN, strlen(SGN))) {
        snprintf(msg, sizeof(msg), 
            STYLE("1") "%s: " STYLE("31") "error: " STYLE("0") "wrong signature\n", from);
        throw proc_exception(msg);
    }
    const char* cur = prog + strlen(SGN);
    const char* fin = prog + size;
    while (cur != fin) {
        size_t command = *(unsigned char*)cur;
        if (command >= __COMMANDS_NUMBER__) {
            snprintf(msg, sizeof(msg), 
                STYLE("1") "%s: " STYLE("31") "error:" STYLE("39") "\n    byte %zu:" STYLE("0")
                " wrong command's code: %zu\n", from, prog - cur, command);
            throw proc_exception(msg);
        }
        ++cur;
        verify_command(command, cur, prog, fin, from);
        cur += sizeof(double) * ARGS_NUMBERS[command];
    }
}
