
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// list.hpp for Simple-XX/SimpleKernel.

// 初始化
template <class T>
void list_init_head(T *list) {
    list->next = list;
    list->prev = list;
    return;
}

// 在中间添加元素
template <class T>
void list_add_middle(T *prev, T *next, T *new_entry) {
    next->prev      = new_entry;
    new_entry->next = next;
    new_entry->prev = prev;
    prev->next      = new_entry;
    return;
}

// 在 prev 后添加项
template <class T>
void list_add_after(T *prev, T *new_entry) {
    list_add_middle(prev, prev->next, new_entry);
    return;
}

// 在 next 前添加项
template <class T>
void list_add_before(T *next, T *new_entry) {
    list_add_middle(next->prev, next, new_entry);
    return;
}

// 删除元素
template <class T>
void list_del(T *list) {
    list->next->prev = list->prev;
    list->prev->next = list->next;
    return;
}

// 返回后面的的元素
template <class T>
T *list_next(T *list) {
    return list->next;
}
