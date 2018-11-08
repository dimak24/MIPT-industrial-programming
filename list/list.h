#include <vector>
#include <array>
#include <iterator>
#include <cassert>
#include <type_traits>


template <typename T>
class List {
private:
    struct Node {
        size_t next, prev;
        T data;
    };

    const static size_t __BLOCK_SIZE__ = 100;
    
    using Block = std::array<Node, __BLOCK_SIZE__>;
    
    std::vector<Block>* buffer_ptr_;
    
    size_t size_;
    size_t first_free_index_;
    size_t* head_index_;
    size_t* tail_index_;


    Node& node_at_(size_t index) {
        return (*buffer_ptr_)[index / __BLOCK_SIZE__][index % __BLOCK_SIZE__];
    }

    const Node& node_at_(size_t index) const {
        return (*buffer_ptr_)[index / __BLOCK_SIZE__][index % __BLOCK_SIZE__];
    }

    size_t& next_index_(size_t index) {
        return node_at_(index).next;
    }

    size_t next_index_(size_t index) const {
        return node_at_(index).next;
    }

    size_t& prev_index_(size_t index) {
        return node_at_(index).prev;
    }

    size_t prev_index_(size_t index) const {
        return node_at_(index).prev;
    }

    T& data_(size_t index) {
        return node_at_(index).data;
    }

    const T& data_(size_t index) const {
        return node_at_(index).data;
    }


    void init_block_(size_t block_index) {
        size_t i0 = block_index * __BLOCK_SIZE__;
        for (size_t i = 0; i < __BLOCK_SIZE__; ++i) {
            next_index_(i0 + i) = i0 + i + 1;
            prev_index_(i0 + i) = i0 + i - 1;
        }
        prev_index_(i0) = 0;
        next_index_(i0 + __BLOCK_SIZE__ - 1) = 0;
    }


    void expand_if_necessary_() {
        if (!first_free_index_) {
            buffer_ptr_->push_back(Block{});
            init_block_(buffer_ptr_->size() - 1);
            first_free_index_ = (buffer_ptr_->size() - 1) * __BLOCK_SIZE__;
        }
    }


    template <typename U>
    void push_back_(U&& what) {
        expand_if_necessary_();

        data_(first_free_index_) = T(std::forward<U>(what));

        auto next_free_index = next_index_(first_free_index_);

        next_index_(first_free_index_) = 0;
        if (*tail_index_) {
            prev_index_(first_free_index_) = *tail_index_;
            next_index_(*tail_index_) = first_free_index_;
        }
        else {
            prev_index_(first_free_index_) = 0;
            *head_index_ = first_free_index_;
        }

        *tail_index_ = first_free_index_;
        first_free_index_ = next_free_index;
        prev_index_(first_free_index_) = 0;

        ++size_;
    }


    template <typename U>
    void insert_(size_t before_which_index, U&& what) {
        if (before_which_index == 0)
            push_back_(what);

        expand_if_necessary_();

        auto next_free_index = next_index_(first_free_index_);

        if (!prev_index_(before_which_index))
            *head_index_ = first_free_index_;

        data_(first_free_index_) = T(std::forward<U>(what));
        
        next_index_(first_free_index_) = before_which_index;
        prev_index_(first_free_index_) = prev_index_(before_which_index);
        
        if (prev_index_(before_which_index))
            next_index_(prev_index_(before_which_index)) = first_free_index_;
        

        prev_index_(before_which_index) = first_free_index_;
        
        first_free_index_ = next_free_index;
        prev_index_(first_free_index_) = 0;

        ++size_;
    }


    T remove_(size_t index) {
        if (index == *head_index_)
            *head_index_ = next_index_(index);
        if (index == *tail_index_)
            *tail_index_ = prev_index_(index);

        T removed_value = data_(index);

        if (prev_index_(index))
            next_index_(prev_index_(index)) = next_index_(index);

        if (next_index_(index))
            prev_index_(next_index_(index)) = prev_index_(index);


        next_index_(index) = first_free_index_;
        prev_index_(index) = 0;
        prev_index_(first_free_index_) = index;

        first_free_index_ = index;

        --size_;

        return removed_value;
    }


