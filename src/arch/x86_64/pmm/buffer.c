
// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// buffer.c for SimpleXX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "buffer.h"

int BUFFER_NUM = 0;
//缓冲块空闲链表
static buffer_head_t *free_list;
//等待空闲缓冲块而睡眠的进程队列
// static struct task_pcb_t *buffer_wait = NULL;
//哈希表
buffer_head_t *hash_table[hash_num];
//请求项数组
request_t all_request[request_num];
//当前存在的请求项
request_t *current_request;

//缓冲区初始化
void buffer_init() {
    //缓冲区清零
    bzero((void *)BUFFER_START, BUFFER_END - BUFFER_START);
    //初始化缓冲块结构体
    buffer_head_t *h = (buffer_head_t *)BUFFER_START;
    //每一页对应一个缓冲块，对每个缓冲块进行初始化,结果为1015.07，说明最多包含1015个缓冲块
    int num = (BUFFER_END - BUFFER_START + 1) /
              (sizeof(buffer_head_t) + PMM_PAGE_SIZE);
    //缓冲块存储区起始地址则从结构体后一页开始,
    ptr_t buffer_mem_start =
        BUFFER_START +
        ((num * sizeof(buffer_head_t)) / PMM_PAGE_SIZE + 1) * PMM_PAGE_SIZE;
    printk_info("%d\n", sizeof(buffer_head_t));
    printk_info("%d\n", num);
    for (ptr_t i = buffer_mem_start & PMM_PAGE_MASK;
         i <= (BUFFER_END & PMM_PAGE_MASK); i += PMM_PAGE_SIZE) {
        //每个缓冲块的起始地址
        h->addr = i;
        //每个缓冲块对应的设备块号
        h->block_num = 0;
        //设备号
        h->dev      = 0;
        h->uptodate = 0;
        h->dirty    = 0;
        h->count    = 0;
        h->lock     = 0;
        // h->wait     = NULL;
        h->ht_prev = NULL;
        h->ht_next = NULL;
        h->bh_prev = h - 1;
        h->bh_next = h + 1;
        h++;
        //对缓冲块进行计数
        BUFFER_NUM++;
    }
    // printk_info("buffer num:%d\n",BUFFER_NUM);
    h--;
    //初始化缓冲块空闲链表，由于此前没有使用过缓冲块，所以直接指向缓冲块链表即可。
    free_list = (buffer_head_t *)BUFFER_START;
    //将双向链表的首尾相连形成双向循环链表
    free_list->bh_prev = h;
    h->bh_next         = free_list;
    // printk_info("addr:%X\n",free_list->addr);
    //初始化哈希表数组，每个元素均是一个链表，由于没有挂上缓冲块，均设置为NULL
    for (int i = 0; i < hash_num; i++) {
        hash_table[i] = NULL;
    }
}

//给定设备号和块号，将相应数据读入缓冲块中，该过程需要一个进程实现，当读取完数据后，需要在该进程中释放该缓冲块。
buffer_head_t *buffer_read(uint8_t dev, uint32_t block) {
    buffer_head_t *bh;
    //根据设备号和块号获取一个缓冲块
    if (!(bh = get_buffer(dev, block))) {
        printk_info("buffer returned NULL");
    }
    //如果缓冲块数据是有效的（已更新），则直接返回该缓冲块。
    if (bh->uptodate) {
        return bh;
    }
    /******************************************/
    //待补充：从文件系统中读取数据到该缓冲块中
    memset((void *)bh->addr, 0x11, PMM_PAGE_SIZE);
    printk_info("read success!\n");
    bh->uptodate = 1;
    /******************************************/
    //等待缓冲块解锁
    wait_on_buffer(bh);
    //若已经更新，则直接使用
    if (bh->uptodate) {
        return bh;
    }
    //释放缓冲块
    buffer_release(bh);
    return NULL;
}

