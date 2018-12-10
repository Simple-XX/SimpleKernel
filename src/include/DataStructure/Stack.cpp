//
//  Stack.cpp
//  DS
//
//  Created by Zone.Niuzh on 2018/9/25.
//  Copyright © 2018 MRNIU. All rights reserved.
//
// 栈的链表实现
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// Stack.cpp for MRNIU/SimpleKernel.


#include "DataStructuer.h"

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

template <class T>
Stack<T>::Stack(const T data){
    LL.AddtoHead(data);
    return;
}

template <class T>
Stack<T>::Stack(){
    return;
}

template <class T>
T Stack<T>::Top() const{
    return LL.GetHead();
}

template <class T>
T Stack<T>::Bottom() const{
    return LL.GetTail();
}

template <class T>
T Stack<T>::Pop(){
    return LL.RemoveFromHead();
}

template <class T>
void Stack<T>::Push(const T data_input){
    LL.AddtoHead(data_input);
    return;
}

template <class T>
bool Stack<T>::Empty() const{
    return LL.Empty();
}

template <class T>
int Stack<T>::Size() const{
    return LL.size();
}

template <class T>
Stack<T>::~Stack(){
    LL.~LinkedList<T>();
    return;
}

template <class T>
void Stack<T>::test(void){
    LL.test();
    return;
}
