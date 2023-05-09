
/**
 * @file ffsystem.c
 * @brief A Sample Code of User Provided OS Dependent Functions for FatFs
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-05-08
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * Based on http://elm-chan.org/fsw/ff/00index_e.html
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-05-08<td>Zone.N<td>创建文件
 * </table>
 */

#include "ff.h"

/// Use dynamic memory allocation
#if FF_USE_LFN == 3

/// with POSIX API
#    include <stdlib.h>

void* ff_memalloc(UINT _msize) {
    return malloc((size_t)_msize);
}

void ff_memfree(void* _mblock) {
    free(_mblock);
}

#endif

/// Mutal exclusion
#if FF_FS_REENTRANT

// Definitions of Mutex
/// 0:Win32, 1:uITRON4.0, 2:uC/OS-II, 3:FreeRTOS, 4:CMSIS-RTOS
#    define OS_TYPE 0

// Win32
#    if OS_TYPE == 0
#        include <windows.h>
// Table of mutex handle
static HANDLE Mutex[FF_VOLUMES + 1];

// uITRON
#    elif OS_TYPE == 1
#        include "itron.h"
#        include "kernel.h"
// Table of mutex ID
static mtxid Mutex[FF_VOLUMES + 1];

// uc/OS-II
#    elif OS_TYPE == 2
#        include "includes.h"
// Table of mutex pinter
static OS_EVENT* Mutex[FF_VOLUMES + 1];

// FreeRTOS
#    elif OS_TYPE == 3
#        include "FreeRTOS.h"
#        include "semphr.h"
// Table of mutex handle
static SemaphoreHandle_t Mutex[FF_VOLUMES + 1];

// CMSIS-RTOS
#    elif OS_TYPE == 4
#        include "cmsis_os.h"
// Table of mutex ID
static osMutexId Mutex[FF_VOLUMES + 1];

#    endif

int ff_mutex_create(int _vol) {
    // Win32
#    if OS_TYPE == 0
    Mutex[_vol] = CreateMutex(NULL, FALSE, NULL);
    return (int)(Mutex[_vol] != INVALID_HANDLE_VALUE);

    // uITRON
#    elif OS_TYPE == 1
    T_CMTX cmtx = { TA_TPRI, 1 };

    Mutex[_vol] = acre_mtx(&cmtx);
    return (int)(Mutex[_vol] > 0);

    // uC/OS-II
#    elif OS_TYPE == 2
    OS_ERR err;

    Mutex[_vol] = OSMutexCreate(0, &err);
    return (int)(err == OS_NO_ERR);

    // FreeRTOS
#    elif OS_TYPE == 3
    Mutex[_vol] = xSemaphoreCreateMutex();
    return (int)(Mutex[_vol] != NULL);

    // CMSIS-RTOS
#    elif OS_TYPE == 4
    osMutexDef(cmsis_os_mutex);

    Mutex[_vol] = osMutexCreate(osMutex(cmsis_os_mutex));
    return (int)(Mutex[vol] != NULL);

#    endif
}

void ff_mutex_delete(int _vol) {
    // Win32
#    if OS_TYPE == 0
    CloseHandle(Mutex[_vol]);

    // uITRON
#    elif OS_TYPE == 1
    del_mtx(Mutex[_vol]);

    // uC/OS-II
#    elif OS_TYPE == 2
    OS_ERR err;

    OSMutexDel(Mutex[_vol], OS_DEL_ALWAYS, &err);

    // FreeRTOS
#    elif OS_TYPE == 3
    vSemaphoreDelete(Mutex[_vol]);

    // CMSIS-RTOS
#    elif OS_TYPE == 4
    osMutexDelete(Mutex[_vol]);

#    endif
}

int ff_mutex_take(int _vol) {
    // Win32
#    if OS_TYPE == 0
    return (int)(WaitForSingleObject(Mutex[_vol], FF_FS_TIMEOUT)
                 == WAIT_OBJECT_0);

    // uITRON
#    elif OS_TYPE == 1
    return (int)(tloc_mtx(Mutex[_vol], FF_FS_TIMEOUT) == E_OK);

    // uC/OS-II
#    elif OS_TYPE == 2
    OS_ERR err;

        OSMutexPend(Mutex[_vol], FF_FS_TIMEOUT, &err));
        return (int)(err == OS_NO_ERR);

        // FreeRTOS
#    elif OS_TYPE == 3
    return (int)(xSemaphoreTake(Mutex[_vol], FF_FS_TIMEOUT) == pdTRUE);

    // CMSIS-RTOS
#    elif OS_TYPE == 4
    return (int)(osMutexWait(Mutex[_vol], FF_FS_TIMEOUT) == osOK);

#    endif
}

void ff_mutex_give(int _vol) {
    // Win32
#    if OS_TYPE == 0
    ReleaseMutex(Mutex[_vol]);

    // uITRON
#    elif OS_TYPE == 1
    unl_mtx(Mutex[_vol]);

    // uC/OS-II
#    elif OS_TYPE == 2
        OSMutexPost(Mutex[_vol]);

        // FreeRTOS
#    elif OS_TYPE == 3
    xSemaphoreGive(Mutex[_vol]);

    // CMSIS-RTOS
#    elif OS_TYPE == 4
    osMutexRelease(Mutex[_vol]);

#    endif
}

#endif /* FF_FS_REENTRANT */
