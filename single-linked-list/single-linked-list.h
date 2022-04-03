#pragma once

#include <cassert>
#include <cstddef>
#include <string>
#include <utility>
#include <iterator> // Содержит объявления категорий итераторов
#include <algorithm>
#include <exception>
#include <iostream>
#include <new> //bad_alloc!

template <typename Type>
class SingleLinkedList {

    //Узел списка
    struct Node {
        Node() = default;
        Node(const Type& val, Node* next)
            : value(val)
            , next_node(next) {
        }
        Type value;
        Node* next_node = nullptr;
    };

    // Шаблон класса «Базовый Итератор»
    // определяет поведение итератора на элементы односвязного списка
    // ValueType — совпадает с Type (для Iterator) либо с const Type (для ConstIterator)
    template <typename ValueType>
    class BasicIterator {
        
        // Конвертирующий конструктор итератора из указателя на узел списка
        explicit BasicIterator(Node* node) 
            : node_(node)
        {
        }

    public:
        // Объявленные ниже типы сообщают стандартной библиотеке о свойствах этого итератора
        // Категория итератора — forward iterator
        // (итератор, который поддерживает операции инкремента и многократное разыменование)
        using iterator_category = std::forward_iterator_tag;
        // Тип элементов, по которым перемещается итератор
        using value_type = Type;
        // Тип, используемый для хранения смещения между итераторами
        using difference_type = std::ptrdiff_t;
        // Тип указателя на итерируемое значение
        using pointer = ValueType*;
        // Тип ссылки на итерируемое значение
        using reference = ValueType&;

        BasicIterator() = default;

        // Конвертирующий конструктор/конструктор копирования
        // При ValueType, совпадающем с Type, играет роль копирующего конструктора
        // При ValueType, совпадающем с const Type, играет роль конвертирующего конструктора
        BasicIterator(const BasicIterator<Type>& other) noexcept 
            // var 1 : node_(new Node(other.node_->value, other.node_->next_node))
            : node_(other.node_)
        {
        }
        BasicIterator(const BasicIterator<const Type>& other) noexcept 
            //: node_(BasicIterator(*this).node_) //error!
            : node_(other.node_)
        {
        }

        // Чтобы компилятор не выдавал предупреждение об отсутствии оператора = при наличии
        // пользовательского конструктора копирования, явно объявим оператор = и
        // попросим компилятор сгенерировать его за нас
        BasicIterator& operator=(const BasicIterator& rhs) = default;

        // Оператор сравнения итераторов (в роли второго аргумента выступает константный итератор)
        // Два итератора равны, если они ссылаются на один и тот же элемент списка либо на end()
        [[nodiscard]] bool operator==(const BasicIterator<const Type>& rhs) const noexcept {
            
            
            if(this->node_ == rhs.node_) {
                return true;
            }
            else {
                return false;
            }
        }
        [[nodiscard]] bool operator!=(const BasicIterator<const Type>& rhs) const noexcept {
            return !(operator==(rhs));
        }
        [[nodiscard]] bool operator==(const BasicIterator<Type>& rhs) const noexcept {
            if(node_ == rhs.node_) {
                return true;
            }
            else {
                return false;
            }
        }
        [[nodiscard]] bool operator!=(const BasicIterator<Type>& rhs) const noexcept {
            return !(operator==(rhs));
        }

        // Оператор прединкремента. После его вызова итератор указывает на следующий элемент списка
        // Возвращает ссылку на самого себя
        // Инкремент итератора, не указывающего на существующий элемент списка, приводит к неопределённому поведению
        BasicIterator& operator++() noexcept {
            assert(node_ != nullptr);
            node_ = node_->next_node;
            return *this;
        }
        BasicIterator operator++(int) noexcept {
            auto old_value(*this); // Сохраняем прежнее значение объекта для последующего возврата
            ++(*this); // используем логику префиксной формы инкремента
            return old_value;
        }

        // Операция разыменования. Возвращает ссылку на текущий элемент
        // Вызов этого оператора у итератора, не указывающего на существующий элемент списка,
        // приводит к неопределённому поведению
        [[nodiscard]] reference operator*() const noexcept {
            assert(node_ != nullptr);
            return node_->value;
        }
        // Операция доступа к члену класса. Возвращает указатель на текущий элемент списка
        // Вызов этого оператора у итератора, не указывающего на существующий элемент списка,
        // приводит к неопределённому поведению
        [[nodiscard]] pointer operator->() const noexcept {
            assert(node_ != nullptr);
            return &(node_->value);
        }

    private:
        // Разрешаем SingleLinkedList обращаться к приватной области
        friend class SingleLinkedList;
        Node* node_ = nullptr;
    };

public:

    using Iterator = BasicIterator<Type>;
    using ConstIterator = BasicIterator<const Type>;

