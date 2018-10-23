#pragma once


#include <array>
#include <string.h>


#define SGN "DK24o5"
#define STYLE(code) "\033[" code "m"
#define PANIC() \
    do { \
        const int saved_errno = errno; \
        throw std::system_error(saved_errno, std::generic_category(), filename); \
    } while(0)


enum Command : unsigned char {

#define DEF_CMD(cmd, args_number, code) CMD_##cmd,
#include "commands.h"
#undef DEF_CMD

    __COMMANDS_NUMBER__
};

static constexpr auto get_commands_args_() {
    std::array<size_t, __COMMANDS_NUMBER__> args_numbers = {};

#define DEF_CMD(cmd, args_number, code) args_numbers[CMD_##cmd] = args_number;
#include "commands.h"
#undef DEF_CMD

    return args_numbers;
}

static constexpr auto get_commands_names_() {
    std::array<const char*, __COMMANDS_NUMBER__> commands_names = {};

#define DEF_CMD(cmd, args_number, code) commands_names[CMD_##cmd] = #cmd;
#include "commands.h"
#undef DEF_CMD

    return commands_names;
}

enum Register : unsigned char {
    rax,
    rbx,
    rcx,
    rdx,
    __REGISTERS_NUMBER__
};

static constexpr auto get_registers_names_() {
    std::array<const char*, __REGISTERS_NUMBER__> registers_names = {};

    registers_names[rax] = "rax";
    registers_names[rbx] = "rbx";
    registers_names[rcx] = "rcx";
    registers_names[rdx] = "rdx";

    return registers_names;
}


constexpr static auto ARGS_NUMBERS = get_commands_args_();
constexpr static auto COMMANDS_NAMES = get_commands_names_();
constexpr static auto REGISTERS_NAMES = get_registers_names_();

constexpr size_t RAM_SIZE = 1e4;


struct proc_exception : public std::exception {
    const char* msg;

    proc_exception(const char* msg) : msg(msg) {}

    virtual const char* what() {
        return msg;
    }
};

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
            //static char msg[1024];
            snprintf(msg, sizeof(msg), 
                STYLE("1") "%s: " STYLE("31") "error:" STYLE("39") "\n    byte %zu:" STYLE("0")
                " wrong command's code\n", from, prog - cur + strlen(SGN));
            throw proc_exception(msg);
        }
        ++cur;
        for (size_t i = 0; i < ARGS_NUMBERS[command]; ++i) {
            if (fin < cur + sizeof(double)) {
                if (fin >= cur + 3 && *cur == 'r' && *(cur + 1) - 'a' < __REGISTERS_NUMBER__ && *(cur + 2) == 'x')
                    continue;
                //static char msg[1024];
                snprintf(msg, sizeof(msg), 
                    STYLE("1") "%s: " STYLE("31") "error:" STYLE("39") "\n    byte %zu:"
                    STYLE("0") " wrong arguments for command %s (code: %zu)\n", 
                    from, prog - cur + strlen(SGN), COMMANDS_NAMES[command], command);
                throw proc_exception(msg);
            }
            cur += sizeof(double);
        }
    }
}
