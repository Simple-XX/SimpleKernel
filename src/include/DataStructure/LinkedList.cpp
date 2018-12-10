//
//  LinkedList.cpp
//  DS
//
//  Created by Zone.Niuzh on 2018/9/25.
//  Copyright © 2018 MRNIU. All rights reserved.
//
// 双向链表的实现
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// LinkedList.cpp for MRNIU/SimpleKernel.


#include "DataStructuer.h"

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
LLNode<T>::LLNode(T data){
    this->data=data;
    next=NULL;
    prev=NULL;
    return;
}

template <class T>
LLNode<T>::LLNode(){
    data=NULL;
    next=NULL;
    prev=NULL;
    return;
}

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

template <class T>
LinkedList<T>::LinkedList(){
    head=tail=NULL;
    return;
}

template <class T>
LinkedList<T>::LinkedList(const T data){
    head=tail=new LLNode<T>(data);
    return;
}

template <class T>
void LinkedList<T>::AddtoHead(const T data){
    if(Empty()){    // 如果链表为空
        head=tail=new LLNode<T>(data);  // 则使头尾指针均指向 new 的节点
    } else{ // 否则将头指针换成 new 的节点
        head->prev=new LLNode<T>(data);
        head->prev->next=head;
        head=head->prev;
    }
    return;
}

template <class T>
void LinkedList<T>::AddtoTail(const T data){
    if(Empty()){    // 如果链表为空
        head=tail=new LLNode<T>(data);  // 则使头尾指针均指向 new 的节点
    } else{
        tail->next=new LLNode<T>(data);
        tail->next->prev=tail;
        tail=tail->next;
    }
    return;
}

template <class T>
T LinkedList<T>::RemoveFromHead(void){
    if(Empty()){    // 如果链表为空
        throw ("Linked List is empty!");
    } else if ((head==tail)&&(head!=NULL)){ // 仅有一个元素
        T tmp=head->data;
        head=tail=NULL;
        return tmp;
    } else{
        head=head->next;
        T tmp=head->prev->data;
        head->prev=NULL;
        return tmp;
    }
}

template <class T>
T LinkedList<T>::RemoveFromTail(void){
    if(Empty()){    // 如果链表为空
//        return 0xCDCD;
        return NULL;
    } else if ((head==tail)&&(tail!=NULL)){ // 仅有一个元素
        T tmp=tail->data;
        head=tail=NULL;
        return tmp;
    } else{
        tail=tail->prev;
        T tmp=tail->next->data;
        tail->next=NULL;
        return tmp;
    }
}

template <class T>
T LinkedList<T>::GetHead() const{
    if(Empty())
        return 0xCDCD;
    else
        return head->data;
}

template <class T>
T LinkedList<T>::GetTail() const{
    if(Empty())
        return 0xCDCD;
    else
        return tail->data;
}


template <class T>
bool LinkedList<T>::Search(const T key) const{
    LLNode<T> * tmp=head;
    while (tmp->next!=NULL) {
        if(tmp->data==key)
            return true;
        tmp=tmp->next;
    }
    return false;
}

template <class T>
bool LinkedList<T>::Empty(void) const{
    return head==NULL;  // 如果头指针为空，则链表为空
}

template <class T>
int LinkedList<T>::size() const{
    int count=1;
    LLNode<T> * tmp=head;
    while(tmp->next!=NULL){
        count+=1;
        tmp=tmp->next;
    }
    return count;
}

template <class T>
LinkedList<T>::~LinkedList(void){
    while(!Empty()){
        LLNode<T> * tmp=head->next;
        head=NULL;
        head=tmp;
    }
    return;
}
