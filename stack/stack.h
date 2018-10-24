#include <assert.h>
#include <inttypes.h>
#include <memory>
#include <new>
#include <stdint.h>
#include <stdio.h>
#include <type_traits>
#include <stdlib.h>


#define ASSERT_OK() \
    do { \
          if (!ok()) { \
            dump(); \
            exit(1); \
        } \
    } while(0)


template <typename T>
class Stack {
private:
    const unsigned int TRUE_CANARY__ = 0xDEADBEEFu;

    const float GROWTH_FACTOR__ = 2;
    const float SHRINKAGE_FACTOR__ = 0.5;
    const size_t INITIAL_CAPACITY__ = 128;
    const size_t MAX_CAPACITY__ = 1e9;


    unsigned int stack_canary_begin;
    
    size_t capacity_;
    size_t size_;
    unsigned char* buffer_;
    
    unsigned int stack_canary_end;


    unsigned int* get_first_canary_ptr_() const {
        return (unsigned int*)buffer_;
    }

    unsigned int* get_second_canary_ptr_() const {
        return (unsigned int*)(buffer_ + (sizeof(unsigned int) + sizeof(T) * capacity_));
    }

    unsigned int* get_third_canary_ptr_() const {
        return (unsigned int*)(buffer_ + (sizeof(unsigned int) * 2 + 
                                          sizeof(T) * capacity_ + sizeof(uint64_t)));
    }

    uint64_t* get_check_sum_ptr_() const {
        return (uint64_t*)(buffer_ + (sizeof(unsigned int) * 2 + sizeof(T) * capacity_));
    }

    uint64_t sum_up_() const {
        uint64_t ans = 0;
        for (unsigned char* ptr = buffer_ + sizeof(unsigned int); 
             ptr != buffer_ + sizeof(unsigned int) + sizeof(T) * capacity_; ++ptr)
                ans += (uint64_t)*ptr;
        return ans;
    }

    template <typename Dummy = void>
    static typename std::enable_if<std::is_class<T>::value, Dummy>::type dump_(const T& t, FILE* file) {
        t.dump(file);
    }

    static void dump_(int a, FILE* file) {
        fprintf(file, "int (%p) = %d\n", &a, a);
    }

    static void dump_(long long a, FILE* file) {
        fprintf(file, "int (%p) = %lld\n", &a, a);
    }

    static void dump_(const char* a, FILE* file) {
        fprintf(file, "const char* (%p) = \"%s\"\n", a, a);
    }

    static void dump_(double a, FILE* file) {
        fprintf(file, "double (%p) = %g\n", &a, a);
    }

    static void dump_(long unsigned a, FILE* file) {
        fprintf(file, "long unsigned (%p) = %lu\n", &a, a);
    }

    void resize_(float resize_factor) {
        uint64_t check_sum = *get_check_sum_ptr_();

        unsigned char* new_buffer = new unsigned char [(size_t)(sizeof(T) * capacity_ * GROWTH_FACTOR__) + 
                                                       sizeof(uint64_t) + sizeof(unsigned int) * 3];
        std::uninitialized_move((T*)(buffer_ + sizeof(unsigned int)), 
                                (T*)(buffer_ + sizeof(unsigned int) + sizeof(T) * capacity_),
                                (T*)(new_buffer + sizeof(unsigned int)));
        
        std::swap(buffer_, new_buffer);
        
        capacity_ *= resize_factor;
        *get_second_canary_ptr_() = TRUE_CANARY__;
        *get_first_canary_ptr_() = TRUE_CANARY__;
        *get_third_canary_ptr_() = TRUE_CANARY__;

        *get_check_sum_ptr_() = check_sum;
    }

    void resize_if_necessary_() {
        if (size_ == capacity_)
            resize_(GROWTH_FACTOR__);
        else if (capacity_ > INITIAL_CAPACITY__ && size_ == capacity_ / 4)
            resize_(SHRINKAGE_FACTOR__);
    }

public:
    Stack() 
        : stack_canary_begin(TRUE_CANARY__), 
          capacity_(INITIAL_CAPACITY__), size_(0), 
          buffer_(new unsigned char [sizeof(T) * capacity_ + sizeof(uint64_t) + sizeof(unsigned int) * 3]),
          stack_canary_end(TRUE_CANARY__) {
            *get_second_canary_ptr_() = TRUE_CANARY__;
            *get_first_canary_ptr_() = TRUE_CANARY__;
            *get_third_canary_ptr_() = TRUE_CANARY__;

            *get_check_sum_ptr_() = sum_up_();
    }

    Stack(const Stack<T>& another)
        : stack_canary_begin(another.stack_canary_begin), capacity_(another.capacity_), size_(another.size_),
          buffer_(new unsigned char [sizeof(T) * capacity_ + sizeof(uint64_t) + sizeof(unsigned int) * 3]),
          stack_canary_end(another.stack_canary_end) {
            *get_second_canary_ptr_() = *another.get_first_canary_ptr_();
            *get_first_canary_ptr_() = *another.get_second_canary_ptr_();
            *get_third_canary_ptr_() = *another.get_third_canary_ptr_();

            *get_check_sum_ptr_() = *another.get_check_sum_ptr_();

        for (size_t i = 0; i < size_; ++i)
            new ((T*)(buffer_ + (sizeof(unsigned int) + i * sizeof(T)))) 
                T(*((T*)(another.buffer_ + (sizeof(unsigned int) + i * sizeof(T)))));
        }