    // Возвращает итератор, указывающий на позицию перед первым элементом односвязного списка.
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] Iterator before_begin() noexcept {
        Node* head_ptr = &head_;
        return Iterator{head_ptr};
    }
    // Возвращает константный итератор, указывающий на позицию перед первым элементом односвязного списка.
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] ConstIterator before_begin() const noexcept {
        return cbefore_begin();
    }
    // Возвращает константный итератор, указывающий на позицию перед первым элементом односвязного списка.
    // Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
    [[nodiscard]] ConstIterator cbefore_begin() const noexcept {
        //const Node* ptr = &head_;
        
        Node* ptr = const_cast<Node*>(&(this->head_));
        return ConstIterator{ptr};
    }

    [[nodiscard]] Iterator begin() noexcept {
        // Благодаря дружбе SingleLinkedList имеет доступ к приватному конструктору своего итератора
        return Iterator{head_.next_node};
    }

    [[nodiscard]] Iterator end() noexcept {
        Iterator it = begin();
        std::advance(it, size_);
        return it;
    }
    // Константные версии begin/end для обхода списка без возможности модификации его элементов
    [[nodiscard]] ConstIterator begin() const noexcept {
        return ConstIterator{head_.next_node};
    }
    [[nodiscard]] ConstIterator end() const noexcept {
        ConstIterator it = begin();
        std::advance(it, size_);
        return it;
    }
    // Методы для удобного получения константных итераторов у неконстантного контейнера
    [[nodiscard]] ConstIterator cbegin() const noexcept {
        return begin();
    }
    [[nodiscard]] ConstIterator cend() const noexcept {
        return end();
    }
    /*
     * Вставляет элемент value после элемента, на который указывает pos.
     * Возвращает итератор на вставленный элемент
     * Если при создании элемента будет выброшено исключение, список останется в прежнем состоянии
     */
    Iterator InsertAfter(ConstIterator pos, const Type& value) {
        assert(pos.node_ != nullptr);
        Node* new_node = new Node(value, pos.node_->next_node);
        pos.node_->next_node = new_node;
        ++size_;
        return Iterator(new_node);
    }

    void PopFront() noexcept {
        if(!IsEmpty()) {
            auto front_ptr = head_.next_node;
            head_.next_node = head_.next_node->next_node;
            delete front_ptr;
            --size_;
        }
    }

     /*
     * Удаляет элемент, следующий за pos.
     * Возвращает итератор на элемент, следующий за удалённым
     */
    Iterator EraseAfter(ConstIterator pos) noexcept {
        assert(pos.node_ != nullptr);
        const Node* delete_ptr = pos.node_->next_node;
        pos.node_->next_node = delete_ptr->next_node;
        delete delete_ptr;
        --size_;
        return Iterator(pos.node_->next_node);
    }


    SingleLinkedList()
        : head_(*(new Node()))
        , size_(0) {
    }

    //Конструктор из std::iitializer_list
    SingleLinkedList(std::initializer_list<Type> values) {
        Node** prev_ptr = &head_.next_node;
        for(const auto& value : values) {
            auto node_ptr = new Node(value, nullptr);
            *prev_ptr = node_ptr;
            prev_ptr = &(node_ptr->next_node);
        }
        size_ = values.size();
    }

    //копирующий конструктор
    SingleLinkedList(const SingleLinkedList& other) {
        // Сначала надо удостовериться, что текущий список пуст
        assert(size_ == 0 && head_.next_node == nullptr);

        SingleLinkedList tmp;
        /* скопировать внутрь tmp элементы other */
        Node** prev_ptr = &(tmp.head_.next_node);
        for(auto node_ptr : other) {
            auto copy_node_ptr = new Node(node_ptr, nullptr);
            *prev_ptr = copy_node_ptr;
            prev_ptr = &(copy_node_ptr->next_node);
        }
        tmp.size_ = other.size_;
        swap(tmp);
    }

    ~SingleLinkedList() {
        Clear();
    }

    // Возвращает количество элементов в списке за время O(1)
    [[nodiscard]] size_t GetSize() const noexcept {
        return size_;
    }

    // Сообщает, пустой ли список за время O(1)
    [[nodiscard]] bool IsEmpty() const noexcept {
        if(size_ > 0) return false; else return true;
    }

    void PushFront(const Type& value) {
        head_.next_node = new Node(value, head_.next_node);
        ++size_;
    }

    void Clear() noexcept {
        while(!IsEmpty()) {
            //const Node* first = head_.next_node;
            Node* next = head_.next_node->next_node;
            delete head_.next_node;
            --size_;
            head_.next_node = next;
        }
        size_ = 0;
    }
    
    SingleLinkedList& operator=(const SingleLinkedList& other) {
        if(this != &other) {
            auto other_copy(other);
            swap(other_copy);
        }
        return *this;
    }

    // Обменивает содержимое списков за время O(1)    
    void swap(SingleLinkedList& other) noexcept {
        std::swap(head_.next_node, other.head_.next_node);
        std::swap(size_, other.size_);
    }

private:
    // Фиктивный узел, используется для вставки "перед первым элементом"
    Node head_;
    size_t size_ = 0;
};

template <typename Type>
void swap(SingleLinkedList<Type>& lhs, SingleLinkedList<Type>& rhs) noexcept {
    lhs.swap(rhs);
}

template <typename Type>
bool operator==(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {

    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
bool operator!=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
bool operator>(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return std::lexicographical_compare(rhs.cbegin(), rhs.cend(), lhs.cbegin(), lhs.cend());
}

template <typename Type>
bool operator<=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
bool operator>=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(lhs < rhs);
}