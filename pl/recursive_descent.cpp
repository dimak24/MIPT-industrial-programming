#include <assert.h>
#include <stdio.h>


/*
    G >> E\0
    E >> T{[+-]T}*
    T >> P{[/*]P}*
    P >> (E)|N
    N >> [0-9]+
*/


int p = 0;
const char* s = nullptr;

int getE();


int getN() {
    int val = 0;

    int p0 = p;
    while ('0' <= s[p] && s[p] <= '9') {
        val = val * 10 + (s[p] - '0');   
        ++p;
    }

    assert(p0 != p);

    return val;
}

int getP() {
    int val = 0;

    if (s[p] == '(') {
        ++p;
        val = getE();

        assert(s[p] == ')');
        ++p;
        return val;
    }

    val = getN();

    return val;
}

int getT() {
    int val = getP();

    while (s[p] == '*' || s[p] == '/') {
        char op = s[p];
        ++p;
        int val2 = getP();

        if (op == '*')
            val *= val2;
        else
            val /= val2;
    }

    return val;
}

int getE() {
    int val = getT();

    while (s[p] == '-' || s[p] == '+') {
        char op = s[p];
        ++p;
        int val2 = getT();

        if (op == '-')
            val -= val2;
        else
            val += val2;
    }

    return val;
}

int getG(const char* str) {
    s = str;
    p = 0;

    int val = getE();
    
    assert(s[p] == '\0');
    ++p;

    return val;
}


int main() {
    printf("%d\n", getG("(10+20)*(3+4)*2-10*4"));
}
