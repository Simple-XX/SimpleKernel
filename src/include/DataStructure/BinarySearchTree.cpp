//
//  BinaryTree.cpp
//  DataStructuer
//
//  Created by Zone.N on 2018/10/29.
//  Copyright © 2018 Zone.N. All rights reserved.
//
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// BinaryTree.cpp for MRNIU/SimpleKernel.

#include "DataStructuer.h"

// Binary search tree node
// 节点：BSTN
// 左，右，数据
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
BSTN<T>::BSTN(){
    this->data=0;
    this->right=NULL;
    this->left=NULL;
    this->parent=NULL;
    return;
}

template <class T>
BSTN<T>::BSTN(T data, BSTN<T> * left, BSTN<T> * right){
    this->data=data;
    this->left=left;
    this->right=right;
    return;
}

template <class T>
BSTN<T>::~BSTN(){
    return;
}

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

template <class T>
BinarySearchTree<T>::BinarySearchTree(void){
    this->root=NULL;
    return;
}

template <class T>
BinarySearchTree<T>::BinarySearchTree(const T data){
    this->root=new BSTN<T>(data);
    this->root->data=data;
    return;
}

template <class T>
BinarySearchTree<T>::~BinarySearchTree(void){
    return;
}

template <class T>
int BinarySearchTree<T>::ipl(BSTN<T> * bstn) const{
    if(bstn==NULL) return 0;
    else{

    }
    return 0;
}

template <class T>
bool BinarySearchTree<T>::Search(const T data) const{
    return search_in_sub_tree(this->root, data);
}

template <class T>
bool BinarySearchTree<T>::search_in_sub_tree(BSTN<T> * bstn, const T data) const{
    if(bstn==NULL)  return false;
    if(data<bstn->data)
        return search_in_sub_tree(bstn->left, data);
    else if(data>bstn->data)
        return search_in_sub_tree(bstn->right, data);
    else
        return true;    // 找到了，返回对应节点指针。
}

template <class T>
bool BinarySearchTree<T>::Empty(void) const{
    return this->root==NULL;
}

template <class T>
bool BinarySearchTree<T>::insert(BSTN<T> * bstn, const T data) const{
    if(bstn==NULL){
        bstn=new BSTN<T>(data);
        bstn->data=data;
        return true;
    }
    else{
        BSTN<T> * tmp = NULL;
        while(bstn!=NULL){
            tmp=bstn;
            if(data<bstn->data)
                bstn=bstn->left;
            else if(data>bstn->data)
                bstn=bstn->right;
            else
                return false;
        }
        if(data<tmp->data)
            tmp->left=new BSTN<T>(data);
        else
            tmp->right=new BSTN<T>(data);
        return true;
    }
}

template <class T>
bool BinarySearchTree<T>::Insert(const T data){
    return this->insert(this->root, data);
}

template <class T>
int BinarySearchTree<T>::GetHeight() const{
    return get_height(this->root);
}

template <class T>
int BinarySearchTree<T>::GetLeaf() const{
    return get_leaf(this->root);
}

template <class T>
int BinarySearchTree<T>::get_height(BSTN<T> * bstn) const{
    int height=0;
    bfs(bstn, [&height](BSTN<T> * bstn_t){
        height++;
        });
    return height;
}

template <class T>
int BinarySearchTree<T>::get_leaf(BSTN<T> *bstn) const{
    int counter=0;
    bfs(bstn, [&counter](BSTN<T> *bstn){
        counter++;
    });
    return counter;
}

template <class T>
void BinarySearchTree<T>::Clear(){
    clear(this->root);
    return;
}


template <class T>
void BinarySearchTree<T>::clear(BSTN<T> * bstn){
    if(bstn!=NULL){
        clear(bstn->left);
        clear(bstn->right);
        delete bstn;
    }
    return;
}

template <class T>
void BinarySearchTree<T>::visit(BSTN<T> * bstn) const{
    return;
}

template <class T>
void BinarySearchTree<T>::bfs(BSTN<T> *bstn) const{
    if(bstn!=NULL){
        Queue<BSTN<T>*> queue;
        queue.EnQueue(bstn);
        while(!queue.Empty()){
            BSTN<T> * bstn_tmp=queue.DeQueue();
            visit(bstn_tmp);
            if(bstn_tmp->left!=NULL)
                queue.EnQueue(bstn_tmp->left);
            if(bstn_tmp->right!=NULL)
                queue.EnQueue(bstn_tmp->right);
        }
    }
    return;
}

template <class T>
void BinarySearchTree<T>::bfs(BSTN<T> *bstn, std::function<void (BSTN<T>*)> fun) const{
    if(bstn!=NULL){
        Queue<BSTN<T>*> queue;
        queue.EnQueue(bstn);
        while(!queue.Empty()){
            BSTN<T> * bstn_tmp=queue.DeQueue();
            fun(bstn_tmp);
            if(bstn_tmp->left!=NULL)
                queue.EnQueue(bstn_tmp->left);
            if(bstn_tmp->right!=NULL)
                queue.EnQueue(bstn_tmp->right);
        }
    }
    return;
}

template <class T>
void BinarySearchTree<T>::dfs_lrv(BSTN<T> *bstn) const{
    if(bstn!=NULL){
        dfs_lrv(bstn->left);
        dfs_lrv(bstn->right);
        visit(bstn);
        return;
    }
    return;
}

template <class T>
void BinarySearchTree<T>::dfs_lrv(BSTN<T> *bstn, std::function<void (BSTN<T>*)> fun) const{
    if(bstn!=NULL){
        dfs_lrv(bstn->left);
        dfs_lrv(bstn->right);
        fun(bstn);
        return;
    }
    return;
}

template <class T>
void BinarySearchTree<T>::dfs_lvr(BSTN<T> *bstn) const{
    if(bstn!=NULL){
        dfs_lvr(bstn->left);
        visit(bstn);
        dfs_lvr(bstn->right);
        return;
    }
    return;
}

template <class T>
void BinarySearchTree<T>::dfs_lvr(BSTN<T> *bstn, std::function<void (BSTN<T>*)> fun) const{
    if(bstn!=NULL){
        dfs_lvr(bstn->left);
        fun(bstn);
        dfs_lvr(bstn->right);
        return;
    }
    return;
}

template <class T>
void BinarySearchTree<T>::dfs_vlr(BSTN<T> *bstn) const{
    if(bstn!=NULL){
        (bstn);
        dfs_vlr(bstn->left);
        dfs_vlr(bstn->right);
        return;
    }
    return;
}

template <class T>
void BinarySearchTree<T>::dfs_vlr(BSTN<T> *bstn, std::function<void (BSTN<T>*)> fun) const{
    if(bstn!=NULL){
        fun(bstn);
        dfs_vlr(bstn->left);
        dfs_vlr(bstn->right);
        return;
    }
    return;
}
