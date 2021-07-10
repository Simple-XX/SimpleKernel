
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// list.hpp for Simple-XX/SimpleKernel.

#ifndef _LIST_HPP_
#define _LIST_HPP_

// 在堆可用之前使用的双向链表
template <class T>
class tmp_list_t {
private:
    // 在中间添加元素
    void add_middle(tmp_list_t<T> *prev, tmp_list_t<T> *next,
                    tmp_list_t<T> *new_entry) {
        next->prev      = new_entry;
        new_entry->next = next;
        new_entry->prev = prev;
        prev->next      = new_entry;
        return;
    }

protected:
public:
    // 数据
    T data;
    // 前一个
    tmp_list_t<T> *prev;
    // 下一下
    tmp_list_t<T> *next;

    // 初始化
    tmp_list_t(void) {
        return;
    }

    ~tmp_list_t(void) {
        return;
    }

    // 在 prev 后添加项
    void add_after(tmp_list_t<T> *new_entry) {
        add_middle(this, this->next, new_entry);
        return;
    }

    // // 在 next 前添加项
    // void list_add_before(T *next, T *new_entry) {
    //     add_middle(next->prev, next, new_entry);
    //     return;
    // }

    // // 删除元素
    // void list_del(T *list) {
    //     list->next->prev = list->prev;
    //     list->prev->next = list->next;
    //     return;
    // }

    // 返回后面的的元素
    tmp_list_t<T> *list_next(void) {
        return next;
    }
};

#endif /* _LIST_HPP_ */
