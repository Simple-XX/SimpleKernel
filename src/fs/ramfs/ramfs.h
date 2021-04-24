
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// ramfs.h for Simple-XX/SimpleKernel.

#ifndef _RAMFS_H_
#define _RAMFS_H_

#include "vfs.h"

class RAMFS : FS {
private:
protected:
public:
    RAMFS(void);
    virtual ~RAMFS(void);
    virtual int open();
    virtual int close();
    virtual int seek();
};

static RAMFS ramfs;

#endif /* _RAMFS_H_ */
