
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vfs.h for Simple-XX/SimpleKernel.

#ifndef _VFS_H_
#define _VFS_H_

#include "stdint.h"
#include "io.h"

class VFS {
private:
    static IO io;

protected:
public:
    VFS(void);
    ~VFS(void);
};

#endif /* _VFS_H_ */