//根据设备号和块号确定一个缓冲块
buffer_head_t *get_buffer(uint8_t dev, uint32_t block) {
    buffer_head_t *tmp, *bh;
repeat:
    //从哈希表中获取是否已经存在设备号和块号对应的缓冲块，若有则直接返回
    bh = get_hash_table(dev, block);
    if (bh) {
        return bh;
    }
    //遍历缓冲块链表
    tmp = free_list;
    do {
        //如果该缓冲块被使用，则寻找下一个
        if (tmp->count)
            continue;
        //首先找lock和dirty都为0的缓冲块，若有则直接退出，若没有则将bh设置为badness最小的缓冲块
        if (!bh || BADNESS(tmp) < BADNESS(bh)) {
            bh = tmp;
            // if (!BADNESS(tmp)) {
            if (BADNESS(tmp) == 0) {
                break;
            }
        }
    } while ((tmp = tmp->bh_next) != free_list);
    //如果一轮下来所有缓冲块均被使用，则该进程进入睡眠状态，等待有空闲缓冲块时唤醒
    if (!bh) {
        //进入睡眠状态，涉及到多进程
        // sleep_on(&buffer_wait);
        goto repeat;
    }
    //当找到一个缓冲块时，先等待其解锁
    wait_on_buffer(bh);
    //如果解锁过程中被其他进程占用那么需要重新找一个
    if (bh->count)
        goto repeat;
    //当找到的缓冲块是“脏”的，即已经被改写，需要把它写到设备文件中，然后再使用该缓冲块
    while (bh->dirty) {
        //同步操作，将缓冲块的内容同步到设备文件中
        // sync_dev(bh->dev);
        //等待解锁
        wait_on_buffer(bh);
        //若仍被其他进程占用
        if (bh->count)
            goto repeat;
    }
    //在进程睡眠过程中，其他进程可能已经将设备号和块号对应的缓冲块加入到链表中，这样我们就需要重新寻找该缓冲块
    if (find_buffer(dev, block))
        goto repeat;
    //此时，我们找到了一个未被占用、未被上锁并且是干净的缓冲块。
    //计数为1，只有该进程独占
    bh->count    = 1;
    bh->dirty    = 0;
    bh->uptodate = 0;
    //在缓冲区中移除原来设备号和块号所占用的缓冲块
    remove_buffer(bh);
    //对缓冲块重新设置新的设备号和块号
    bh->dev       = dev;
    bh->block_num = block;
    //在缓冲区中加入新的设备号和块号所占用的缓冲块
    insert_buffer(bh);
    return bh;
}

//释放缓冲块
void buffer_release(buffer_head_t *bh) {
    //若缓冲块为空，则直接返回
    if (!bh)
        return;
    //等待缓冲块解锁
    wait_on_buffer(bh);
    //缓冲块的进程计数减一
    if (!(bh->count--))
        printk_info("error!,free a free buffer!");
    //唤醒等待缓冲块的进程
    // wake_up(&buffer_wait);
}

//在哈希表中查看是否有设备号和块号对应的缓冲块
buffer_head_t *find_buffer(uint8_t dev, uint32_t block) {
    buffer_head_t *tmp;
    // hash操作是根据设备号和块号确定哈希表中的位置
    for (tmp = hash(dev, block); tmp != NULL; tmp = tmp->ht_next)
        //根据具体缓冲块的设备号和块号确定是否是我们需要的缓冲块，若是则返回该缓冲块
        if (tmp->dev == dev && tmp->block_num == block)
            return tmp;
    //若哈希表中没有则返回NULL
    return NULL;
}

//从哈希表中获取缓冲块
buffer_head_t *get_hash_table(uint8_t dev, uint32_t block) {
    buffer_head_t *bh;
    while (1) {
        //若哈希表中设备号和块号对应的缓冲块，则返回空
        if (!(bh = find_buffer(dev, block)))
            return NULL;
        //若有，则计数+1，即该进程需要使用
        bh->count++;
        //等待这个缓冲块解锁
        wait_on_buffer(bh);
        //如果在解锁过程中，该缓冲块没被其他进程占用，即设备号和块号不变，则返回该缓冲块
        if (bh->dev == dev && bh->block_num == block)
            return bh;
        //在解锁过程中，缓冲块被其他进程占用，将其计数-1，并重新找一个新的缓冲块。
        bh->count--;
    }
}

