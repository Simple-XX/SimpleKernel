
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://github.com/zouxiaohang/TinySTL
// List.hpp for Simple-XX/SimpleKernel.

#ifndef _LIST_HPP_
#define _LIST_HPP_

#include "Allocator.hpp"
#include "Iterator.hpp"
#include "ReverseIterator.hpp"
#include "UninitializedFunctions.hpp"

#include "type_traits"

namespace STL {
    template <class T>
    class list;
    // the class of node
    template <class T>
    struct node {
        T        data;
        node *   prev;
        node *   next;
        list<T> *container;
        node(const T &d, node *p, node *n, list<T> *c)
            : data(d), prev(p), next(n), container(c) {
        }
        bool operator==(const node &n) {
            return data == n.data && prev == n.prev && next == n.next &&
                   container == n.container;
        }
    };
    // the class of list iterator
    template <class T>
    struct listIterator : public iterator<bidirectional_iterator_tag, T> {
        template <class U>
        friend class list;

    public:
        typedef node<T> *nodePtr;
        nodePtr          p;

        explicit listIterator(nodePtr ptr = nullptr) : p(ptr) {
        }

        listIterator &operator++();
        listIterator  operator++(int);
        listIterator &operator--();
        listIterator  operator--(int);
        T &           operator*() {
            return p->data;
        }
        T *operator->() {
            return &(operator*());
        }

        template <class U>
        friend bool operator==(const listIterator<T> &lhs,
                               const listIterator<T> &rhs);
        template <class U>
        friend bool operator!=(const listIterator<T> &lhs,
                               const listIterator<T> &rhs);
    };

    template <class T>
    listIterator<T> &listIterator<T>::operator++() {
        p = p->next;
        return *this;
    }
    template <class T>
    listIterator<T> listIterator<T>::operator++(int) {
        auto res = *this;
        ++*this;
        return res;
    }
    template <class T>
    listIterator<T> &listIterator<T>::operator--() {
        p = p->prev;
        return *this;
    }
    template <class T>
    listIterator<T> listIterator<T>::operator--(int) {
        auto res = *this;
        --*this;
        return res;
    }
    template <class T>
    bool operator==(const listIterator<T> &lhs, const listIterator<T> &rhs) {
        return lhs.p == rhs.p;
    }
    template <class T>
    bool operator!=(const listIterator<T> &lhs, const listIterator<T> &rhs) {
        return !(lhs == rhs);
    }

    // the class of list
    template <class T>
    class list {
    public:
        typedef T                            value_type;
        typedef listIterator<T>              iterator;
        typedef listIterator<const T>        const_iterator;
        typedef reverse_iterator_t<iterator> reverse_iterator;
        typedef T &                          reference;
        typedef size_t                       size_type;

    private:
        template <class U>
        friend struct listIterator;
        typedef allocator<node<T>> nodeAllocator;
        typedef node<T> *          nodePtr;
        iterator                   head;
        iterator                   tail;
        void ctorAux(size_type n, const value_type &val, std::true_type);
        template <class InputIterator>
        void ctorAux(InputIterator first, InputIterator last, std::false_type);
        nodePtr newNode(const T &val = T());
        void    deleteNode(nodePtr p);
        void    insert_aux(iterator position, size_type n, const T &val,
                           std::true_type);
        template <class InputIterator>
        void           insert_aux(iterator position, InputIterator first,
                                  InputIterator last, std::false_type);
        const_iterator changeIteratorToConstIterator(iterator &it) const;

    public:
        list();
        explicit list(size_type n, const value_type &val = value_type());
        template <class InputIterator>
        list(InputIterator first, InputIterator last);
        list(const list &l);
        list &operator=(const list &l);
        ~list();

        bool empty() const {
            return head == tail;
        }
        size_type size() const;
        reference front() {
            return (head.p->data);
        }
        reference back() {
            return (tail.p->prev->data);
        }

        void push_front(const value_type &val);
        void pop_front();
        void push_back(const value_type &val);
        void pop_back();

        iterator         begin();
        iterator         end();
        const_iterator   begin() const;
        const_iterator   end() const;
        reverse_iterator rbegin();
        reverse_iterator rend();

