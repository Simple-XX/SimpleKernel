# src/kernel/mem
## 文件说明

- buddy.c

    buddy 算法实现。

- first_fit.c

    firrstfit 首次适应算法实现。

- mem

    内存管理代码 ，页表管理。

- slab.c

    slab内存分配机制实现，slab分配算法采用cache 存储内核对象，slab 缓存、从缓存中分配和释放对象然后销毁缓存的过程必须要定义一个 kmem_cache 对象，然后对其进行初始化这个特定的缓存包含 32 字节的对象，具体描述可参考docs文件夹下内容。

    

    

    
