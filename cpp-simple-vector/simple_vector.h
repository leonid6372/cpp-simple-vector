#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>

#include "array_ptr.h"

class ReserveProxyObj {
    public:
        ReserveProxyObj(size_t capacity_to_reserve){
            reserve_size = capacity_to_reserve;
        }
        size_t reserve_size;
    };

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    
    SimpleVector(ReserveProxyObj obj){
        if(obj.reserve_size > capacity_){
            ArrayPtr<Type> temp(obj.reserve_size);
            std::copy(begin(), end(), temp.Get());
            items_.swap(temp);
            capacity_ = obj.reserve_size;
        }
    }
    
    void Reserve(size_t new_size){
        if(new_size > capacity_){
            ArrayPtr<Type> temp(new_size);
            std::copy(begin(), end(), temp.Get());
            items_.swap(temp);
            capacity_ = new_size;
        }
    }

    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) 
        : items_(size)
        , size_(size)
        , capacity_(size) 
    {
        std::fill(items_.Get(), items_.Get() + size_, Type());
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : items_(size)
        , size_(size)
        , capacity_(size) 
    {
        std::fill(items_.Get(), items_.Get() + size_, value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : items_(init.size())
        , size_(init.size())
        , capacity_(init.size()) 
    {
        std::copy(init.begin(), init.end(), items_.Get());
    }
    
    SimpleVector(const SimpleVector& other)
        : items_(other.size_)
        , size_(other.size_)
        , capacity_(other.size_)
    {
        std::copy(other.items_.Get(), other.items_.Get() + other.size_, items_.Get());
    }
    
    SimpleVector(SimpleVector&& other)
        : items_(nullptr)
        , size_(0)
        , capacity_(0)
    {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
    
    SimpleVector& operator=(const SimpleVector& rhs) {
        SimpleVector temp(rhs);
        items_.swap(temp.items_);
        std::swap(size_, temp.size_);
        std::swap(capacity_, temp.size_);
        return *this;
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
        return size_ == 0 ? true : false;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if(index >= size_){
            throw std::out_of_range("");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if(index >= size_){
            throw std::out_of_range("");
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if(new_size <= size_){
            size_ = new_size;
        } else {
            new_size = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> to_copy(new_size);
            std::move(std::make_move_iterator(items_.Get()), std::make_move_iterator(items_.Get() + size_), to_copy.Get());
            for(size_t i = size_; i != new_size; ++i){
                to_copy[i] = std::move(Type{});
            }
            items_.swap(to_copy);
            size_ = new_size;
            capacity_ = new_size;
        }
    }
    
    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if(capacity_ == 0){
            ArrayPtr<Type> temp(1);
            items_.swap(temp);
            capacity_ = 1;
        }
        const size_t index = size_;
        if(size_ < capacity_){
            items_[index] = std::move(const_cast<Type&&>(item));
            ++size_;
        } else {
            size_t new_size = capacity_ * 2;
            Resize(new_size);
            size_ = index + 1;
            items_[index] = std::move(const_cast<Type&&>(item));
        }
    }
    
    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t new_size;
		if(size_ < capacity_){
			new_size = size_ + 1;
		} else {
			new_size = capacity_ == 0 ? 1 : capacity_ * 2;
            capacity_ = new_size;
		}
		ArrayPtr<Type> temp(new_size);
        std::move(std::make_move_iterator(items_.Get()), std::make_move_iterator(const_cast<Iterator>(pos)), temp.Get());
        auto index = std::distance(items_.Get(), const_cast<Iterator>(pos));
        temp[index] = std::move(const_cast<Type&&>(value));
        std::move_backward(std::make_move_iterator(const_cast<Iterator>(pos)), std::make_move_iterator(items_.Get() + size_), temp.Get() + size_ + 1);
        items_.swap(temp);
        ++size_;
        return &items_[index];
    }
    
    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if(size_ != 0){
            --size_;
        }
    }
    
    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        auto index = std::distance(items_.Get(), const_cast<Iterator>(pos));
        std::move(std::make_move_iterator(const_cast<Iterator>(pos) + 1), std::make_move_iterator(items_.Get() + size_), const_cast<Iterator>(pos));
        --size_;
        return &items_[index];
    }
    
    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
    
    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }
    
private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs) && !(lhs < rhs);
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs > rhs) || (lhs < rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                        rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs) && !(lhs >= rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
} 