        iterator insert(iterator position, const value_type &val);
        template <class InputIterator>
        void insert(iterator position, size_type n, const value_type &val);
        template <class InputIterator>
        void insert(iterator position, InputIterator first, InputIterator last);
        iterator erase(iterator position);
        iterator erase(iterator first, iterator last);
        void     swap(list &x);
        void     clear();
        void     splice(iterator position, list &x);
        void     splice(iterator position, list &x, iterator i);
        void splice(iterator position, list &x, iterator first, iterator last);
        void remove(const value_type &val);
        template <class Predicate>
        void remove_if(Predicate pred);
        void unique();
        template <class BinaryPredicate>
        void unique(BinaryPredicate binary_pred);
        void merge(list &x);
        template <class Compare>
        void merge(list &x, Compare comp);
        void sort();
        template <class Compare>
        void sort(Compare comp);
        void reverse();
        template <class U>
        friend void swap(list<U> &x, list<U> &y);
        template <class U>
        friend bool operator==(const list<U> &lhs, const list<U> &rhs);
        template <class U>
        friend bool operator!=(const list<U> &lhs, const list<U> &rhs);
    };

    template <class T>
    list<T>::list() {
        head.p = newNode(); // add a dummy node
        tail.p = head.p;
    }

    template <class T>
    list<T>::list(size_type n, const value_type &val) {
        ctorAux(n, val, std::is_integral<value_type>());
    }

    template <class T>
    template <class InputIterator>
    list<T>::list(InputIterator first, InputIterator last) {
        ctorAux(first, last, std::is_integral<InputIterator>());
    }

    template <class T>
    list<T>::list(const list &l) {
        head.p = newNode(); // add a dummy node
        tail.p = head.p;
        for (auto node = l.head.p; node != l.tail.p; node = node->next)
            push_back(node->data);
    }

    template <class T>
    list<T> &list<T>::operator=(const list &l) {
        if (this != &l) {
            list(l).swap(*this);
        }
        return *this;
    }

    template <class T>
    list<T>::~list() {
        for (; head != tail;) {
            auto temp = head++;
            // bug fix
            nodeAllocator::destroy(temp.p);
            nodeAllocator::deallocate(temp.p);
        }
        nodeAllocator::deallocate(tail.p);
    }

    template <class T>
    void list<T>::insert_aux(iterator position, size_type n, const T &val,
                             std::true_type) {
        for (auto i = n; i != 0; --i) {
            position = insert(position, val);
        }
    }
    template <class T>
    template <class InputIterator>
    void list<T>::insert_aux(iterator position, InputIterator first,
                             InputIterator last, std::false_type) {
        for (--last; first != last; --last) {
            position = insert(position, *last);
        }
        insert(position, *last);
    }
    template <class T>
    typename list<T>::nodePtr list<T>::newNode(const T &val) {
        nodePtr res = nodeAllocator::allocate();
        nodeAllocator::construct(res, node<T>(val, nullptr, nullptr, this));
        return res;
    }
    template <class T>
    void list<T>::deleteNode(nodePtr p) {
        p->prev = p->next = nullptr;
        nodeAllocator::destroy(p);
        nodeAllocator::deallocate(p);
    }
    template <class T>
    void list<T>::ctorAux(size_type n, const value_type &val, std::true_type) {
        head.p = newNode(); // add a dummy node
        tail.p = head.p;
        while (n--)
            push_back(val);
    }
    template <class T>
    template <class InputIterator>
    void list<T>::ctorAux(InputIterator first, InputIterator last,
                          std::false_type) {
        head.p = newNode(); // add a dummy node
        tail.p = head.p;
        for (; first != last; ++first)
            push_back(*first);
    }
    template <class T>
    typename list<T>::size_type list<T>::size() const {
        size_type length = 0;
        for (auto h = head; h != tail; ++h)
            ++length;
        return length;
    }