    Stack(Stack<T>&& another)
        : stack_canary_begin(another.stack_canary_begin), 
          capacity_(another.capacity_), size_(another.size_),
          stack_canary_end(another.stack_canary_end) {
            buffer_ = another.buffer_;
            another.buffer_ = nullptr;
        }

    ~Stack() {
        for (size_t i = 0; i < size_; ++i)
            ((T*)(buffer_ + (sizeof(unsigned int) + i * sizeof(T))))->~T();
        
        delete[] buffer_;
    }

    Stack& operator=(const Stack& another) = delete;
    Stack& operator=(Stack&& another) = delete;

    bool ok() const {
        return this &&
               buffer_ &&
               size_ >= 0 && capacity_ >= INITIAL_CAPACITY__ &&
               capacity_ <= MAX_CAPACITY__ &&
               size_ <= capacity_ &&
               stack_canary_begin == TRUE_CANARY__ &&
               stack_canary_end == TRUE_CANARY__ &&
               get_third_canary_ptr_() &&
               get_second_canary_ptr_() &&
               get_third_canary_ptr_() &&
               get_check_sum_ptr_() &&
               *get_third_canary_ptr_() == TRUE_CANARY__ &&
               *get_second_canary_ptr_() == TRUE_CANARY__ &&
               *get_first_canary_ptr_() == TRUE_CANARY__ &&
               *get_check_sum_ptr_() == sum_up_();
    }

    void dump(FILE* file = nullptr) const {
        if (!file)
            file = fopen("stack_dump", "w");

        fprintf(file, "Stack at %p (%s) {\n    stack_canary_begin (%p) = 0x%x (%s)\n    capacity_(%p) = %zu (%s)\n    size_(%p) = %zu (%s)\n\
    buffer_ (%p) = {\n        first_canary (%p) = 0x%x (%s)\n", 
            this, ok() ? "OK" : "ERROR", 
            &stack_canary_begin, stack_canary_begin, stack_canary_begin == TRUE_CANARY__ ? "OK" : "ERROR",
            &capacity_, capacity_, capacity_ >= INITIAL_CAPACITY__ && capacity_ <= MAX_CAPACITY__ ? "OK" : "ERROR",
            &size_, size_, size_ >= 0 && size_ <= capacity_ ? "OK" : "ERROR",
            buffer_,
            get_first_canary_ptr_(), *get_first_canary_ptr_(), *get_first_canary_ptr_() == TRUE_CANARY__ ? "OK" : "ERROR"
            );

        for (size_t i = 0; i < size_; ++i) {
            fprintf(file, "        [%zu] = {\n==================================\n", i);
            T* ptr = (T*)(buffer_ + (sizeof(unsigned int) + i * sizeof(T)));
            if (!ptr)
                fprintf(file, "NULL\n");
            dump_(*ptr, file);
            fprintf(file, "\n==================================\n        }\n");
        }
        fprintf(file, "        second_canary (%p) = 0x%x (%s)\n        check_sum (%p) = %" PRIu64 " (%s)\n        third_canary (%p) = 0x%x (%s)\n    }\n\
    stack_canary_end (%p) = 0x%x (%s)\n}", 
            get_second_canary_ptr_(), *get_second_canary_ptr_(), *get_second_canary_ptr_() == TRUE_CANARY__ ? "OK" : "ERROR",
            get_check_sum_ptr_(), *get_check_sum_ptr_(), *get_check_sum_ptr_() == sum_up_() ? "OK" : "ERROR",
            get_third_canary_ptr_(), *get_third_canary_ptr_(), *get_third_canary_ptr_() == TRUE_CANARY__ ? "OK" : "ERROR",
            &stack_canary_end, stack_canary_end, stack_canary_end == TRUE_CANARY__ ? "OK" : "ERROR"
            );

    }

    void push(const T& item) {
        ASSERT_OK();
        
        resize_if_necessary_();

        new ((T*)(buffer_ + (sizeof(unsigned int) + size_++ * sizeof(T)))) T(item);
        *get_check_sum_ptr_() = sum_up_();

        ASSERT_OK();
    }

    const T pop() {
        assert(!empty() && "Try to pop element from empty stack!!!");

        ASSERT_OK();

        resize_if_necessary_();

        T* copy_ptr = (T*)(buffer_ + (sizeof(unsigned int) + --size_ * sizeof(T)));
        T copy = *copy_ptr;
        copy_ptr->~T();
        *get_check_sum_ptr_() = sum_up_();
        
        ASSERT_OK();

        return copy;
    }

    size_t size() const {
        ASSERT_OK();
        return size_;
    }

    bool empty() const {
        return !size_;
    } 

    bool operator!() const {
        return !ok();
    }
};
