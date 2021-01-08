
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

#ifdef __cplusplus
extern "C" {
#endif

void kernel_main(void);

#ifdef __cplusplus
}
#endif

class KERNEL {
private:
public:
    KERNEL(void);
    ~KERNEL();
    int k();
};

#endif /* _KERNEL_H_ */
