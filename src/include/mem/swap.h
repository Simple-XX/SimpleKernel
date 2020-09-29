#ifndef _SWAP_H_
#define _SWAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stddef.h"

#define MAX_DISK_PAGE 5
#define MAX_MEMORY_PAGE 20

struct disk_page{
    int disk_num;
    int page_num;
};

void initQueue();  //初始化
void enQueue();//入队
void deQueue();  //出队
void test_swap();


#ifdef __cplusplus
}
#endif

#endif /* _SWAP_H_ */