//
//  SortAlgorithm.cpp
//  DataStructuer
//
//  Created by Zone.N on 2018/10/29.
//  Copyright © 2018 Zone.N. All rights reserved.
//
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// SortAlgorithm.cpp for MRNIU/SimpleKernel.


#include "DataStructuer.h"

#define PASS(addr, n) (*(addr+n))
#define EOA(addr, end) (addr==end)
#define SIZE(b, e) (e-b) // C++11

template <typename T>
void swap233(T & src, T & desc){
    T tmp=src;
    src=desc;
    desc=tmp;
    return;
}

// 插入排序
template <typename T>
void sort_insert233(T * begin, T * end){
    for(auto j=1; j<SIZE(begin, end); j++){
        T key= begin[j];
        auto i=j-1;
        while(i>=0 && begin[i]>key){
            begin[i+1] = begin[i];
            i--;
        }
        begin[i+1]=key;
    }
    return;
}

//
//void sort233(int b,int e) {
//    if(e-b<=0) return;
//    int mid=(b+e)/2,p1=b,p2=mid+1,i=b;
//    sort(b,mid);
//    sort(mid+1,e);
//    while( p1<=mid || p2<=e )
//        if( p2>e|| (p1<=mid&&a[p1]<=a[p2]) )
//            t[i++]=a[p1++];
//        else t[i++]=a[p2++];
//    for(i=b;i<=e; i++)
//        a[i]=t[i];
//}



// 这个归并没搞懂，后面补上。
template <class T>
void mergesort(T a[], size_t l, size_t h) {
    if (h - l == 1) {
        if (a[l] > a[h]) {
            T t = a[l];
            a[l] = a[h];
            a[h] = t;
        }
    } else if (h == l) {

    } else if (h > l) {
        size_t size = h - l + 1;
        size_t m = l + (h - l) / 2;
        mergesort(a, l, m);
        mergesort(a, m+1, h);
        T* b = new T[size];
        size_t k = 0;
        size_t i = l;
        size_t j = m+1;
        while (i <= m && j <= h)
            if (a[i] <= a[j])
                b[k++] = a[i++];
            else
                b[k++] = a[j++];
        while(i <= m)
            b[k++] = a[i++];
        while(j <= h)
            b[k++] = a[j++];
        for (k = 0; k < size; k++) {
            a[l+k] = b[k];
        }
        delete[] b;
    }
}

template <class T>
void sort_merge233(T * begin, T * end) {
    auto size=end-begin;
    mergesort(begin, 0, size-1);
}

// 选择排序
template <typename T>
void sort_selection233(T * begin, T * end){
    auto size=end-begin;
    for(auto i=0;i<size;i++){
        auto least_index=i;
        for(auto j=i+1;j<size;j++){
            if(begin[j]<begin[least_index])
                least_index=j;
        }
        swap233(begin[least_index], begin[i]);
    }
    return;
}

// 选择排序改进，每次循环找到最小值和最大值，半成品
template <typename T>
void sort_selection234(T * begin, T * end){
    auto size=end-begin;
    for(auto i=0;i<size;i++){
        auto min_index=i;
        auto max_index=size-i-1;
        for(auto j=i+1;j<size-i;j++){
            if(begin[j]<begin[min_index]){
                min_index=j;
                continue;
            }
            if(begin[j]>begin[max_index])
                max_index=j;
        }
        swap233(begin[min_index], begin[i]);
        swap233(begin[max_index], begin[size-i-1]);
        //if(
    }
    return;
}

// 冒泡排序
template <typename T>
void sort_bubble233(T * begin, T * end){
    auto size=end-begin;
    for(auto i=0;i<size;i++)
        for(auto j=size;j>i;j--)
            if(begin[j]<begin[j-1])
                swap233(begin[j], begin[j-1]);
    return;
}
