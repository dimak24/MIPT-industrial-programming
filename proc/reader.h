#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "processor.h"


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