    template <typename Dummy = void>
    static typename std::enable_if<std::is_class<T>::value, Dummy>::type dump_(const T& t, 
                                                                               FILE* file, 
                                                                               int indent = 0) {
        t.dump(file, indent);
    }

    static void dump_(int a, FILE* file, int indent = 0) {
        fprintf(file, "%*sint (%p) = %d\n", indent, "", &a, a);
    }

    static void dump_(long long a, FILE* file, int indent = 0) {
        fprintf(file, "%*slong long (%p) = %lld\n", indent, "", &a, a);
    }

    static void dump_(const char* a, FILE* file, int indent = 0) {
        fprintf(file, "%*sconst char* (%p) = \"%s\"\n", indent, "", a, a);
    }

    static void dump_(double a, FILE* file, int indent = 0) {
        fprintf(file, "%*sdouble (%p) = %g\n", indent, "", &a, a);
    }

    static void dump_(long unsigned a, FILE* file, int indent = 0) {
        fprintf(file, "%*slong unsigned (%p) = %lu\n", indent, "", &a, a);
    }



    template <typename ValueType>
    class ListIterator : public std::iterator<std::bidirectional_iterator_tag,
                                              T, 
                                              std::ptrdiff_t, 
                                              ValueType*, 
                                              ValueType&> {
        friend class List;
    private:
        std::vector<Block>* buffer_ptr_;
        size_t* head_index_;
        size_t* tail_index_;

        size_t index_;


    public:
        ListIterator(std::vector<Block>* buffer_ptr, size_t* head_index, size_t* tail_index, size_t index)
            : buffer_ptr_(buffer_ptr), head_index_(head_index), tail_index_(tail_index), index_(index) {}


        ListIterator()
            : buffer_ptr_(nullptr) {}


        ListIterator(const ListIterator&) = default;


        ~ListIterator() {}


        bool operator==(const ListIterator& another) const {
            return buffer_ptr_ == another.buffer_ptr_ && index_ == another.index_;
        }


        bool operator!=(const ListIterator& another) const {
            return !(*this == another);
        }


        ListIterator& operator++() {
            index_ = index_ ? (*buffer_ptr_)[index_ / __BLOCK_SIZE__][index_ % __BLOCK_SIZE__].next
                            : *head_index_;
            return *this;
        }


        ListIterator& operator++(int) const {
            ListIterator copy(*this);
            ++*this;
            return copy;
        }


        ListIterator& operator--() {
            index_ = index_ ? (*buffer_ptr_)[index_ / __BLOCK_SIZE__][index_ % __BLOCK_SIZE__].prev 
                            : *tail_index_;
            return *this;
        }


        ListIterator& operator--(int) const {
            ListIterator copy(*this);
            --*this;
            return copy;
        }


        ValueType* operator->() const {
            return &((*buffer_ptr_)[index_ / __BLOCK_SIZE__][index_ % __BLOCK_SIZE__].data);
        }


        ValueType& operator*() const {
            return (*buffer_ptr_)[index_ / __BLOCK_SIZE__][index_ % __BLOCK_SIZE__].data;
        }
    };


public:
    List()
        : buffer_ptr_(new std::vector<Block>(1)), size_(0), 
          first_free_index_(1), head_index_(new size_t(0)), tail_index_(new size_t(0)) {
            init_block_(0);
            next_index_(0) = 0;
        }


    ~List() {
        delete buffer_ptr_;
        delete tail_index_;
        delete head_index_;
    }


    List(const List& another)
        : buffer_ptr_(new std::vector<Block>()), size_(another.size()),
          first_free_index_(another.first_free_index_),
          head_index_(new size_t(*another.head_index_)), tail_index_(new size_t(*another.tail_index_)) {
            for (size_t i = 0; i < another.buffer_ptr_->size(); ++i)
                buffer_ptr_->push_back((*another.buffer_ptr_)[i]);
          }

