/******************************************************************************\
|* Copyright (c) 2020 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <isp_version.h>
#include <ic_dev.h>
#include <isp_ioctl.h>
#include <linux/videodev2.h>
#include <linux/media.h>
#include <poll.h>

#include "oslayer/oslayer.h"

#include "altera_fpga.h"

#include <ebase/trace.h>
#include <hal/hal_altera_pci.h>
#ifdef SUBDEV_CHAR
#include "soc_ioctl.h"
#endif
#include "viv_video_kevent.h"

CREATE_TRACER(FPGA_INFO , "FPGA: ", INFO,  0);
CREATE_TRACER(FPGA_DEBUG, "FPGA: ", INFO,  0);
CREATE_TRACER(FPGA_ERROR, "FPGA: ", ERROR, 1);

#define FPGA_DMA_EXPANSION        (FPGA_DATA_BLOCK_STRIDE / FPGA_DATA_BLOCK_SIZE)
#define FPGA_DMA_BUFFER_SIZE      (FPGA_DMA_BLOCK_SIZE * FPGA_DMA_EXPANSION)
#define FPGA_DATA_ITEM_SIZE_MASK  (FPGA_DATA_ITEM_SIZE - 1)
#define FPGA_DATA_BLOCK_SIZE_MASK (FPGA_DATA_BLOCK_SIZE - 1)
#define FPGA_DATA_ITEM_ADDR_MASK  (FPGA_DATA_BLOCK_SIZE_MASK & (~FPGA_DATA_ITEM_SIZE_MASK))

#define FPGA_MANGLE_DMA_ADDRESS(i) ( \
            /* block address */  ((i) & ~FPGA_DATA_BLOCK_SIZE_MASK) * FPGA_DMA_EXPANSION + \
            /* item  address */  ((i) & FPGA_DATA_ITEM_ADDR_MASK) + \
            /* byte  address */  FPGA_DATA_ITEM_SIZE_MASK - ((i) & FPGA_DATA_ITEM_SIZE_MASK) )

#define FPGA_ALIGN_BLOCK_SIZE(x) \
            (((x) + FPGA_DMA_SIZE_ALIGNMENT - 1) & (~(FPGA_DMA_SIZE_ALIGNMENT - 1)))

//#define ENABLE_IRQ

const AlteraFPGAPLL_t AlteraFPGAPLL_1 = { FPGA_REG_ADDRESS_MOD(3 * 4), (1 << 8), (1 << 9) };
const AlteraFPGAPLL_t AlteraFPGAPLL_2 = { FPGA_REG_ADDRESS_MOD(9 * 4), (1 << 6), (1 << 7) };

#define VIV_VIDIOC_QUERY_EXTMEM         _IOWR('V', BASE_VIDIOC_PRIVATE + 106, struct ext_buf_info)

/*******************************************************************************/

