
#include <stdio.h>
#include <assert.h>
#include <new>
#include <stdint.h>
#include <memory>
#include <stdlib.h>
#include <inttypes.h>


#define ASSERT_OK() \
    do { \
        if (!ok_()) { \
            print_error_(); \
            abort(); \
        } \
    } while(0)


template <typename T>
class Stack {
private:
    const unsigned int TRUE_CANARY__ = 0xDEADBEEFu;
    const float GROWTH_FACTOR = 2;

    size_t capacity_;
    size_t size_;
    unsigned char* buffer_;

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

    bool ok_() {
        return *get_third_canary_ptr_() == TRUE_CANARY__ &&
               *get_second_canary_ptr_() == TRUE_CANARY__ &&
               *get_first_canary_ptr_() == TRUE_CANARY__ &&
               *get_check_sum_ptr_() == sum_up_();
    }

    void print_error_() {
        printf("     ERROR!!!!!!!\n");
        if (*get_first_canary_ptr_() != TRUE_CANARY__)
            printf("The first canary is dead!!!\n    Expected: 0x%x\n    Found: 0x%x\n",
                   TRUE_CANARY__, *get_first_canary_ptr_());

        if (*get_second_canary_ptr_() != TRUE_CANARY__)
            printf("The second canary is dead!!!\n    Expected: 0x%x\n    Found: 0x%x\n",
                   TRUE_CANARY__, *get_second_canary_ptr_());

        if (*get_third_canary_ptr_() != TRUE_CANARY__)
            printf("The third canary is dead!!!\n    Expected: 0x%x\n    Found: 0x%x\n",
                   TRUE_CANARY__, *get_third_canary_ptr_());

        if (*get_check_sum_ptr_() != sum_up_())
            printf("Check sum is wrong!!!\n    Expected: %" PRIu64 "\n    Found: %" PRIu64 "\n",
                   sum_up_(), *get_check_sum_ptr_());
    }

    void expand_if_necessary_() {
        if (size_ == capacity_) {
            uint64_t check_sum = *get_check_sum_ptr_();

            unsigned char* new_buffer = new unsigned char [(size_t)(sizeof(T) * capacity_ * GROWTH_FACTOR) + 
                                                           sizeof(uint64_t) + sizeof(unsigned int) * 3];
            std::uninitialized_move((T*)(buffer_ + sizeof(unsigned int)), 
                                    (T*)(buffer_ + sizeof(unsigned int) + sizeof(T) * capacity_),
                                    (T*)(new_buffer + sizeof(unsigned int)));
            
            std::swap(buffer_, new_buffer);
            
            capacity_ *= GROWTH_FACTOR;
            *get_second_canary_ptr_() = TRUE_CANARY__;
            *get_first_canary_ptr_() = TRUE_CANARY__;
            *get_third_canary_ptr_() = TRUE_CANARY__;

            *get_check_sum_ptr_() = check_sum;

        }
    }

public:
    Stack() 
        : capacity_(128), size_(0), 
          buffer_(new unsigned char [sizeof(T) * capacity_ + sizeof(uint64_t) + sizeof(unsigned int) * 3]) {
            *get_second_canary_ptr_() = TRUE_CANARY__;
            *get_first_canary_ptr_() = TRUE_CANARY__;
            *get_third_canary_ptr_() = TRUE_CANARY__;

            *get_check_sum_ptr_() = 0;
    }

    Stack(const Stack<T>& another)
        : capacity_(another.capacity_), size_(another.size_),
          buffer_(new unsigned char [sizeof(T) * capacity_ + sizeof(uint64_t) + sizeof(unsigned int) * 3]) {
            *get_second_canary_ptr_() = *another.get_first_canary_ptr_();
            *get_first_canary_ptr_() = *another.get_second_canary_ptr_();
            *get_third_canary_ptr_() = *another.get_third_canary_ptr_();

            *get_check_sum_ptr_() = *another.get_check_sum_ptr_();

        for (size_t i = 0; i < size_; ++i)
            new ((T*)(buffer_ + (sizeof(unsigned int) + i * sizeof(T)))) 
                T(*((T*)(another.buffer_ + (sizeof(unsigned int) + i * sizeof(T)))));
        }

    Stack(Stack<T>&& another)
        : capacity_(another.capacity_), size_(another.size_) {
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

    void push(const T& item) {
        ASSERT_OK();
        expand_if_necessary_();

        new ((T*)(buffer_ + (sizeof(unsigned int) + size_++ * sizeof(T)))) T(item);

        *get_check_sum_ptr_() = sum_up_();
    }

    const T pop() {
        assert(!empty() && "Try to pop element from empty stack!!!");

        ASSERT_OK();

        T* copy_ptr = (T*)(buffer_ + (sizeof(unsigned int) + --size_ * sizeof(T)));
        T copy = *copy_ptr;
        copy_ptr->~T();

        *get_check_sum_ptr_() = sum_up_();

        return copy;
    }

    size_t size() const {
        ASSERT_OK();
        return size_;
    }

    bool empty() const {
        return !size_;
    }
};