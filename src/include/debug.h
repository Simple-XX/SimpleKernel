
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// debug.h for Simple-XX/SimpleKernel.

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "stddef.h"
#include "stdint.h"

class DEBUG {
private:
protected:
public:
    DEBUG(void);
    ~DEBUG(void);
    void init(uint32_t magic, void *addr);
    void print_cur_status(void);
    void panic(const char *msg);
    void print_stack(size_t count);
};

extern DEBUG debug;

#endif /* _DEBUG_H_ */