uint32_t AlteraFPGABoard_Open(HalHandle_t HalHandle) {
    // get context from handle & check
    HalContext_t *pHalCtx = (HalContext_t *)HalHandle;
    if (pHalCtx == NULL)
    {
        return ~0U;
    }

    if (pHalCtx->board_using == BOOL_TRUE) {
        return FPGA_RES_OK;
    }

    pHalCtx->irq_cancel_all = 0;
//    if (osMutexInit(&pHalCtx->poll_mutex) != OSLAYER_OK) {
//        return FPGA_RES_FAIL;
//    }

    if (pHalCtx->isp_fd < 0) {
        TRACE(FPGA_ERROR, "%s: failed to get isp fd!\n", __func__);
        return FPGA_RES_NODEV;
    }
    
#ifdef SUBDEV_CHAR
    struct isp_extmem_info ext_mem;
    ioctl(pHalCtx->isp_fd, ISPIOC_G_QUERY_EXTMEM, &ext_mem);
    pHalCtx->reservedMemBase = ext_mem.addr;
    pHalCtx->reservedMemSize = ext_mem.size;

    pHalCtx->extern_mem_virtual_base = (uint8_t*) mmap(0, pHalCtx->reservedMemSize, 
           PROT_READ | PROT_WRITE, MAP_SHARED, pHalCtx->isp_fd, pHalCtx->reservedMemBase);
    if (pHalCtx->extern_mem_virtual_base  == (void *)-1) {
        return FPGA_RES_FAIL;
    }
#endif

#ifdef SUBDEV_V4L2
    int video_fd;
    struct ext_buf_info ext_buf;
    int streamid;//= -2; /* ISP0: -2, ISP1: -3 */
    pHalCtx->extern_mem_virtual_base = NULL;
    streamid = (pHalCtx->isp_index == 0) ? -2 : -3;
    video_fd = HalGetFdHandle(HalHandle, HAL_MODULE_VIDEO);
    if (video_fd < 0) {
        TRACE(FPGA_ERROR, "%s: failed to get /dev/videoX!\n", __func__);
        return FPGA_RES_NODEV;
    }
    ioctl(video_fd, VIV_VIDIOC_S_STREAMID, &streamid);

#ifdef ENABLE_IRQ
    if (video_fd >= 0) {
        struct v4l2_event_subscription subscribe;
        memset(&subscribe, 0, sizeof(subscribe));
        subscribe.type = VIV_VIDEO_ISPIRQ_TYPE;
        subscribe.id = 0;
        if (ioctl(video_fd, VIDIOC_SUBSCRIBE_EVENT, &subscribe) < 0) {
            TRACE(FPGA_ERROR, "%s subscribe failed.\n", __func__);
        }
    }
#endif

    ioctl(video_fd, VIV_VIDIOC_QUERY_EXTMEM, &ext_buf);
    pHalCtx->reservedMemBase = ext_buf.addr;
    pHalCtx->reservedMemSize = ext_buf.size;

    TRACE(FPGA_INFO, "%s mem_file:%d mmap buffer.\n", __func__, video_fd);
    pHalCtx->extern_mem_virtual_base = (uint8_t*) mmap(0, pHalCtx->reservedMemSize,
                 PROT_READ | PROT_WRITE, MAP_SHARED, video_fd, pHalCtx->reservedMemBase);
    if (!pHalCtx->extern_mem_virtual_base || 
        (pHalCtx->extern_mem_virtual_base == MAP_FAILED)) {
        TRACE(FPGA_ERROR, "%s: failed to map data: 0x%08lx, 0x%08lx, %p!\n", __func__,
           pHalCtx->reservedMemBase, pHalCtx->reservedMemSize, (void*)pHalCtx->extern_mem_virtual_base);
        pHalCtx->extern_mem_virtual_base = NULL;
        return FPGA_RES_FAIL;
    }
#endif

    pHalCtx->board_using = BOOL_TRUE;
    return FPGA_RES_OK;
}


void AlteraFPGABoard_Reset(HalHandle_t HalHandle) {
    HalContext_t *pHalCtx = (HalContext_t *)HalHandle;
    if (pHalCtx == NULL)
    {
        return ;
    }

    if (pHalCtx->isp_fd < 0) {
        exit(1);
    }
    ioctl(pHalCtx->isp_fd, ISPIOC_RESET, 0);
}

/*******************************************************************************/

uint32_t AlteraFPGABoard_Close(HalHandle_t HalHandle) {
    HalContext_t *pHalCtx = (HalContext_t *)HalHandle;
    if (pHalCtx == NULL)
    {
        return ~0U;
    }

    if (pHalCtx->isp_fd < 0) {
        return FPGA_RES_NODEV;
    }

    pHalCtx->irq_cancel_all = 1;
    usleep(2 * FPGA_IRQ_POLL_INTERVAL);
//    osMutexDestroy(&pHalCtx->poll_mutex);
    if (pHalCtx->extern_mem_virtual_base) {
        munmap((void*) pHalCtx->extern_mem_virtual_base, pHalCtx->reservedMemSize);
    }
    
    pHalCtx->board_using = BOOL_FALSE;

    return FPGA_RES_OK;
}

/*******************************************************************************/

uint32_t AlteraFPGABoard_ReadBAR(int32_t isp_fd, uint32_t address) {
    struct isp_reg_t reg;
    reg.offset = address;

    if (isp_fd < 0) {
        return FPGA_READ_ERROR;
    }

    if (ioctl(isp_fd, ISPIOC_READ_REG, &reg) != 0 ) {
        return FPGA_READ_ERROR;
    }
    return reg.val;
}

/*******************************************************************************/

