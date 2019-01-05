#include "parser.h"
#include "../proc/reader.h"


int main(int argc, char** argv) {
    if (argc < 2) {
        printf("fatal error: no input files\n");
        exit(1);
    }

    size_t sz = 0;
    const char* text = nullptr;

    std::tie(text, sz) = read_text(argv[1]);

    try {
        auto node = Parser(Lexer({text, sz})).parse();
        // dump(&node);
        generate_asm(&node);
    } catch (parser_exception& e) {
        printf(e.what());
    }
}
