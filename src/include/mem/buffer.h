#ifndef _BUFFER_H_
#define _BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "pmm.h"
#include "vmm.h"
#include "string.h"
#include "task/task.h"
#include "sync.hpp"
#include "intr/include/intr.h"
//缓冲区的数据各个进程都能访问，所以应放入内核对应的内存分区中，即NORMAL分区
//定义缓冲区的起始地址和终止地址，为了节约内存，将缓冲区设置为20MB-24MB
#define BUFFER_START (0x1400000) // 20MB
#define BUFFER_END                                                             \
    (0x1800000 - 1) // 24MB，-1是为了进行页对齐，刚好使用4MB的内存空间
//哈希表大小
#define hash_num 400
//定义哈希操作
#define hash(dev, block) hash_table[(uint32_t)(dev ^ block) % hash_num]
//定义缓冲块的优先级别，BADNESS越低则优先级越高
#define BADNESS(bh) (((bh)->dirty) << 1 + (bh)->lock)
//请求项大小
#define request_num 40
//缓冲块结构体，一个缓冲块=一页
struct buffer_head {
    ptr_t addr; //缓冲块地址
    uint32_t block_num; //块号，假定使用的文件系统数据块大小等于页大小
    uint8_t             dev;      //设备号
    uint8_t             uptodate; //是否需要更新位
    uint8_t             dirty;    //脏位
    uint8_t             count;    //计数器，使用该块的所有进程
    uint8_t             lock; //锁位，0-代表已解锁，1-代表未解锁
    task_pcb_t *        wait; //进程等待链表
    struct buffer_head *ht_prev; //哈希表链表
    struct buffer_head *ht_next; //哈希表链表
    struct buffer_head *bh_prev; //缓冲块链表
    struct buffer_head *bh_next; //缓冲块链表
};
struct request {
    uint8_t             dev;    //设备号
    uint8_t             cmd;    // 0代表读/1代表写
    uint32_t            sector; //请求项对应的扇区号
    task_pcb_t *        wait;   //等待请求项的进程队列
    struct buffer_head *bh;     //请求项对应的缓冲块
    struct request *    next;   //请求项队列
};
void                buffer_init();
struct buffer_head *buffer_read(uint8_t dev, uint32_t block);
struct buffer_head *get_buffer(uint8_t dev, uint32_t block);
void                buffer_release(struct buffer_head *bh);
struct buffer_head *find_buffer(uint8_t dev, uint32_t block);
struct buffer_head *get_hash_table(uint8_t dev, uint32_t block);
void                wait_on_buffer(struct buffer_head *bh);
void                remove_buffer(struct buffer_head *bh);
void                insert_buffer(struct buffer_head *bh);
void                request_init();
bool                add_request(uint8_t dev, uint8_t cmd, uint32_t sector);
void                do_request();
void                test_buffer();
#ifdef __cplusplus
}
#endif

#endif /* _BUFFER_H_ */