//等待缓冲块解锁，需关中断，并且该进程变成睡眠状态
void wait_on_buffer(buffer_head_t *bh) {
    bool intr_flag = false;
    //关中断
    local_intr_store(intr_flag);
    {
        //被上锁则该进程需进入睡眠状态
        while (bh->lock)
            ;
        // sleep_on(&buffer_wait);
    }
    //开中断
    local_intr_restore(intr_flag);
}

//从缓冲区中移除缓冲块
void remove_buffer(buffer_head_t *bh) {
    //首先移除哈希表中的缓冲块对应节点
    if (bh->ht_next)
        bh->ht_next->ht_prev = bh->ht_prev;
    if (bh->ht_prev)
        bh->ht_prev->ht_next = bh->ht_next;
    if (hash(bh->dev, bh->block_num) == bh)
        hash(bh->dev, bh->block_num) = bh->ht_next;
    //然后移除缓冲块链表中的对应节点
    if (!bh->bh_prev || !bh->bh_next)
        printk_info("error!\n");
    bh->bh_next->bh_prev = bh->bh_prev;
    bh->bh_prev->bh_next = bh->bh_next;
    //判断是不是free_list指向的节点，若是则free_list指向下一个节点
    if (free_list == bh)
        free_list = bh->bh_next;
}

void insert_buffer(buffer_head_t *bh) {
    //首先将节点插入到缓冲块链表中
    bh->bh_prev                 = free_list->bh_prev;
    bh->bh_next                 = free_list;
    free_list->bh_prev->bh_next = bh;
    free_list->bh_prev          = bh;
    //然后将节点插入到哈希表中
    bh->ht_prev = NULL;
    bh->ht_next = NULL;
    if (!bh->dev)
        return;
    bh->ht_next                  = hash(bh->dev, bh->block_num);
    hash(bh->dev, bh->block_num) = bh;
    bh->ht_next->ht_prev         = bh;
}

//请求项初始化
void request_init() {
    //主要是对请求项数组的40个元素进行初始化
    for (int i = 0; i < request_num; i++) {
        all_request[i].dev    = 0;
        all_request[i].cmd    = 0;
        all_request[i].sector = 0;
        // all_request[i].wait   = NULL;
        all_request[i].bh   = NULL;
        all_request[i].next = NULL;
    }
}

//新添请求项，输入设备号、读/写命令、设备扇区号等
bool add_request(uint8_t dev, uint8_t cmd, uint32_t sector) {
    //遍历请求项数组
    for (int i = 0; i < request_num; i++) {
        //找到第一个空请求项
        if (!all_request[i].dev) {
            //设置请求项的属性
            all_request[i].dev    = dev;
            all_request[i].cmd    = cmd;
            all_request[i].sector = sector;
            //当请求项申请不到一个缓冲块时，返回false
            if (!(all_request[i].bh = buffer_read(dev, sector)))
                return false;
            //将current_request指向当前请求项
            if (current_request)
                all_request[i].next = current_request;
            current_request = (request_t *)(&all_request[i]);
            //申请到一个空请求项，并设置完毕后，返回true
            return true;
        }
    }
    // 40个请求项坑位均被占用，返回false
    return false;
}

//请求项处理函数
void do_request() {
    //当前处理的请求项不为空
    if (!current_request) {
        //请求项为读数据块
        if (current_request->cmd == 0)
            ;
        /**********************/
        //调用设备驱动程序将设备中对应的数据块拷贝到请求项对应的缓冲块中
        /**********************/
        //请求项为写数据块
        else {
            ;
        }
        /**********************/
        //调用设备驱动程序将请求项对应的缓冲块拷贝到设备中对应的数据块中
        /**********************/
    }
}

//对缓冲区进行测试
void test_buffer() {
    buffer_head_t *h;
    printk_info("test buffer....\n");
    buffer_init();
    //第一个缓冲块地址
    printk_info("addr:%X\n", free_list->addr);
    //模拟缓冲块建立
    //假定读取dev为1（比如硬盘），块号为100的数据块
    h = buffer_read(1, 100);
    //读取缓冲块中数据块的前4个字节
    uint32_t *data = (uint32_t *)h->addr;
    printk_info("dev:1 block_num:100----data:%x\n", *data);
}

#ifdef __cplusplus
}
#endif