//
//  Queue.cpp
//  DataStructuer
//
//  Created by Zone.Niuzh on 2018/9/27.
//  Copyright © 2018 MRNIU. All rights reserved.
//
// 队列的链表实现
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// Queue.cpp for MRNIU/SimpleKernel.


#include "DataStructuer.h"

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

template <class T>
Queue<T>::Queue(){
    return;
}

template <class T>
Queue<T>::Queue(const T data){
    LL.AddtoHead(data);
    return;
}

template <class T>
void Queue<T>::EnQueue(const T data){
    LL.AddtoHead(data);
    return;
}

template <class T>
T Queue<T>::DeQueue(){
    return LL.RemoveFromTail();
}

template <class T>
bool Queue<T>::Empty() const{
    return LL.Empty();
}

template <class T>
T Queue<T>::GetFirst() const{
    return LL.GetHead();
}

template <class T>
Queue<T>::~Queue(){
    LL.~LinkedList<T>();
    return;
}
