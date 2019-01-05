#include "derivative.h"
#include "to_latex.h"
#include "lexer.h"
#include "parser.h"
#include <string_view>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "trie.h"
#include <iostream>


struct S {
    int a;

    friend std::ostream& operator<<(std::ostream& out, S s) {
        return out << s.a;
    }
};

int main() {
    auto prog = "b := 2 arr := [1, 2, 3, b] a := arr[3,4] a = 6 print(a)";

    try {
        auto node = Parser(Lexer({prog, strlen(prog)})).parse();
        // dump(&node);
        // generate_asm(&node);
    } catch (parser_exception& e) {
        printf(e.what());
    }
}