uint32_t AlteraFPGABoard_WriteBAR(int32_t isp_fd, uint32_t address, uint32_t data) {
    struct isp_reg_t reg;
    reg.offset = address;
    reg.val = data;
    if (isp_fd < 0) {
        return FPGA_READ_ERROR;
    }

    if (ioctl(isp_fd, ISPIOC_WRITE_REG, &reg) != 0) {
        return FPGA_READ_ERROR;
    }

    return FPGA_RES_OK;
}

/*******************************************************************************/

uint32_t AlteraFPGABoard_RawDMAWrite(HalHandle_t HalHandle, void* data, uint32_t address, uint32_t size) {
    HalContext_t *pHalCtx = (HalContext_t *)HalHandle;
    if (!pHalCtx)
    {
        return FPGA_RES_FAIL;
    }

    if (pHalCtx->isp_fd < 0) {
        return FPGA_RES_NODEV;
    }
#ifdef APPMODE_NATIVE
    memcpy(pHalCtx->extern_mem_virtual_base + (address - pHalCtx->reservedMemBase), data, size);
#endif
    return FPGA_RES_OK;
}

/*******************************************************************************/

uint32_t AlteraFPGABoard_RawDMARead(HalHandle_t HalHandle, void* data, uint32_t address, uint32_t size) {

    HalContext_t *pHalCtx = (HalContext_t *)HalHandle;
    if (!pHalCtx)
    {
        return FPGA_RES_FAIL;
    }

    TRACE(FPGA_INFO, "%s (data=%p, address=0x%08x, size=%d)\n", __func__, data, address, size);

    if (pHalCtx->isp_fd < 0) {
        TRACE(FPGA_INFO, "%s no device_file, 0x%x\n", __func__, (unsigned long)pHalCtx->isp_fd);
        return FPGA_RES_NODEV;
    }
#ifdef APPMODE_NATIVE
    memcpy(data, pHalCtx->extern_mem_virtual_base + (address - pHalCtx->reservedMemBase), size);
#endif
    TRACE(FPGA_INFO, "%s copy done\n", __func__);
    return FPGA_RES_OK;
}

uint32_t AlteraFPGABoard_SetupIRQ(fpga_irq_handle_t *irq, uint32_t mis_addr, uint32_t cis_addr, uint32_t timeout) {
    if (!irq) {
        return FPGA_RES_INVAL;
    }
    if (timeout > (0x7FFFFFFF / 1000)) {
        return FPGA_RES_INVAL;
    }
    irq->mis_addr = mis_addr;
    irq->cis_addr = cis_addr;
    irq->timeout = timeout * 1000;
    irq->cancel = 0;
    if (osMutexInit(&irq->poll_mutex) != OSLAYER_OK) {
        return FPGA_RES_FAIL;
    }

    return FPGA_RES_OK;
}

/*******************************************************************************/

uint32_t AlteraFPGABoard_StopIRQ(fpga_irq_handle_t *irq) {
    if (!irq) {
        return FPGA_RES_INVAL;
    }
    osMutexLock(&irq->poll_mutex);
    irq->mis_addr = 0;
    irq->cis_addr = 0;
    irq->timeout = 0;
    irq->cancel = 1;
    osMutexUnlock(&irq->poll_mutex);

    osMutexDestroy(&irq->poll_mutex);
    
    return FPGA_RES_OK;
}

#ifdef ENABLE_IRQ
uint32_t AlteraFPGABoard_IRQPolling(fpga_irq_handle_t *irq, int fd) {
    int rc;
    struct pollfd pollfds;
    uint32_t num_fds = 1;
    uint32_t irq_val = 0;
    struct v4l2_event event;
    struct isp_irq_data *irq_data;

    if (fd <= 0)
        return irq_val;

    pollfds.fd = fd;
    pollfds.events = POLLIN|POLLPRI;
    rc = poll(&pollfds, (nfds_t)num_fds, 5/*FPGA_IRQ_POLL_INTERVAL*/);
    if (rc > 0) {
        rc = ioctl(fd, VIDIOC_DQEVENT, &event);
        if (rc < 0)
            return irq_val;
        irq_data = (struct isp_irq_data *)(&event.u.data[0]);
        irq_val = irq_data->val;
        TRACE(FPGA_INFO, "v4l2 event type:0x%x, addr:0x%x, mis_val:0x%x.\n", event.type, irq_data->addr, irq_data->val);
    }
    return irq_val;
}
#endif

