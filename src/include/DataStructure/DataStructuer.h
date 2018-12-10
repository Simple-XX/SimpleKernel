
#ifndef DataStructuer_h
#define DataStructuer_h

#include "stddef.h"


// linkedlist
template <class T>
class LLNode {
public:
    T data;
    LLNode * next;
    LLNode * prev;
    LLNode(T data);
    LLNode(void);
    ~LLNode();
};

template <class T>
class LinkedList {
private:
    LLNode<T> * head;
    LLNode<T> * tail;
public:
    void test(void) const;    // 测试函数
    LinkedList(void);
    LinkedList(const T data);
    void AddtoHead(const T data); // 添加到头部
    void AddtoTail(const T data); // 添加到尾部
    T RemoveFromHead(void); // 从头部删除
    T RemoveFromTail(void); // 从尾部删除
    T GetHead(void) const;    // 获取头部数据
    T GetTail(void) const;    // 获取尾部数据
    bool Search(const T key) const; // 判断 key 是否在链表内
    bool Empty(void) const;   // 判断链表是否为空
    int size(void) const; // 返回链表大小
    ~LinkedList(void);
};

// binary tree
template <class T>
class BSTN {
public:
    T data;
    BSTN<T> * left;
    BSTN<T> * right;
    BSTN<T> * parent;
    BSTN(void);
    BSTN(T data, BSTN<T> * left=NULL, BSTN<T> * right=NULL);
    ~BSTN(void);
};

template <class T>
class BinarySearchTree {
protected:
    BSTN<T> * root; // 保存树根节点
    int height; // 保存树高
    int ipl(BSTN<T> * bstn) const; // Internal Path Length 内部路径长度
    void visit(BSTN<T> * bstn) const;   // ok
    void bfs(BSTN<T> * bstn) const; // 广度优先遍历 ok
    void bfs(BSTN<T> *bstn, std::function<void (BSTN<T>*)> fun) const;
    void dfs_vlr(BSTN<T> * bstn) const; // VLR ok
    void dfs_vlr(BSTN<T> * bstn, std::function<void (BSTN<T>*)> fun) const;
    void dfs_lvr(BSTN<T> * bstn) const; // LVR ok
    void dfs_lvr(BSTN<T> * bstn, std::function<void (BSTN<T>*)> fun) const;
    void dfs_lrv(BSTN<T> * bstn) const; // LRV ok
    void dfs_lrv(BSTN<T> * bstn, std::function<void (BSTN<T>*)> fun) const;
    void display_tree(BSTN<T> * bstn) const; // 打印树结构
    const BSTN<T> * array_to_bst(T * arr);  // 将数组转换为 BST 算法见 IA 3e 中文版 p84
    void clear(BSTN<T> *bstn);  // 删除以指定节点为根的树 ok
    int get_height(BSTN<T> *bstn) const; // 返回指定树的树高
    int get_leaf(BSTN<T> *bstn) const;  // 返回以指定节点为根的树的叶子结点数量
    bool insert(BSTN<T> * bstn, const T data) const;  // 插入 ok
    bool search_in_sub_tree(BSTN<T> * bstn, const T data) const;   // 在指定子树中搜索 data ok
public:
    BinarySearchTree(void);
    BinarySearchTree(const T data);
    ~BinarySearchTree(void);
    bool Search(const T data) const;   // 搜索 data
    bool Empty(void) const; // 返回是否为空
    bool Insert(const T data);  // 插入
    int GetHeight(void) const;  // 返回树高
    int GetLeaf(void) const;  // 返回树叶子节点数量
    void Clear();  // 删除树
};

// queue
template <class T>
class Queue{
private:
    LinkedList<T> LL;
public:
    Queue();
    Queue(const T data);
    ~Queue();
    void EnQueue(const T data);
    T DeQueue(void);
    bool Empty(void) const;
    T GetFirst(void) const;
};

// stack
template <class T>
class Stack {
private:
    LinkedList<T> LL;
public:
    void test(void);    // 测试函数
    Stack(void);
    Stack(const T data);
    ~Stack();
    T Top(void) const;    // 返回栈顶数据
    T Bottom(void) const; // 返回栈底数据
    T Pop(void);    // 出栈
    void Push(const T data);  // 压栈
    int Size(void) const; // 返回栈的大小
    bool Empty(void) const;   // 判断栈是否为空
};

// alg
template <typename T>
void sort_bubble233(T * begin, T * end);

template <typename T>
void sort_selection234(T * begin, T * end);

template <typename T>
void sort_selection233(T * begin, T * end);

template <class T>
void mergesort(T a[], size_t l, size_t h);

template <typename T>
void sort_insert233(T * begin, T * end);


#endif