    template <class T>
    void list<T>::push_front(const value_type &val) {
        auto node    = newNode(val);
        head.p->prev = node;
        node->next   = head.p;
        head.p       = node;
    }
    template <class T>
    void list<T>::pop_front() {
        auto oldNode = head.p;
        head.p       = oldNode->next;
        head.p->prev = nullptr;
        deleteNode(oldNode);
    }
    template <class T>
    void list<T>::push_back(const value_type &val) {
        auto node      = newNode();
        (tail.p)->data = val;
        (tail.p)->next = node;
        node->prev     = tail.p;
        tail.p         = node;
    }
    template <class T>
    void list<T>::pop_back() {
        auto newTail  = tail.p->prev;
        newTail->next = nullptr;
        deleteNode(tail.p);
        tail.p = newTail;
    }
    template <class T>
    typename list<T>::iterator list<T>::insert(iterator          position,
                                               const value_type &val) {
        if (position == begin()) {
            push_front(val);
            return begin();
        }
        else if (position == end()) {
            auto ret = position;
            push_back(val);
            return ret;
        }
        auto node        = newNode(val);
        auto prev        = position.p->prev;
        node->next       = position.p;
        node->prev       = prev;
        prev->next       = node;
        position.p->prev = node;
        return iterator(node);
    }

    template <class T>
    template <class InputIterator>
    void list<T>::insert(iterator position, size_type n,
                         const value_type &val) {
        insert_aux(position, n, val,
                   typename std::is_integral<InputIterator>::type());
    }

    template <class T>
    template <class InputIterator>
    void list<T>::insert(iterator position, InputIterator first,
                         InputIterator last) {
        insert_aux(position, first, last,
                   typename std::is_integral<InputIterator>::type());
    }

    template <class T>
    typename list<T>::iterator list<T>::erase(iterator position) {
        if (position == head) {
            pop_front();
            return head;
        }
        else {
            auto prev              = position.p->prev;
            prev->next             = position.p->next;
            position.p->next->prev = prev;
            deleteNode(position.p);
            return iterator(prev->next);
        }
    }

    template <class T>
    typename list<T>::iterator list<T>::erase(iterator first, iterator last) {
        typename list<T>::iterator res;
        for (; first != last;) {
            auto temp = first++;
            res       = erase(temp);
        }
        return res;
    }

    template <class T>
    void list<T>::clear() {
        erase(begin(), end());
    }

    template <class T>
    typename list<T>::iterator list<T>::begin() {
        return head;
    }

    template <class T>
    typename list<T>::iterator list<T>::end() {
        return tail;
    }

    template <class T>
    typename list<T>::const_iterator
    list<T>::changeIteratorToConstIterator(iterator &it) const {
        using nodeP        = node<const T> *;
        auto          temp = (list<const T> *const)this;
        auto          ptr  = it.p;
        node<const T> node(ptr->data, (nodeP)(ptr->prev), (nodeP)(ptr->next),
                           temp);
        return const_iterator(&node);
    }

    template <class T>
    typename list<T>::const_iterator list<T>::begin() const {
        auto temp = (list *const)this;
        return changeIteratorToConstIterator(temp->head);
    }

    template <class T>
    typename list<T>::const_iterator list<T>::end() const {
        auto temp = (list *const)this;
        return changeIteratorToConstIterator(temp->tail);
    }

    template <class T>
    typename list<T>::reverse_iterator list<T>::rbegin() {
        return reverse_iterator(tail);
    }

    template <class T>
    typename list<T>::reverse_iterator list<T>::rend() {
        return reverse_iterator(head);
    }

    template <class T>
    void list<T>::reverse() { //����β�巨
        if (empty() || head.p->next == tail.p)
            return;
        auto curNode = head.p;
        head.p       = tail.p->prev;
        head.p->prev = nullptr;
        do {
            auto nextNode      = curNode->next;
            curNode->next      = head.p->next;
            head.p->next->prev = curNode;
            head.p->next       = curNode;
            curNode->prev      = head.p;
            curNode            = nextNode;
        } while (curNode != head.p);
    }

    template <class T>
    void list<T>::remove(const value_type &val) {
        for (auto it = begin(); it != end();) {
            if (*it == val)
                it = erase(it);
            else
                ++it;
        }
    }

    template <class T>
    template <class Predicate>
    void list<T>::remove_if(Predicate pred) {
        for (auto it = begin(); it != end();) {
            if (pred(*it))
                it = erase(it);
            else
                ++it;
        }
    }

    template <class T>
    void list<T>::swap(list &x) {
        STL::swap(head.p, x.head.p);
        STL::swap(tail.p, x.tail.p);
    }

    template <class T>
    void swap(list<T> &x, list<T> &y) {
        x.swap(y);
    }

