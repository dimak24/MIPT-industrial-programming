#pragma once

#include <string.h>
#include <map>


class Trie {
private:
    struct Vertex {
        std::map<char, Vertex*> next;
        int is_terminal;

        Vertex() : is_terminal(0) {}

        ~Vertex() {
            for (auto it = next.begin(); it != next.end(); ++it)
                delete it->second;
            next.clear();
        }
    };

    Vertex* root_;

public:
    Trie() : root_(new Vertex()) {}

    ~Trie() {
        delete root_;
    }

    Trie(const Trie&) = delete;
    Trie(Trie&&) = delete;


    void append(const char* word, int op) {
        Vertex* cur = root_;
        while (*word) {
            if (!cur->next.count(*word))
                cur->next[*word] = new Vertex();
            cur = cur->next[*word];
            ++word;
        }
        cur->is_terminal = op;
    }

    std::pair<int, int> find_longest_prefix(const char* text, const char* last) const {
        Vertex* cur = root_;
        int ans = 0, len = 0;
        while (text != last) {
            if (!cur->next.count(*text))
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
