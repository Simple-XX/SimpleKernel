
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// debug.h for Simple-XX/SimpleKernel.

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "stddef.h"
#include "stdint.h"
#include "io.h"

class DEBUG {
private:
    static IO io;

protected:
public:
    DEBUG(uint32_t magic, void *addr);
    ~DEBUG(void);
    void print_cur_status(void);
    void panic(const char *msg);
    void print_stack(size_t count);
};

#endif /* _DEBUG_H_ */
