#pragma once
    
#include <array>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <algorithm>
#include <tuple>
#include <exception>
#include <system_error>
#include <errno.h>


#define SGN "DK24o5"
#define STYLE(code) "\033[" code "m"
#define PANIC() \
    do { \
        const int saved_errno = errno; \
        throw std::system_error(saved_errno, std::generic_category(), filename); \
    } while(0)


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


struct proc_exception : public std::exception {
    const char* msg;

    proc_exception(const char* msg) : msg(msg) {}

    virtual const char* what() {
        return msg;
    }
};


static std::pair<const char*, size_t> read_text(const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        PANIC();

    struct stat s = {};
    if (fstat(fd, &s) == -1)
        PANIC();

    size_t size = s.st_size;
    char* text = new char [size];
    
    for (size_t n_read = 0; n_read != size; ) {
        ssize_t r = read(fd, text + n_read, size - n_read);
        if (r == -1)
            PANIC();
        if (!r)
            break;
        n_read += r;
    }

    close(fd);

    return {text, size};
}

static void verify(const char* prog, size_t size, const char* from) {
    if (strncmp(prog, SGN, strlen(SGN))) {
        static char msg[1024];
        snprintf(msg, sizeof(msg), 
            STYLE("1") "%s: " STYLE("31") "error: " STYLE("0") "wrong signature\n", from);
        throw proc_exception(msg);
    }
    const char* cur = prog + strlen(SGN);
    const char* fin = prog + size;  
    while (cur != fin) {
        size_t command = *(unsigned char*)cur;
        if (command >= __COMMANDS_NUMBER__) {
            char msg[1024];
            snprintf(msg, sizeof(msg), 
                STYLE("1") "%s: " STYLE("31") "error:" STYLE("39") "\n    byte %zu:" STYLE("0")
                " wrong command's code\n", from, prog - cur + strlen(SGN));
            throw proc_exception(msg);
        }
        ++cur;
        for (size_t i = 0; i < ARGS_NUMBERS[command]; ++i) {
            if (fin < cur + sizeof(double)) {
                char msg[1024];
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
