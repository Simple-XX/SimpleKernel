
// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// buffer.h for SimpleXX/SimpleKernel.

#ifndef _BUFFER_H_
#define _BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pmm.h"
#include "string.h"
#include "intr.h"

// 缓冲区的数据各个进程都能访问，所以应放入内核对应的内存分区中，即 NORMAL 分区
// 定义缓冲区的起始地址和终止地址，为了节约内存，将缓冲区设置为 20MB-24MB
// 20MB
#define BUFFER_START (0x1400000)

// 24MB，-1 是为了进行页对齐，刚好使用 4MB 的内存空间
#define BUFFER_END (0x1800000 - 1)

// 哈希表大小
#define hash_num 400

// 定义哈希操作
#define hash(dev, block) hash_table[(uint32_t)(dev ^ block) % hash_num]

// 定义缓冲块的优先级别，BADNESS越低则优先级越高
#define BADNESS(bh) (((bh)->dirty) << (1 + (bh)->lock))

// 请求项大小
#define request_num 40

// 缓冲块结构体，一个缓冲块=一页
typedef struct buffer_head {
    // 缓冲块地址
    ptr_t addr;
    // 块号，假定使用的文件系统数据块大小等于页大小
    uint32_t block_num;
    // 设备号
    uint8_t dev;
    // 是否需要更新位
    uint8_t uptodate;
    // 脏位
    uint8_t dirty;
    // 计数器，使用该块的所有进程
    uint8_t count;
    // 锁位，0-代表已解锁，1-代表未解锁
    uint8_t lock;
    // 进程等待链表
    // task_pcb_t *        wait;
    // 哈希表链表
    struct buffer_head *ht_prev;
    // 哈希表链表
    struct buffer_head *ht_next;
    // 缓冲块链表
    struct buffer_head *bh_prev;
    // 缓冲块链表
    struct buffer_head *bh_next;
} buffer_head_t;

typedef struct request {
    // 设备号
    uint8_t dev;
    //  0代表读/1代表写
    uint8_t cmd;
    // 请求项对应的扇区号
    uint32_t sector;
    // 等待请求项的进程队列
    // task_pcb_t *        wait;
    // 请求项对应的缓冲块
    buffer_head_t *bh;
    // 请求项队列
    struct request *next;
} request_t;

// 缓冲区初始化
void buffer_init();

// 给定设备号和块号，将相应数据读入缓冲块中，该过程需要一个进程实现，当读取完数据后，需要在该进程中释放该缓冲块。
buffer_head_t *buffer_read(uint8_t dev, uint32_t block);

// 根据设备号和块号确定一个缓冲块
buffer_head_t *get_buffer(uint8_t dev, uint32_t block);

// 释放缓冲块
void buffer_release(buffer_head_t *bh);

// 在哈希表中查看是否有设备号和块号对应的缓冲块
buffer_head_t *find_buffer(uint8_t dev, uint32_t block);

// 从哈希表中获取缓冲块
buffer_head_t *get_hash_table(uint8_t dev, uint32_t block);

// 等待缓冲块解锁，需关中断，并且该进程变成睡眠状态
void wait_on_buffer(buffer_head_t *bh);

// 从缓冲区中移除缓冲块
void remove_buffer(buffer_head_t *bh);

// 插入缓冲块到缓冲区
void insert_buffer(buffer_head_t *bh);

// 请求项初始化
void request_init();

// 新添请求项，输入设备号、读/写命令、设备扇区号等
bool add_request(uint8_t dev, uint8_t cmd, uint32_t sector);

// 请求项处理函数
void do_request();

// 测试
void test_buffer();

#ifdef __cplusplus
}
#endif

#endif /* _BUFFER_H_ */