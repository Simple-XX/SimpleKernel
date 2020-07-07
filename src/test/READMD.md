# test/

- 主要是用来测试部分函数功能的测试用例，一般在修改完内核代码后，可以编译调用test.c来进行测试，查看下有没有某些函数再修改过程中出现问题。
- 可以看到，代码中主要有test_pmm、test_vmm、test_libc、test_heap、test_task、test_sched等，也就是测试上层文件夹中对应代码。

