//
// // This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// //
// // LinkedList.c for MRNIU/SimpleKernel.
//
// #include "stddef.h"
// #include "stdbool.h"
//
// typedef struct LLNode * LL;
// struct LLNode {
//   void * data;
//   LL prev;
//   LL next;
//   LL head;
//   LL tail;
//   void (*AddtoHead)(LL node, void * data);
//   void (*AddtoTail)(LL node, void * data);
//   void *(*RemoveFromHead)(LL node);     // 从头部删除
//   void *(*RemoveFromTail)(LL node);    // 从尾部删除
//   void *(*GetHead)(LL node);       // 获取头部数据
//   void *(*GetTail)(LL node);       // 获取尾部数据
//   bool (*Search)(LL node, void * key);     // 判断 key 是否在链表内
//   bool (*Empty)(LL node);       // 判断链表是否为空
//   size_t (*size)(LL node);     // 返回链表大小
// };
//
// void AddtoHead(LL node, void * data){
//   if(node->Empty(node)) {     // 如果链表为空
//     node->head=node->tail=(LL)malloc(sizeof(*node));      // 则使头尾指针均指向 new 的节点
//   } else{   // 否则将头指针换成 new 的节点
//     node->head->prev=(LL)malloc(sizeof(*node));
//     node->head->prev->next=node->head;
//     node->head=node->head->prev;
//   }
//   return;
// }
//
// bool Empty(LL node){
//   return node->head==NULL;    // 如果头指针为空，则链表为空
// }
//
// void AddtoTail(LL node, void * data){
//   if(node->Empty(node)) {     // 如果链表为空
//     node->head=node->tail=(LL)malloc(sizeof(*node));      // 则使头尾指针均指向 new 的节点
//   } else{
//     node->tail->next=(LL)malloc(sizeof(*node));
//     node->tail->next->prev=node->tail;
//     node->tail=node->tail->next;
//   }
//   return;
// }
//
// void * RemoveFromHead(LL node, size_t type_size){
//   if(node->Empty(node)) {     // 如果链表为空
//     return NULL;
//   } else if ((node->head==node->tail)&&(node->head!=NULL)) {  // 仅有一个元素
//     void * tmp = malloc(type_size);
//     tmp=node->head->data;
//     node->head=node->tail=NULL;
//     return tmp;
//   } else{
//     node->head=node->head->next;
//     void * tmp = malloc(type_size);
//     tmp=node->head->prev->data;
//     node->head->prev=NULL;
//     return tmp;
//   }
// }
//
// void *RemoveFromTail(LL node, size_t type_size){
//   if(node->Empty(node)) {     // 如果链表为空
//     return NULL;
//   } else if ((node->head==node->tail)&&(node->tail!=NULL)) {  // 仅有一个元素
//     void * tmp = malloc(type_size);
//     tmp=node->tail->data;
//     node->head=node->tail=NULL;
//     return tmp;
//   } else{
//     node->tail=node->tail->prev;
//     void * tmp = malloc(type_size);
//     tmp=node->tail->next->data;
//     node->tail->next=NULL;
//     return tmp;
//   }
// }
//
// void * GetHead(LL node){
//   if(node->Empty(node))
//     return NULL;
//   else
//     return node->head->data;
// }
//
// void * GetTail(LL node) {
//   if(node->Empty(node))
//     return NULL;
//   else
//     return node->tail->data;
// }
//
// bool Search(LL node, void * key){
//   LL tmp=node->head;
//   while (tmp->next!=NULL) {
//     if(tmp->data==key)
//       return true;
//     tmp=tmp->next;
//   }
//   return false;
// }
//
// size_t size(LL node){
//   int count=1;
//   LL tmp=node->head;
//   while(tmp->next!=NULL) {
//     count+=1;
//     tmp=tmp->next;
//   }
//   return count;
// }
//
// LL init_LL(){
//   LL node=(LL)malloc(sizeof(*node));
//   node->data=NULL;
//   node->prev=NULL;
//   node->next=NULL;
//   node->head=NULL;
//   node->tail=NULL;
//   node->AddtoHead=AddtoHead;
//   node->AddtoTail=AddtoTail;
//   node->RemoveFromHead=RemoveFromHead;
//   node->RemoveFromTail=RemoveFromTail;
//   node->GetHead=GetHead;
//   node->GetTail=GetTail;
//   node->Search=Search;
//   node->Empty=Empty;
//   node->size=size;
//   return node;
// }
//
// LL list=init_LL();