    template <class T>
    void list<T>::unique() {
        nodePtr curNode = head.p;
        while (curNode != tail.p) {
            nodePtr nextNode = curNode->next;
            if (curNode->data == nextNode->data) {
                if (nextNode == tail.p) {
                    curNode->next = nullptr;
                    tail.p        = curNode;
                }
                else {
                    curNode->next        = nextNode->next;
                    nextNode->next->prev = curNode;
                }
                deleteNode(nextNode);
            }
            else {
                curNode = nextNode;
            }
        }
    }
    template <class T>
    template <class BinaryPredicate>
    void list<T>::unique(BinaryPredicate binary_pred) {
        nodePtr curNode = head.p;
        while (curNode != tail.p) {
            nodePtr nextNode = curNode->next;
            if (binary_pred(curNode->data, nextNode->data)) {
                if (nextNode == tail.p) {
                    curNode->next = nullptr;
                    tail.p        = curNode;
                }
                else {
                    curNode->next        = nextNode->next;
                    nextNode->next->prev = curNode;
                }
                deleteNode(nextNode);
            }
            else {
                curNode = nextNode;
            }
        }
    }

    template <class T>
    void list<T>::splice(iterator position, list &x) {
        this->insert(position, x.begin(), x.end());
        x.head.p = x.tail.p;
    }

    template <class T>
    void list<T>::splice(iterator position, list &x, iterator first,
                         iterator last) {
        if (first.p == last.p)
            return;
        auto tailNode = last.p->prev;
        if (x.head.p == first.p) {
            x.head.p       = last.p;
            x.head.p->prev = nullptr;
        }
        else {
            first.p->prev->next = last.p;
            last.p->prev        = first.p->prev;
        }
        if (position.p == head.p) {
            first.p->prev  = nullptr;
            tailNode->next = head.p;
            head.p->prev   = tailNode;
            head.p         = first.p;
        }
        else {
            position.p->prev->next = first.p;
            first.p->prev          = position.p->prev;
            tailNode->next         = position.p;
            position.p->prev       = tailNode;
        }
    }

    template <class T>
    void list<T>::splice(iterator position, list &x, iterator i) {
        auto next = i;
        this->splice(position, x, i, ++next);
    }

    template <class T>
    void list<T>::merge(list &x) {
        auto it1 = begin(), it2 = x.begin();
        while (it1 != end() && it2 != x.end()) {
            if (*it1 <= *it2)
                ++it1;
            else {
                auto temp = it2++;
                this->splice(it1, x, temp);
            }
        }
        if (it1 == end()) {
            this->splice(it1, x, it2, x.end());
        }
    }

    template <class T>
    template <class Compare>
    void list<T>::merge(list &x, Compare comp) {
        auto it1 = begin(), it2 = x.begin();
        while (it1 != end() && it2 != x.end()) {
            if (comp(*it2, *it1)) {
                auto temp = it2++;
                this->splice(it1, x, temp);
            }
            else
                ++it1;
        }
        if (it1 == end()) {
            this->splice(it1, x, it2, x.end());
        }
    }

    template <class T>
    bool operator==(const list<T> &lhs, const list<T> &rhs) {
        auto node1 = lhs.head.p, node2 = rhs.head.p;
        for (; node1 != lhs.tail.p && node2 != rhs.tail.p;
             node1 = node1->next, node2 = node2->next) {
            if (node1->data != node2->data)
                break;
        }
        if (node1 == lhs.tail.p && node2 == rhs.tail.p)
            return true;
        return false;
    }

    template <class T>
    bool operator!=(const list<T> &lhs, const list<T> &rhs) {
        return !(lhs == rhs);
    }

    template <class T>
    void list<T>::sort() {
        STL::sort(STL::less<T>());
    }

    template <class T>
    template <class Compare>
    void list<T>::sort(Compare comp) {
        if (empty() || head.p->next == tail.p)
            return;

        list carry;
        list counter[64];
        int  fill = 0;
        while (!empty()) {
            carry.splice(carry.begin(), *this, begin());
            int i = 0;
            while (i < fill && !counter[i].empty()) {
                counter[i].merge(carry, comp);
                carry.swap(counter[i++]);
            }
            carry.swap(counter[i]);
            if (i == fill)
                ++fill;
        }
        for (int i = 0; i != fill; ++i) {
            counter[i].merge(counter[i - 1], comp);
        }
        swap(counter[fill - 1]);
    }
}

#endif /* _LIST_HPP_ */
