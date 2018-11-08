#include "list.h"
#include <iostream>
#include <string>


void push_pop_test() {
    List<int> list;

    assert(list.empty());

    for (int i = 0; i < 1050; ++i) {
        list.push_back(i);
        list.push_front(-i);

        assert(list.back() == i);
        assert(list.front() == -i);
        assert(list.size() == (size_t)(2 * (i + 1)));
    }

    for (int i = 0; i < 1050; ++i) { 
        assert(list.back() == 1049 - i);
        list.pop_back();

        assert(list.front() == i - 1049);
        list.pop_front();

        assert(list.size() == (size_t)(2100 - 2 * (i + 1)));
    }

    assert(list.empty());
}


void copy_move_test() {
    std::string str[] = {"lol", "KEK"};

    List<std::string> list;
    for (int i = 0; i < 1050; ++i)
        list.push_back(str[i % 2]);

    List<std::string> list1(list);
    List<std::string> list2(std::move(list));

    for (int i = 0; i < 1050; ++i) { 
        assert(list1.back() == str[(i + 1) % 2]);
        assert(list2.back() == str[(i + 1) % 2]);
    
        list1.pop_back();
        list2.pop_back();
    }
}

struct A {
    int a;
    std::string b, c;

    A() {}
    A(int a, std::string b, std::string c) : a(a), b(b), c(c) {}

    bool operator==(const A& another) {
        return a == another.a && b == another.b && c == another.c;
    }

    void square() {
        a *= a;
    }

    void dump(FILE* file, int indent = 0) const {
        fprintf(file, "%*sstruct A at %p = {\n"
                      "%*s  a (%p) = %d\n"
                      "%*s  b (%p) = %s\n"
                      "%*s  c (%p) = %s\n%*s}\n" ,
                indent, "", this, indent, "", &a, a, 
                indent, "", &b, b.c_str(), 
                indent, "", &c, c.c_str(), indent, "");
    }
};


void emplace_test() {
    List<A> list;

    list.emplace_back(2, "lol", "kek");
    list.emplace_back(4, "kek", "lol");
    list.emplace_front(-2, "lol", "kek");
    list.emplace_front(-4, "lol", "kek");

    assert(list.front() == A(-4, "lol", "kek"));
    list.pop_front();
    assert(list.back() == A(4, "kek", "lol"));
    list.pop_back();


    assert(list.back() == A(2, "lol", "kek"));
    list.pop_back();
    assert(list.front() == A(-2, "lol", "kek"));
}


void iterators_test() {
    {
        List<int> list;
        for (int i = 0; i < 2000; ++i)
            list.push_back(i);


        int i = 0;
        for (List<int>::iterator it = list.begin(); it != list.end(); ++it, ++i)
            assert(*it == i);
    

        --i;
        for (List<int>::reverse_iterator it = list.rbegin(); it != list.rend(); ++it, --i)
            assert(*it == i);
    }

    {
        List<int> list;
        for (int i = 0; i < 10; ++i)
            list.push_back(i);

        assert(*list.begin() == 0 && *(--list.end()) == 9 &&
               *list.rbegin() == 9 && *(--list.rend()) == 0 &&
               ++--list.begin() == list.begin() && --++list.end() == list.end() &&
               ++--list.rbegin() == list.rbegin() && --++list.rend() == list.rend());
    }

    {
        List<A> list;
        list.emplace_back(10, "KEK", "LOL");
        list.begin()->square();
        assert(list.back() == A(100, "KEK", "LOL"));
    }
}


void insert_remove_by_iterators_test() {
    List<int> list;
    for (int i = 0; i < 10; ++i)
        list.push_back(i);

    List<int>::iterator it = list.begin();

    ++it; ++it; ++it; ++it; ++it;
    assert(*it == 5);

    list.insert(it, 20);
    list.insert(it, 30);

    ++it; ++it; ++it; ++it;
    list.insert(it, 25);
    list.insert(it, 35);

    list.remove(it);

    it = list.begin();
    ++it; ++it;

    list.remove(list.remove(it));


    int a[] = {0, 1, 4, 20, 30, 5, 6, 7, 8, 25, 35};

    int j = 0;
    for (List<int>::iterator i = list.begin(); i != list.end(); ++i, ++j)
        assert(*i == a[j]);
}


void indices_test() {
    List<int> list;
    for (int i = 0; i < 10; ++i)
        list.push_back(i);                                  // 0..9

    List<int>::iterator it = list.begin();

    ++it; ++it; ++it; ++it; ++it;
    assert(*it == 5);

    list.insert(it, 20);
    list.insert(it, 30);                                   // 0..4 20 30 5..9

    ++it; ++it; ++it; ++it;
    list.insert(it, 25);
    list.insert(it, 35);                                   // 0..4 20 30 5..8 25 35 9

    list.remove(it);                                       // 0..4 20 30 5..8 25 35

    it = list.begin();
    ++it; ++it;

    list.remove(list.remove(it));                          // 0 1 4 20 30 5..8 25 35

    list.push_back(100);
    list.push_front(200);                                  //200 0 1 4 20 30 5..8 25 35 100

    list.insert(--list.end(), 156);                        //200 0 1 4 20 30 5..8 25 35 156 100


    list.pull();
    list.insert(4, 420);
    list.remove(8);                                        //200 0 1 4 420 20 30 5 6 8 25 35 156 100
    list.pull();

    int a[] = {200, 0, 1, 4, 420, 20, 30, 5, 6, 8, 25, 35, 156, 100};

    for (size_t j = 0; j < list.size(); ++j)
        assert(list[j] == a[j]);
}


void dump_test() {
    List<A> list;

    list.push_back(A(100, "lol", "kek"));
    list.push_back(A(200, "lol", "kek"));
    list.push_back(A(300, "lol", "kek"));
    list.remove(----list.end());

    list.dump();
}


int main() {
    push_pop_test();
    copy_move_test();
    emplace_test();
    iterators_test();
    insert_remove_by_iterators_test();
    indices_test();
    dump_test();
}
