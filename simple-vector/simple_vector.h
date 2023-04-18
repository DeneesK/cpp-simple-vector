#pragma once

#include "array_ptr.h"

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <utility>

class ReserveProxyObj
{
public:
    ReserveProxyObj(size_t capacity_to_reserve): capacity_to_reserve_(capacity_to_reserve) {
    }

    size_t capacity() const { 
        return capacity_to_reserve_;
    }
private:
    size_t capacity_to_reserve_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size): SimpleVector(size, Type()) {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value): ptr_(size), size_(size), capacity_(size) {
        std::fill(begin(), end(), value);
    }

    SimpleVector(size_t size, Type&& value): ptr_(size), size_(size), capacity_(size) {
        std::fill(begin(), end(), std::move(value));
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init): SimpleVector(init.size()) {
        std::copy(init.begin(), init.end(), begin());       
    }

    SimpleVector(const SimpleVector& other): SimpleVector(other.GetSize()) {
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector&& other) {
        ptr_.swap(other.ptr_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    SimpleVector(ReserveProxyObj obj) {
        ArrayPtr<Type> arr(obj.capacity());
        ptr_.swap(arr);
        capacity_ = obj.capacity();
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if(*this == rhs) {
            return *this;
        }
        SimpleVector<Type> new_vec(rhs);
        swap(new_vec);
        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        // Напишите тело самостоятельно
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        // Напишите тело самостоятельно
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(size_ != 0);
        return ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(size_ != 0);
        return ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if(index >= size_) {
            throw std::out_of_range("");
        }
        return ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if(index >= size_) {
            throw std::out_of_range("");
        }
        return ptr_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
            return;
        }
        if (new_size <= capacity_) {
            // std::fill(begin()+size_, begin()+new_size-1, std::move(Type()));
            for(size_t i = size_; i < new_size; ++i) {
                ptr_[i] = std::move(Type());
            }
            size_ = new_size;
            return;
        }

        size_t new_cap = std::max(new_size, capacity_ * 2);
        ArrayPtr<Type> new_items(new_cap);
        std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_items.Get());
        ptr_.swap(new_items);
        capacity_ = new_cap;
        // std::fill(begin()+size_, begin()+(capacity_ - 1), Type());
            for(size_t i =size_; i < capacity_; ++i) {
                ptr_[i] = std::move(Type());
            }
        size_ = new_size;

    }

    void Reserve(size_t new_capacity) {
        if (capacity_ >= new_capacity) {
            return;
        }
        ArrayPtr<Type> new_items(new_capacity);
        std::copy(begin(), end(), new_items.Get());
        ptr_.swap(new_items);
        capacity_ = new_capacity;
    } 

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return Iterator(ptr_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return Iterator(ptr_.Get() + size_);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return ConstIterator(ptr_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return ConstIterator(ptr_.Get() + size_);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return ConstIterator(ptr_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return ConstIterator(ptr_.Get() + size_);
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if(size_ < capacity_) {
            ptr_[size_] = item;
            ++size_;
            return;
        }

        capacity_ = std::max(capacity_ * 2, static_cast<size_t>(1));
        SimpleVector new_items(capacity_);
        std::copy(begin(), end(), new_items.begin());
        new_items.size_ = size_;
        swap(new_items);
        ptr_[size_] = item;
        ++size_;
    }

    void PushBack(Type&& item) {
        if(size_ < capacity_) {
            ptr_[size_] = std::move(item);
            ++size_;
            return;
        }
        capacity_ = std::max(capacity_ * 2, static_cast<size_t>(1));
        ArrayPtr<Type> new_items(capacity_);
        std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_items.Get());
        ptr_.swap(new_items);
        ptr_[size_] = std::move(item);
        ++size_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(begin() <= pos && pos <= end());
        auto n = std::distance(begin(), Iterator(pos));
        if(size_ < capacity_) {
            std::copy_backward(Iterator(pos), end(), end());
            ptr_[n] = value;
            ++size_;
            return begin() + n;
        }
        size_t old_size= size_;
        size_t new_cap = std::max(capacity_ * 2, static_cast<size_t>(1));
        Resize(new_cap);
        size_ = old_size;
        std::copy_backward(begin() + n, end(), end()+1);
        ptr_[n] = value;
        ++size_;
        return begin() + n;
    }

    Iterator Insert(ConstIterator pos, Type &&value) {
        assert(begin() <= pos && pos <= end());
        auto n = std::distance(begin(), Iterator(pos));
        if(size_ < capacity_) {
            std::copy_backward(std::make_move_iterator(Iterator(pos)), std::make_move_iterator(end()), end());
            ptr_[n] = std::move(value);
            ++size_;
            return begin() + n;
        }
        size_t old_size= size_;
        size_t new_cap = std::max(capacity_ * 2, static_cast<size_t>(1));
        Resize(new_cap);
        size_ = old_size;
        std::copy_backward(std::make_move_iterator(begin() + n), std::make_move_iterator(end()), end()+1);
        ptr_[n] = std::move(value);
        ++size_;
        return begin() + n;        
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if(!IsEmpty()) {
            --size_;
            ptr_[size_] = Type();
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(begin() <= pos && pos < end());
        assert(size_ != 0);
        auto n = std::distance(begin(), Iterator(pos));
        std::copy(std::make_move_iterator(begin() + (n + 1)), std::make_move_iterator(end()), begin() + n);        
        --size_;
        return Iterator(pos);        
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        ptr_.swap(other.ptr_);
        std::swap(this->capacity_, other.capacity_);
        std::swap(this->size_, other.size_);
    }
private:
    ArrayPtr<Type> ptr_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if(lhs.GetSize() != rhs.GetSize()) {
        return false;
    }
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs == rhs || lhs < rhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs) && !(lhs < rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    // Заглушка. Напишите тело самостоятельно
    return lhs == rhs || !(lhs < rhs);
}