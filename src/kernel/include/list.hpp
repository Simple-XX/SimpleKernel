
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// list.hpp for Simple-XX/SimpleKernel.

#ifndef _LIST_HPP_
#define _LIST_HPP_

#include "assert.h"

// 初始化: 物理地址范围，在此范围内的内存会被用来保存 node_t
// 添加数据: 传入 data，内部的 node 处理隐藏起来
// 额外规定: 相邻节点的的内存范围是连续的

// 在堆可用之前使用的双向链表
template <class T>
class tmp_list_t {
private:
    struct node_t {
        // 数据
        T data;
        // 链表指针
        node_t *prev;
        node_t *next;
        // 是否已经使用
        bool used;
    };

    // 获取空闲节点指针
    node_t *get_node(void) {
        for (size_t i = 0; i < nodes_len; i++) {
            if (nodes[i].used == false) {
                // 设置指针
                nodes[i].prev = &nodes[i];
                nodes[i].next = &nodes[i];
                nodes[i].used = true;
                return &nodes[i];
            }
        }
        // 执行到这里说明没有空闲节点了，返回空
        return nullptr;
    }

    // 在中间添加元素
    void add_middle(node_t *_prev, node_t *_next, node_t *_new_node) {
        _next->prev     = _new_node;
        _new_node->next = _next;
        _new_node->prev = _prev;
        _prev->next     = _new_node;
        size++;
        return;
    }

protected:
public:
    // 节点数组
    node_t *nodes;
    // 数组长度
    size_t nodes_len;
    // 当前链表项
    size_t size;
    // 地址
    void *start;
    void *end;

    // 初始化
    tmp_list_t(void *_start, void *_end) {
        start       = _start;
        end         = _end;
        nodes       = (node_t *)start;
        nodes->prev = nodes;
        nodes->next = nodes;
        nodes->used = true;
        nodes_len   = ((uint8_t *)end - (uint8_t *)start) / sizeof(T);
        size        = 1;
        return;
    }

    ~tmp_list_t(void) {
        return;
    }

    // 添加数据到 _prev_data 之后
    // _prev_data: 前一项数据
    // _data: 要添加的数据
    void add_after(T &_prev_data, T &_data) {
        // 找到前一个节点
        node_t *prev = nullptr;
        for (size_t i = 0; i < size; i++) {
            if (nodes[i].data == _prev_data) {
                prev = &nodes[i];
            }
        }
        // 获取一个未使用的节点
        node_t *new_node = get_node();
        new_node->data   = _data;
        add_middle(prev, prev->next, new_node);
        return;
    }

    // 删除元素
    void del(const T &_data) {
        // 根据 _data 找到对应的节点
        node_t *node = nullptr;
        for (size_t i = 0; i < size; i++) {
            if (nodes[i].data == _data) {
                node = &nodes[i];
            }
        }
        // 设置未使用
        node->used = false;
        // 设置指针
        node->next->prev = node->prev;
        node->prev->next = node->next;
        size -= 1;
        return;
    }

    // 返回前面的 data
    T &prev(const T &_data) {
        node_t *node = nullptr;
        for (size_t i = 0; i < size; i++) {
            if (nodes[i].data == _data) {
                node = &nodes[i];
            }
        }
        return node->prev->data;
    }

    // 返回后面的 data
    T &next(const T &_data) {
        node_t *node = nullptr;
        for (size_t i = 0; i < size; i++) {
            if (nodes[i].data == _data) {
                node = &nodes[i];
            }
        }
        return node->next->data;
    }

    T &operator[](size_t _idx) {
        if (nodes[_idx].used == true) {
            return nodes[_idx].data;
        }
        else {
            return *(T *)nullptr;
        }
    }

    // 根据 _data 查找节点
    find_idx(const T &_data) {
        return;
    }
};

#endif /* _LIST_HPP_ */
