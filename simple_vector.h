#pragma once

#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <utility>

class ReserveProxyObj {
public:
    ReserveProxyObj() noexcept = default;

    ReserveProxyObj(size_t capacity_to_reserve)
        : capacity_to_reserve_(capacity_to_reserve) { }

public:
    size_t GetCapacity() {
        return capacity_to_reserve_;
    }

private:
    size_t capacity_to_reserve_ = 0;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) : vec_(size), size_(size), capacity_(size) { }

    SimpleVector(size_t size, const Type& value) : vec_(size), size_(size), capacity_(size) {
        std::fill(vec_.Get(), vec_.Get() + size_, value);
    }

    SimpleVector(std::initializer_list<Type> init) : vec_(init.size()), size_(init.size()), capacity_(size_) {
        std::copy(init.begin(), init.end(), &vec_[0]);
    }

    SimpleVector(ReserveProxyObj obj) {
        capacity_ = obj.GetCapacity();
    }

    SimpleVector(const SimpleVector& other)
        : vec_(other.size_)
        , size_(other.size_)
        , capacity_(other.size_) {
        std::copy(other.begin(), other.end(), vec_.Get());
    }

    SimpleVector(SimpleVector&& other) :
        vec_(std::move(other.vec_)),
		size_(std::exchange(other.size_, 0)),
		capacity_(std::exchange(other.capacity_, 0)) { }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (rhs != *this) {
            SimpleVector<Type> temp(rhs.GetSize());
            std::copy(rhs.begin(), rhs.end(), &temp[0]);
            swap(temp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        if (&rhs != this) {
            vec_ = std::move(rhs.vec_);
            size_ = std::exchange(rhs.size_, 0);
        }
        return *this;
    }

    Type& operator[](size_t index) noexcept {
        return vec_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return vec_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) throw std::out_of_range("out_of_range");
        return vec_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) throw std::out_of_range("out_of_range");
        return vec_[index];
    }

public:
    static void FillWithDefaultValue(Type* from, Type* to) {
        while (from != to) {
            *from = Type{};
            ++from;
        }
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            auto cur_size = size_;
            SimpleVector<Type> temp(new_capacity);
            std::move(begin(), end(), &temp[0]);
            swap(temp);
            size_ = cur_size;
            capacity_ = new_capacity;
        }
    }

    void PushBack(const Type& item) {
        if (capacity_ == 0) {
            Reserve(1);
            vec_[0] = item;
        }
        else {
            if (size_ < capacity_) {
                vec_[size_] = item;
            }
            else {
                Reserve(size_ * 2);
                vec_[size_] = item;
            }
        }

        ++size_;
    }

    void PushBack(Type&& item) {
        if (capacity_ == 0) {
            Reserve(1);
            vec_[0] = std::move(item);
        }
        else {
            if (size_ < capacity_) {
                vec_[size_] = std::move(item);
            }
            else {
                Reserve(size_ * 2);
                vec_[size_] = std::move(item);
            }
        }

        ++size_;
    }

    void PopBack() noexcept {
        if (!IsEmpty()) {
            --size_;
        }
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t pos_distance = std::distance(cbegin(), pos);

        if (capacity_ == 0) {
            Reserve(1);
        }
        else {
            if (size_ < capacity_) {
                std::move_backward(&vec_[pos_distance], end(), &vec_[size_ + 1]);
            }
            else {
                Reserve(size_ * 2);
                std::move_backward(&vec_[pos_distance], end(), &vec_[size_ + 1]);
            }
        }

        vec_[pos_distance] = value;
        ++size_;
        return &vec_[pos_distance];
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        size_t pos_distance = std::distance(cbegin(), pos);

        if (capacity_ == 0) {
            Reserve(1);
        }
        else {
            if (size_ < capacity_) {
                std::move_backward(&vec_[pos_distance], end(), &vec_[size_ + 1]);
            }
            else {
                Reserve(size_ * 2);
                std::move_backward(&vec_[pos_distance], end(), &vec_[size_ + 1]);
            }
        }

        vec_[pos_distance] = std::move(value);
        ++size_;
        return &vec_[pos_distance];
    }

    Iterator Erase(ConstIterator pos) {
        auto pos_it = std::next(begin(), std::distance(cbegin(), pos));
        std::move(pos_it + 1, end(), pos_it);
        --size_;
        return pos_it;
    }

    void swap(SimpleVector& other) noexcept {
        vec_.swap(other.vec_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }
        else {
            if (new_size <= capacity_) {
                FillWithDefaultValue(vec_.Get() + size_, vec_.Get() + new_size);
                size_ = new_size;
            }
            else {
                //new_size > capacity_
                Reserve(new_size);
                FillWithDefaultValue(vec_.Get() + size_, vec_.Get() + new_size);
                size_ = new_size;
            }
        }
    }

    Iterator begin() noexcept {
        return vec_.Get();
    }

    Iterator end() noexcept {
        return vec_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return vec_.Get();
    }

    ConstIterator end() const noexcept {
        return vec_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return vec_.Get();
    }

    ConstIterator cend() const noexcept {
        return vec_.Get() + size_;
    }

private:
    ArrayPtr<Type> vec_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs.GetSize() == rhs.GetSize() &&
        std::equal(lhs.begin(), lhs.end(), rhs.begin()));
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.begin() + lhs.GetSize(), rhs.begin(), rhs.begin() + rhs.GetSize());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs == rhs) || (lhs < rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs <= rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}
