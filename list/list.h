#include <vector>
#include <array>
#include <iterator>
#include <cassert>


template <typename T>
class List {
    friend class ListIterator;

private:
    const static size_t __BLOCK_SIZE__ = 100;
    const static size_t __POISON__ = 666;
    using Node = std::array<size_t, 3>;
    using Block = std::array<Node, __BLOCK_SIZE__>;


    std::vector<Block> buffer_;
    size_t size_;
    size_t first_free_index_;
    size_t head_index_;


    Node& node_at_(size_t index) {
        return buffer_[index / __BLOCK_SIZE__][index % __BLOCK_SIZE__];
    }

    size_t& data_ptr_(Node node) {
        return node[0];
    }

    size_t& next_index_(Node node) {
        return node[2];
    }

    size_t& prev_index_(Node node) {
        return node[1];
    }

    bool poison_(Node node) const {
        return node[0] == __POISON__;
    }


    void init_block_(Block& block) {
        data_ptr_(block[0]) = __POISON__;
        for (size_t i = 1; i < __BLOCK_SIZE__; ++i) {
            data_ptr_(block[i]) = __POISON__;
            next_index_(block[i]) = i + 1;
            prev_index_(block[i]) = i - 1;
        }
        next_index_(block[__BLOCK_SIZE__ - 1]) = 0;
    }


    template <typename U>
    void insert_(size_t before_which_index, U&& what) {
        if (!first_free_index_) {
            buffer_.push_back(Block());
            init_block_(buffer_.back());
            first_free_index_ = (buffer_.size() - 1) * __BLOCK_SIZE__;
        }


        Node& new_node = node_at_(first_free_index_);
        Node& before_which = node_at_(before_which_index);
        Node& prev = node_at_(prev_index_(before_which));


        if (poison_(prev))
            head_index_ = first_free_index_;


        data_ptr_(new_node) = (size_t)(new T(std::forward(what)));
        next_index_(new_node) = before_which_index;
        prev_index_(new_node) = poison_(prev) ? 0 : prev_index_(before_which);
        
        if (!poison_(prev))
            next_index_(prev) = first_free_index_;
        
        if (!poison_(before_which))
            prev_index_(before_which) = first_free_index_;
        

        first_free_index_ = next_index_(first_free_index_);


        ++size_;
    }


    T remove_(size_t index) {
        Node& removed = node_at_(index);
        

        T* removed_data_ptr = (T*)data_ptr_(removed);
        T removed_value = *removed_data_ptr;
        removed_data_ptr->~T();
        data_ptr_(removed) = __POISON__;


        Node& prev = node_at_(prev_index_(removed));
        Node& next = node_at_(next_index_(removed));

        if (!poison_(prev))
            next_index_(prev) = poison_(next) ? 0 : next_index_(removed);

        if (!poison_(next))
            prev_index_(next) = poison_(prev) ? 0 : prev_index_(removed);


        next_index_(removed) = first_free_index_;
        first_free_index_ = index;


        --size_;

        return removed_value;
    }


public:
    List()
        : buffer_(1), size_(0), first_free_index_(1), head_index_(0) {
            init_block_(buffer_[0]);
        }


    ~List() {
        for (Block& block : buffer_)
            for (Node& node : block)
                if (!poison_(node))
                    ((T*)data_ptr_(node))->~T();
    }


    List(const List& another)
        : buffer_(another.buffer_.size()), size_(another.size()),
          first_free_index_(another.first_free_index_), head_index_(another.head_index_) {
            for (size_t i = 0; i < buffer_.size(); ++i)
                for (size_t j = 0; j < __BLOCK_SIZE__; ++j) {
                    buffer_[i][j][1] = another.buffer_[i][j][1];
                    buffer_[i][j][2] = another.buffer_[i][j][2]; 
                    buffer_[i][j][0] = poison_(another.buffer_[i][j]) ? __POISON__ 
                                        : (size_t)(new T(*((T*)(another.buffer_[i][j][0]))));
                }
        }


