#pragma once
#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity_to_reserve) :capacity_to_reserve_(capacity_to_reserve)
    {
    }

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
    explicit SimpleVector(size_t size) : array_ptr_(size) {
        size_ = size;
        capacity_ = size;
        for(size_t i = 0; i < size; i++){
            array_ptr_[i] = Type();
        }
    }
    
    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : array_ptr_(size) {
        size_ = size;
        capacity_ = size;
        for(size_t i = 0; i < size; i++){
            array_ptr_[i] = value;
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) :
    array_ptr_(init.size()),
    size_(init.size()),
    capacity_(init.size()) {
        std::copy(init.begin(), init.end(), begin());
    }
    
    SimpleVector(const SimpleVector& other) {
        ArrayPtr<Type> temp(other.capacity_);
        std::copy(other.begin(), other.end(), Iterator{temp.Get()});
        array_ptr_.swap(temp);
        size_ = other.size_;
        capacity_ = other.capacity_;
    }
    
    SimpleVector(SimpleVector&& other) {
        array_ptr_.swap(other.array_ptr_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
    }
    
    SimpleVector(ReserveProxyObj capacity_to_reserve)
        :array_ptr_(capacity_to_reserve.capacity_to_reserve_),
        capacity_(capacity_to_reserve.capacity_to_reserve_)
    {
        // Reserve(capacity_to_reserve.capacity_to_reserve_);
        std::fill(begin(), end(), Type());
    }
    
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (*this != rhs){
            SimpleVector temp(rhs);
            swap(temp);
        }
        return *this;
    }
    
    SimpleVector& operator=(SimpleVector&& rhs) {
        if(this != &rhs){
           array_ptr_.swap(rhs.array_ptr_);
           std::swap(size_, rhs.size_);
           std::swap(capacity_, rhs.capacity_);
        }
        return *this;
    }
    
    void PushBack(const Type& item) {
        if (capacity_ > size_){
            array_ptr_[size_] = item;
            size_++;
        } else{
            SimpleVector<Type> temp(std::max(size_ + 1, capacity_ * 2));
            std::copy(begin(), end(), temp.begin());
            temp[size_] = item;
            size_++;
            capacity_ = temp.capacity_;
            array_ptr_.swap(temp.array_ptr_);
            return;
        }
    }
    
    void PushBack(Type&& item){
        if (capacity_ > size_){
            array_ptr_[size_] = std::move(item);
            size_++;
        } else{
            SimpleVector<Type> temp(std::max(size_ + 1, capacity_ * 2));
            std::move(array_ptr_.Get(), array_ptr_.Get() + size_, temp.array_ptr_.Get());
            temp[size_] = std::move(item);
            size_++;
            capacity_ = std::move(temp.capacity_);
            array_ptr_.swap(std::move(temp.array_ptr_));
            return;
        }
    }
    
    void PopBack() noexcept {
        if (size_ == 0) return;
        --size_;
    }
    
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        auto ds = std::distance(cbegin(), pos);
        if (capacity_ > size_){
            std::copy_backward(begin() + ds, end(), end() + 1);
            *(begin() + ds) = value;
            size_++;
        }else{
            SimpleVector temp(std::max(size_ + 1, capacity_ * 2));
            std::copy(begin(), begin() + ds, temp.begin());
            temp[ds] = value;
            std::copy(begin() + ds, end(), temp.begin() + ds + 1);
            size_++;
            capacity_ = temp.capacity_;
            array_ptr_.swap(temp.array_ptr_);
        }
        return begin() + ds;
    }
    
    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        auto ds = std::distance(cbegin(), pos);
        if (capacity_ > size_){
            std::move_backward(begin() + ds, end(), end() + 1);
            *(begin() + ds) = std::move(value);
            size_++;
        }else{
            SimpleVector temp(std::max(size_ + 1, capacity_ * 2));
            std::move(array_ptr_.Get(), array_ptr_.Get() + ds, temp.array_ptr_.Get());
            temp[ds] = std::move(value);
            std::move(array_ptr_.Get() + ds, array_ptr_.Get() + size_, temp.array_ptr_.Get() + ds + 1);
            size_++;
            capacity_ = std::move(temp.capacity_);
            array_ptr_.swap(temp.array_ptr_);
        }
        return begin() + ds;
    }
    
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        auto temp = std::distance(cbegin(), pos);
        std::move(begin() + temp + 1, end(), begin() + temp);
        --size_;
        return begin() + temp;
    }
    
    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        array_ptr_.swap(other.array_ptr_); 
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return array_ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return array_ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_){
            throw std::out_of_range("index out of range");
        }
        return array_ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_){
            throw std::out_of_range("index out of range");
        }
        return array_ptr_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if(new_size <= size_){
            size_ = std::move(new_size);
        }
        else if(new_size <= capacity_){
            std::generate(begin() + size_, begin() + new_size,[](){return Type{}; });
            size_ = std::move(new_size);
        } else{
            new_size = std::move(std::max(new_size, capacity_ * 2));
            SimpleVector<Type> temp(new_size);
            std::move(begin(), end(), temp.begin());
            array_ptr_.swap(temp.array_ptr_);
            size_ = std::move(temp.size_);
            capacity_ = std::move(temp.capacity_);
        }
    }
    
    void Reserve(size_t new_capacity){
        if (new_capacity > capacity_){
            SimpleVector temp(new_capacity);
            std::copy(begin(), end(), temp.begin());
            array_ptr_.swap(temp.array_ptr_);
            capacity_ = temp.capacity_;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return Iterator{array_ptr_.Get()};
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return Iterator{array_ptr_.Get() + size_};
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return ConstIterator{array_ptr_.Get()};
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return ConstIterator{array_ptr_.Get() + size_};
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return ConstIterator{array_ptr_.Get()};
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return ConstIterator{array_ptr_.Get() + size_};
    }
    
private:
    ArrayPtr<Type> array_ptr_;
    size_t size_ = 0;
    size_t capacity_ = 0;
    Type a;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) { 
    return !std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (rhs < lhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
} 
