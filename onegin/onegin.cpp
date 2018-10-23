/*!
    \file
    \brief Сортировка строк текста
    \author Камальдинов Дмитрий
    \date 19.09.2018
    \warning Работает только с текстом в кодировке UTF-8.
*/

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

#define PANIC() \
    do { \
        const int saved_errno = errno; \
        throw std::system_error(saved_errno, std::generic_category(), filename); \
    } while(0)


/*!
    \brief Структура, содержащая указатель на начало строки в тексте и её длину.
*/
struct String {
    const char* begin;
    size_t lenght;
};


/*!
    \brief Функция, необходимая для сортировки строк с конца. Проглатывает пунктуацию в конце строки.
    \param[in] end Ссылка на указатель на конец строки.
    \param[in] begin Указатель на начало строки.
    \param[out] end Ссылка на возможно сдвинутый указатель на конец строки.
*/
void shift(const char*& end, const char* begin) {
    static const char *uni[] = {
        "»",
        "«",
        "…",
        "—",
        " " // неразрывный пробел
    };
    while (end != begin) {
        if (strchr("(),.;:\"\'/{}[]<>#!?@\t ", end[-1])) {
            --end;
            continue;
        }

        bool uni_found = false;

        for (size_t i = 0; i < sizeof(uni) / sizeof(uni[0]); ++i)  {
            const size_t n = strlen(uni[i]);
            if ((size_t) (end - begin) >= n && !memcmp(end - n, uni[i], n)) {
                uni_found = true;
                end -= n;
                break;
            }
        }

        if (!uni_found) 
            break;
    }
}


/*!
    \brief Проверяет, лежит ли символ в ASCII.
    \param[in] c Символ для проверки. 
*/
static inline bool is_ascii(unsigned char c) {
    return c < 128;
}


/*!
    \brief Считывает текст из файла, возвращает его размер в байтах.
    \param[in] filename Название файла.
    \return Пара из считанного текста и его размера в байтах.
    \warning Выходит (как если бы с помощью функции exit), если что-либо пошло не так.
             Возвращаемый указатель указывает на кусок памяти, выделенный как если бы
             с помощью оператора new[].
 */
std::pair<const char*, size_t> read_text(const char* filename) {
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


/*!
    \brief Находит начала строк в тексте и их длины.
    \param[in] text Текст.
    \param[in] size Размер текста в байтах.
    \return Пара из массива структур String, выделенного как если бы с помощью оператора new[], и его размера.
*/
std::pair<String*, size_t> split_text(const char* text, size_t size) {
    size_t str_count = 1;
    const char* cur = text;
    while (cur != text + size && (cur = (char*)memchr(cur, '\n', size - (cur - text)))) {
        ++str_count;
        ++cur;
    }
    String* strings = new String [str_count];
    cur = text;
    const char* next = text;
    size_t last = 0;
    while (cur != text + size && (next = (char*)memchr(cur, '\n', size - (cur - text)))) {
        strings[last++] = {cur, (size_t)(next - cur + 1)};
        cur = ++next;
    }
    strings[last++] = {cur, (size_t)(text + size - cur)};
    return {strings, str_count};
}


/*!
    \brief Выводит текст в файл.
    \param[in] strings Массив из структур String, определяющих строки.
    \param[in] strings_count Количество строк.
    \param[in] cmp Компаратор, принимающий пару аргументов типа String*.
    \warning Выходит (как если бы с помощью функции exit), если открыть файл не удалось.
*/
void write_text(String* strings, size_t strings_count, const char* filename) {
    FILE* sorted = fopen(filename, "w");
    if (!sorted)
        PANIC();
    for (size_t i = 0; i < strings_count; ++i)
        fwrite(strings[i].begin, 1, strings[i].lenght, sorted);
    fclose(sorted);
}


int main() {

// чтение Онегина
    const char* text = "";
    size_t size = 0;

    std::tie(text, size) = read_text("onegin.txt");

// выделение строк
    String* strings = 0;
    size_t strings_count = 0;
    std::tie(strings, strings_count) = split_text(text, size);

// обычная сортировка
    std::sort(strings, strings + strings_count, [](String a, String b) {
                                                    size_t min_lenght = std::min(a.lenght, b.lenght);
                                                    int tmp = memcmp(a.begin, b.begin, min_lenght);
                                                    if (tmp)
                                                        return tmp < 0;
                                                    return a.lenght < b.lenght;
                                                                    });
    write_text(strings, strings_count, "normally_sorted_onegin.txt");

// сортировка с конца (рифмы)
    std::sort(strings, strings + strings_count, [](String a, String b) {
                                                    const char *a_ptr = a.begin + a.lenght,
                                                               *b_ptr = b.begin + b.lenght;

                                                    while (true) {
                                                        shift(a_ptr, a.begin);
                                                        shift(b_ptr, b.begin);  

                                                        if (a_ptr == a.begin || b_ptr == b.begin)
                                                            return b_ptr != b.begin;

                                                        if (is_ascii(a_ptr[-1]) && is_ascii(b_ptr[-1])) {
                                                            if (a_ptr[-1] < b_ptr[-1])
                                                                return true;
                                                            if (a_ptr[-1] > b_ptr[-1])
                                                                return false;
                                                            --a_ptr;
                                                            --b_ptr;
                                                            continue;
                                                        }

                                                        if (is_ascii(a_ptr[-1]) ^ is_ascii(b_ptr[-1]))
                                                            return is_ascii(a_ptr[-1]);

                                                        int curr_ans = 0;
                                                        while (true) {
                                                            unsigned char aa = a_ptr[-1];
                                                            unsigned char bb = b_ptr[-1];

                                                            bool a_ended = (aa >> 6) == 3,
                                                                 b_ended = (bb >> 6) == 3;

                                                            --a_ptr;
                                                            --b_ptr;
                                                            if (a_ended ^ b_ended)
                                                                return a_ended;
                                                            if (aa < bb)
                                                                curr_ans = -1;
                                                            else if (aa > bb)
                                                                curr_ans = 1;
                                                            if (a_ended && b_ended) {
                                                                if (curr_ans)
                                                                    return curr_ans == -1;
                                                                break;
                                                            }
                                                        }
                                                    }});
    write_text(strings, strings_count, "rhythm_sorted_onegin.txt");

    delete[] text;
    delete[] strings;

}