/*******************************************************************************/

uint32_t AlteraFPGABoard_WaitForIRQ(HalIrqCtx_t *pIrqCtx, uint32_t irq_src, uint32_t *irq_status) {

    uint32_t mis = 0;
    int32_t time_left;
    fpga_irq_handle_t *irq ;
    HalContext_t *pHalCtx;
    
    if (!pIrqCtx) {
        return FPGA_RES_INVAL;
    }
    irq = &pIrqCtx->AlteraIrqHandle;
    
    pHalCtx = (HalContext_t *)pIrqCtx->HalHandle;
    if(pHalCtx == NULL)
    {
        return FPGA_RES_INVAL;
    }

    if (pHalCtx->isp_fd <= 0) {
        return FPGA_RES_NODEV;
    }

    if (!irq->mis_addr && !irq->cis_addr) {
        return FPGA_RES_INVAL;
    }
    time_left = (int32_t) irq->timeout;

#ifdef ENABLE_IRQ
    int video_fd = HalGetFdHandle(pIrqCtx->HalHandle, HAL_MODULE_VIDEO);
#endif

    while (!irq->cancel && !pHalCtx->irq_cancel_all && (time_left >= 0)) {
#ifdef ENABLE_IRQ
        osMutexLock(&irq->poll_mutex);
        if (irq_src == eHalIsrSrcIspIrq) {
            mis = AlteraFPGABoard_IRQPolling(irq, video_fd);
            if (mis) {
                osMutexUnlock(&irq->poll_mutex);
                break;
            }
            osMutexUnlock(&irq->poll_mutex);
            if (irq->timeout) {
                time_left -= FPGA_IRQ_POLL_INTERVAL;
            }
        } else {
            mis = AlteraFPGABoard_ReadReg(pHalCtx->isp_fd, irq->mis_addr);
            if (mis) {
                AlteraFPGABoard_WriteReg(pHalCtx->isp_fd, irq->cis_addr, mis);
                osMutexUnlock(&irq->poll_mutex);
                break;
            }
            osMutexUnlock(&irq->poll_mutex);
            usleep(FPGA_IRQ_POLL_INTERVAL);
            if (irq->timeout) {
                time_left -= FPGA_IRQ_POLL_INTERVAL;
            }
        }
#else
        osMutexLock(&irq->poll_mutex);
        mis = AlteraFPGABoard_ReadReg(pHalCtx->isp_fd, irq->mis_addr);
        if (mis) {
            AlteraFPGABoard_WriteReg(pHalCtx->isp_fd, irq->cis_addr, mis);
            osMutexUnlock(&irq->poll_mutex);
            break;
        }
        osMutexUnlock(&irq->poll_mutex);
        usleep(FPGA_IRQ_POLL_INTERVAL);
        if (irq->timeout) {
            time_left -= FPGA_IRQ_POLL_INTERVAL;
        }
#endif
    }

    //if(mis != 0) TRACE(FPGA_INFO, "##IRQ read mis: 0x%x\n", mis);
    if (irq_status) {
        *irq_status = mis;
    }
    return (mis && !irq->cancel && !pHalCtx->irq_cancel_all) ? FPGA_RES_OK : FPGA_RES_FAIL;
}

/*******************************************************************************/

uint32_t AlteraFPGABoard_CancelIRQ(fpga_irq_handle_t *irq) {
    if (!irq) {
        return FPGA_RES_INVAL;
    }
    irq->cancel = 1;
    usleep(2 * FPGA_IRQ_POLL_INTERVAL);
    return FPGA_RES_OK;
}
void AlteraFPGABoard_ForbidIRQPolling(fpga_irq_handle_t *irq) {
    osMutexLock(&irq->poll_mutex);
}

void AlteraFPGABoard_AllowIRQPolling(fpga_irq_handle_t *irq) {
    osMutexUnlock(&irq->poll_mutex);
}

uint32_t AlteraFPGABoard_SetPLLConfig(const AlteraFPGAPLL_t* pll, const AlteraFPGAPLLConfig_t* cfg) {
    return 0;
}