    List(List&& another)
        : buffer_(another.buffer_.size()), size_(another.size()),
          first_free_index_(another.first_free_index_), head_index_(another.head_index_) {
            for (size_t i = 0; i < buffer_.size(); ++i)
                for (size_t j = 0; j < __BLOCK_SIZE__; ++j) {
                    buffer_[i][j][1] = another.buffer_[i][j][1];
                    buffer_[i][j][2] = another.buffer_[i][j][2]; 
                    buffer_[i][j][0] = poison_(another.buffer_[i][j]) ? __POISON__ 
                                        : (size_t)(new T(std::move(*((T*)(another.buffer_[i][j][0])))));
                }
        }



    void insert(size_t before_which_index, const T& what) {
        insert_(before_which_index + 1, what);
    }

    void insert(size_t before_which_index, T&& what) {
        insert_(before_which_index + 1, what);
    }


    T& operator[](size_t index) {
        assert(index < __BLOCK_SIZE__ * buffer_.size() - 1);
        return *data_ptr_(node_at_(index + 1));
    }

    T operator[](size_t index) const {
        assert(index < __BLOCK_SIZE__ * buffer_.size() - 1);
        return *data_ptr_(node_at_(index + 1));        
    }


    T remove(size_t index) {
        return remove_(index + 1);
    }


    size_t size() const {
        return size_;
    }


    void dump(FILE* file = nullptr) const {                                                             //TODO
        if (!file)
            file = fopen("list_dump", "w");
    }


    template <typename ValueType>
    class ListIterator : public std::iterator<std::bidirectional_iterator_tag,
                                              T, 
                                              std::ptrdiff_t, 
                                              ValueType*, 
                                              ValueType&> {
    private:
        List<ValueType>* list_ptr_;
        size_t index_;


    public:
        ListIterator(List<ValueType>* list_ptr, size_t index)
            : list_ptr_(list_ptr), index_(index) {}


        ListIterator()
            : list_ptr_(nullptr) {}


        ListIterator(const ListIterator&) = default;


        ~ListIterator() {}


        bool operator==(const ListIterator& another) const {
            return list_ptr_ == another.list_ptr_ && index_ == another.index_;
        }


        bool operator!=(const ListIterator& another) const {
            return !(*this == another);
        }


        ListIterator& operator++() {
            index_ = next_index_(list_ptr_->node_at_(index_));
            return *this;
        }


        ListIterator& operator++(int) const {
            ListIterator copy(*this);
            ++*this;
            return copy;
        }


        ListIterator& operator--() {
            index_ = prev_index_(list_ptr_->node_at_(index_));
            return *this;
        }


        ListIterator& operator--(int) const {
            ListIterator copy(*this);
            --*this;
            return copy;
        }


        ValueType* operator->() const {
            return data_ptr_(list_ptr_->node_at_(index_));
        }


        ValueType& operator*() const {
            return *data_ptr_(list_ptr_->node_at_(index_));
        }
    };


    using iterator = ListIterator<T>;
    using const_iterator = ListIterator<const T>;
    using reverse_iterator = std::reverse_iterator<ListIterator<T>>;
    using const_reverse_iterator = std::reverse_iterator<ListIterator<const T>>;


    iterator begin() {
        return iterator(this, head_index_);
    }


    iterator end() {
        return iterator(this, 0);
    }


    const_iterator cbegin() const {
        return const_iterator(this, head_index_);
    }


    const_iterator cend() const {
        return const_iterator(this, 0);
    }


    reverse_iterator rbegin() {
        return iterator(this, head_index_);
    }


    reverse_iterator rend() {
        return iterator(this, 0);
    }


    const_reverse_iterator crbegin() const {
        return const_iterator(this, head_index_);
    }


    const_reverse_iterator crend() const {
        return const_iterator(this, 0);
    }
};
