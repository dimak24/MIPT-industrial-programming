#pragma once

#include <string.h>
#include <map>

constexpr const char* ALPHABET = "=+-*^%@$!~/";
constexpr const size_t ALPHABET_SIZE = strlen(ALPHABET);


class Trie {
private:
    struct Vertex {
        std::map<char, Vertex*> next;
        int is_terminal;

        Vertex() : is_terminal(0) {}
    };

    Vertex* root_;

public:
    Trie() : root_(new Vertex()) {}

    void append(const char* word, int op) {
        Vertex* cur = root_;
        while (*word) {
            if (cur->next.empty() || cur->next.find(*word) == cur->next.end())
                cur->next[*word] = new Vertex();
            cur = cur->next[*word];
            ++word;
        }
        cur->is_terminal = op;
    }

    std::pair<int, int> find_longest_prefix(const char* text, const char* last) {
        Vertex* cur = root_;
        int ans = 0, len = 0;
        while (text != last) {
            if (cur->next.find(*text) == cur->next.end())
                return {ans, len};

            cur = cur->next[*text];
            ++len;

            if (cur->is_terminal)
                ans = cur->is_terminal;

            ++text;
        }
        return {ans, len};
    }
};