    List(List&& another)
        : buffer_ptr_(new std::vector<Block>()), size_(another.size()),
          first_free_index_(another.first_free_index_),
          head_index_(new size_t(*another.head_index_)), tail_index_(new size_t(*another.tail_index_)) {
            for (size_t i = 0; i < another.buffer_ptr_->size(); ++i)
                buffer_ptr_->push_back(std::move((*another.buffer_ptr_)[i]));
          }


    void insert(size_t before_which_index, const T& what) {
        insert_(before_which_index + 1, what);
    }

    void insert(size_t before_which_index, T&& what) {
        insert_(before_which_index + 1, std::move(what));
    }


    T& operator[](size_t index) {
        assert(index < __BLOCK_SIZE__ * buffer_ptr_->size() - 1);
        return data_(index + 1);
    }

    T operator[](size_t index) const {
        assert(index < __BLOCK_SIZE__ * buffer_ptr_->size() - 1);
        return data_(index + 1);      
    }


    T remove(size_t index) {
        return remove_(index + 1);
    }


    void push_back(const T& item) {
        push_back_(item);
    }

    void push_back(T&& item) {
        push_back_(std::move(item));
    }


    void pop_back() {
        remove_(*tail_index_);
    }


    void push_front(const T& item) {
        insert_(*head_index_, item);
    }

    void push_front(T&& item) {
        insert_(*head_index_, std::move(item));
    }


    void pop_front() {
        remove_(*head_index_);
    }


    T& back() {
        assert(size());
        return data_(*tail_index_);
    }

    const T& back() const {
        assert(size());
        return data_(*tail_index_);
    }


    T& front() {
        assert(size());
        return data_(*head_index_);
    }

    const T& front() const {
        assert(size());
        return data_(*head_index_);
    }


    template <typename... Args>
    void emplace_back(Args&&... args) {
        expand_if_necessary_();

        data_(first_free_index_) = T(std::forward<Args>(args)...);
        
        auto next_free_index = next_index_(first_free_index_);
        next_index_(first_free_index_) = 0;

        if (*tail_index_) {
            prev_index_(first_free_index_) = *tail_index_;
            next_index_(*tail_index_) = first_free_index_;
        }
        else {
            prev_index_(first_free_index_) = 0;
            *head_index_ = first_free_index_;
        }

        *tail_index_ = first_free_index_;
        first_free_index_ = next_free_index;
        prev_index_(first_free_index_) = 0;

        ++size_;
    }


    template <typename... Args>
    void emplace_front(Args&&... args) {
        expand_if_necessary_();

        data_(first_free_index_) = T(std::forward<Args>(args)...);
        
        auto next_free_index = next_index_(first_free_index_);
        prev_index_(first_free_index_) = 0;
        
        if (*head_index_) {
            next_index_(first_free_index_) = *head_index_;
            prev_index_(*head_index_) = first_free_index_;
        }
        else {
            next_index_(first_free_index_) = 0;
            *tail_index_ = first_free_index_;
        }

        *head_index_ = first_free_index_;
        first_free_index_ = next_free_index;
        prev_index_(first_free_index_) = 0;

        ++size_;
    }


    size_t size() const {
        return size_;
    }


    bool empty() const {
        return size_ == 0;
    }



    using iterator = ListIterator<T>;
    using const_iterator = ListIterator<const T>;
    using reverse_iterator = std::reverse_iterator<ListIterator<T>>;
    using const_reverse_iterator = std::reverse_iterator<ListIterator<const T>>;


    iterator begin() {
        return iterator(buffer_ptr_, head_index_, tail_index_, *head_index_);
    }


    iterator end() {
        return iterator(buffer_ptr_, head_index_, tail_index_, 0);
    }


