#pragma once


#include <array>
#include <system_error>
#include <errno.h>



#define SGN "DK24o5"
#define STYLE(code) "\033[" code "m"
#define PANIC() \
    do { \
        const int saved_errno = errno; \
        throw std::system_error(saved_errno, std::generic_category(), filename); \
    } while(0)


struct proc_exception : public std::exception {
    const char* msg;

    proc_exception(const char* msg) : msg(msg) {}

    virtual const char* what() {
        return msg;
    }
};



enum Command : unsigned char {

#define DEF_CMD(cmd, args_number, code) CMD_##cmd,
#include "commands.h"
#undef DEF_CMD

    __COMMANDS_NUMBER__
};


enum Register : unsigned char {
    rax,
    rbx,
    rcx,
    rdx,
    __REGISTERS_NUMBER__
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
