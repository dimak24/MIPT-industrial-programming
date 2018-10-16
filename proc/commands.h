#pragma once

#define SGN "DK24o5"
#define STYLE(code) "\033[" code "m"
#include <array>

enum Command : unsigned char {
    END,
    PUSH,
    POP,
    MUL,
    ADD,
    SUB,
    DIV,
    IN,
    OUT,
    __COMMANDS_NUMBER__
};

static constexpr auto get_commands_args_() {
    std::array<size_t, __COMMANDS_NUMBER__> args_numbers = {};

    args_numbers[PUSH] = 1;

    return args_numbers;
}

static constexpr auto get_commands_names_() {
    std::array<const char*, __COMMANDS_NUMBER__> commands_names = {};

    commands_names[END] = "END";
    commands_names[PUSH] = "PUSH";
    commands_names[POP] = "POP";
    commands_names[MUL] = "MUL";
    commands_names[ADD] = "ADD";
    commands_names[SUB] = "SUB";
    commands_names[DIV] = "DIV";
    commands_names[IN] = "IN";
    commands_names[OUT] = "OUT";

    return commands_names;
}

constexpr static auto ARGS_NUMBERS = get_commands_args_();

constexpr static auto COMMANDS_NAMES = get_commands_names_();


struct my_exception : public std::exception {
    const char* msg;

    my_exception(const char* msg) : msg(msg) {}

    virtual const char* what() {
        return msg;
    }
};
