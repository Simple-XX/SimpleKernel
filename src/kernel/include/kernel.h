
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

extern "C" void kernel_main(void);

class KERNEL {
private:
public:
    KERNEL(void);
    ~KERNEL();
    int init(void);
};

#endif /* _KERNEL_H_ */