    const_iterator cbegin() const {
        return const_iterator(buffer_ptr_, head_index_, tail_index_, *head_index_);
    }


    const_iterator cend() const {
        return const_iterator(buffer_ptr_, head_index_, tail_index_, 0);
    }


    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }


    reverse_iterator rend() {
        return reverse_iterator(begin());
    }


    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }


    const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }


    iterator insert(iterator it, const T& item) {
        insert_(it.index_, item);
        return --it;
    }

    iterator insert(iterator it, T&& item) {
        insert_(it.index_, std::move(item));
        return --it;
    }

    iterator insert(const_iterator it, const T& item) {
        insert_(it.index_, item);
        return --it;
    }

    iterator insert(const_iterator it, T&& item) {
        insert_(it.index_, std::move(item));
        return --it;
    }


    iterator remove(iterator it) {
        iterator next = ++it;
        remove_((--it).index_);
        return next;
    }

    iterator remove(const_iterator it) {
        iterator next = ++it;
        remove_((--it).index_);
        return next;
    }


    void dump(FILE* file = nullptr) const {
        if (!file)
            file = fopen("list_dump", "w");

        fprintf(file, "List at %p = {\n"
                      "  size_ (%p) = %zu\n"
                      "  head_index_ (%p) = %zu\n"
                      "  tail_index_ (%p) = %zu\n"
                      "  first_free_index_ (%p) = %zu\n"
                      "  buffer_ptr_ = %p\n", this,
                &size_, size_, head_index_, *head_index_, tail_index_, *tail_index_,
                &first_free_index_, first_free_index_, buffer_ptr_);

        fprintf(file, "\n  Used nodes: {\n");
        for (size_t index = *head_index_; index; index = next_index_(index)) {
            fprintf(file, "    Node at %p with index %zu {\n"
                          "      next = %zu\n"
                          "      prev = %zu\n"
                          "      data = {\n", 
                    &node_at_(index), index, next_index_(index), prev_index_(index));
            
            dump_(data_(index), file, 8);
            fprintf(file, "      }\n    }\n");
        }

        fprintf(file, "  }\n  Trash nodes: {\n");
        for (size_t index = first_free_index_; index; index = next_index_(index)) {
            fprintf(file, "    Node at %p with index %zu {\n"
                          "      next = %zu\n"
                          "      prev = %zu\n    }", 
                    &node_at_(index), index, next_index_(index), prev_index_(index));
            
            fprintf(file, "\n");
        }

        fprintf(file, "  }\n}\n");

    }


    //sorts nodes in list by the distance to the head, so using indices is __fast__ and __correct__
    void pull() {
        std::vector<Block>* new_buffer_ptr = new std::vector<Block>(buffer_ptr_->size());
        size_t i = 1;
        for (size_t index = *head_index_; index; index = next_index_(index), ++i)
            (*new_buffer_ptr)[i / __BLOCK_SIZE__][i % __BLOCK_SIZE__].data = std::move(node_at_(index).data);

        for (size_t index = first_free_index_; index; index = next_index_(index), ++i)
            (*new_buffer_ptr)[i / __BLOCK_SIZE__][i % __BLOCK_SIZE__].data = std::move(node_at_(index).data);

        *head_index_ = 1;
        *tail_index_ = size();
        first_free_index_ = size() + 1;

        delete buffer_ptr_;
        buffer_ptr_ = new_buffer_ptr;

        for (size_t i = 0; i < buffer_ptr_->size(); ++i) {
            init_block_(i);
            if (i)
                prev_index_(i * __BLOCK_SIZE__) = i * __BLOCK_SIZE__ - 1;
            if (i != buffer_ptr_->size() - 1)
                next_index_((i + 1) * __BLOCK_SIZE__ - 1) = (i + 1) * __BLOCK_SIZE__;
        }

        next_index_(0) = 0;
        prev_index_(*head_index_) = 0;
        next_index_(*tail_index_) = 0;
        prev_index_(first_free_index_) = 0;
    }
};
