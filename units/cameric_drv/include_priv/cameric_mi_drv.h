/******************************************************************************\
|* Copyright 2010, Dream Chip Technologies GmbH. used with permission by      *|
|* VeriSilicon.                                                               *|
|* Copyright (c) <2020> by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/* VeriSilicon 2020 */

/**
 * @file cameric_mi_drv.h
 *
 * @brief
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup cameric_mi_drv CamerIc MI Driver Internal API
 * @{
 *
 */
#ifndef __CAMERIC_MI_DRV_H__
#define __CAMERIC_MI_DRV_H__

#include <ebase/types.h>
#include <common/align.h>
#include <common/list.h>
#include <common/picture_buffer.h>

#include <hal/hal_api.h>
#include "cameric_drv_cb.h"
#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_mi_drv_api.h>

#ifdef ISP_DEC
#include "cameric_isp_dec_drv_api.h"
#define TILE_SIZE 128
#define TILE_STATUS_BIT 4
#endif


/*******************************************************************************
 *          CamerIcMiDataPathContext_t
 *
 * @brief
 *
 * @note
 */
typedef struct CamerIcMiDataPathContext_s
{
    CamerIcMiDataMode_t             out_mode;                   /**< output format */
    CamerIcMiDataMode_t             in_mode;                    /**< input format */
    CamerIcMiDataLayout_t           datalayout;                 /**< layout of data */
    CamerIcMiDataAlignMode_t alignMode;

#ifdef ISP_MI_FIFO_DEPTH_NANO
    CamerIcMiMpOutputFifoDepth_t    mp_output_fifo_depth;
#endif
#ifdef ISP_MI_ALIGN_NANO
    CamerIcMiMpOutputLsbAlign_t     mp_lsb_alignment;
#endif
#ifdef ISP_MI_BYTESWAP
    CamerIcMiMpOutputSwapByte_t     mp_byte_swap_cfg;
#endif
#ifdef ISP_MI_HANDSHAKE_NANO
    CamerIcMiMpHandshake_t          mp_handshk_cfg;
#endif

    uint32_t                        in_width;
    uint32_t                        in_height;
    uint32_t                        out_width;
    uint32_t                        out_height;

    CamerIcMiOrientation_t          orientation;
    bool_t                          prepare_rotation;

    bool_t                          hscale;
    bool_t                          vscale;

    ScmiBuffer                      *pShdBuffer;
    ScmiBuffer                      *pBuffer;
} CamerIcMiDataPathContext_t;



/*******************************************************************************
 *
 *          CamerIcMiContext_t
 *
 * @brief   Internal MI driver context structure.
 *
 *****************************************************************************/
typedef struct CamerIcMiContext_s
{
    HalIrqCtx_t                     HalIrqCtx;

    CamerIcMiBurstLength_t          y_burstlength;
    CamerIcMiBurstLength_t          c_burstlength;

    CamerIcRequestCb_t              RequestCb;
    CamerIcEventCb_t                EventCb;

    CamerIcMiDataPathContext_t      PathCtx[CAMERIC_MI_PATH_MAX];

    uint32_t                        numFramesToSkip;

    //for dynamic dump mi buffer in sw-test
    char*                 regDynMpBufDumpName;         //!< mi mp buffer dump name.
    FILE*                 regDynMpBufDumpFp;           //!< dynamic mp dump handle

    void *                regDynMpYBufBase;
    uint32_t              regDynMpYBufSize;

    void *                regDynMpCbBufBase;
    uint32_t              regDynMpCbBufSize;

    void *                regDynMpCrBufBase;
    uint32_t              regDynMpCrBufSize;

    char*                 regDynSpBufDumpName;         //!< mi sp buffer dump name.
    FILE*                 regDynSpBufDumpFp;           //!< dynamic sp dump handle

    void *                regDynSpYBufBase;
    uint32_t              regDynSpYBufSize;

    void *                regDynSpCbBufBase;
    uint32_t              regDynSpCbBufSize;

    void *                regDynSpCrBufBase;
    uint32_t              regDynSpCrBufSize;

    char*                 regDynSp2BufDumpName;         //!< mi sp2 buffer dump name.
    FILE*                 regDynSp2BufDumpFp;           //!< dynamic sp2 dump handle

    char*                 regDynRdiBufDumpName;         //!< mi RDI buffer dump name.
    FILE*                 regDynRdiBufDumpFp;           //!< dynamic RDI dump handle

    char*                 regDynMetaBufDumpName;         //!< mi meta buffer dump name.
    FILE*                 regDynMetaBufDumpFp;           //!< dynamic meta dump handle

    void *                regDynSp2YBufBase;
    uint32_t              regDynSp2YBufSize;

    void *                regDynSp2CbBufBase;
    uint32_t              regDynSp2CbBufSize;

    void *                regDynSp2CrBufBase;
    uint32_t              regDynSp2CrBufSize;

    //for runtime dump frame-SP
    uint32_t              regDynSpBufDumpCnt;          //!< runtime cfg, total counter for sp dump
    uint32_t              regDynSpBufDumpSkip;         //!< runtime cfg, skip frames for storage
    char                  regDynSpBufRuntimeName[32];

    uint32_t              regDynSpBufDumpIdx;          //!< runtime cfg, total counter for sp dump
    uint32_t              regDynSpBufSkipCnt;          //!< runtime cfg, skip counter

    //for runtime dump frame-SP2
    uint32_t              regDynSp2BufDumpCnt;         //!< runtime cfg, total counter for sp2 dump
    uint32_t              regDynSp2BufDumpSkip;        //!< runtime cfg, skip frames for storage
    char                  regDynSp2BufRuntimeName[32];

    uint32_t              regDynSp2BufDumpIdx;         //!< runtime cfg, total counter for sp2 dump
    uint32_t              regDynSp2BufSkipCnt;         //!< runtime cfg, skip counter

    //for runtime dump frame-MP
    uint32_t              regDynMpBufDumpCnt;          //!< runtime cfg, total counter for mp dump
    uint32_t              regDynMpBufDumpSkip;         //!< runtime cfg, skip frames for storage
    char                  regDynMpBufRuntimeName[32];

    uint32_t              regDynMpBufDumpIdx;          //!< runtime cfg, total counter for mp dump
    uint32_t              regDynMpBufSkipCnt;          //!< runtime cfg, skip counter

    //for runtime dump frame-Rdi
    uint32_t              regDynRdiBufDumpCnt;         //!< runtime cfg, total counter for RDI dump
    uint32_t              regDynRdiBufDumpSkip;        //!< runtime cfg, skip frames for storage
    char                  regDynRdiBufRuntimeName[32];

    uint32_t              regDynRdiBufDumpIdx;         //!< runtime cfg, total counter for RDI dump
    uint32_t              regDynRdiBufSkipCnt;         //!< runtime cfg, skip counter

    //for runtime dump frame-Meta
    uint32_t              regDynMetaBufDumpCnt;        //!< runtime cfg, total counter for Meta dump
    uint32_t              regDynMetaBufDumpSkip;           //!< runtime cfg, skip frames for storage
    char                  regDynMetaBufRuntimeName[32];

    uint32_t              regDynMetaBufDumpIdx;        //!< runtime cfg, total counter for Meta dump
    uint32_t              regDynMetaBufSkipCnt;        //!< runtime cfg, skip counter

} CamerIcMiContext_t;

typedef struct CamerIcMiBufSlice_s
{
    uint32_t baseY;
    uint32_t sizeY;
    void * pBufY;

    uint32_t baseCb;
    uint32_t sizeCb;
    void * pBufCb;

    uint32_t baseCr;
    uint32_t sizeCr;
    void * pBufCr;
}CamerIcMiBufSlice_t;

/*****************************************************************************/
/**
 *          CamerIcMiInit()
 *
 * @brief   Initialize CamerIc MI driver context
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcMiInit
(
    CamerIcDrvHandle_t  handle
);



/*****************************************************************************/
/**
 *          CamerIcMiRelease()
 *
 * @brief   Release/Free CamerIc MI driver context
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcMiRelease
(
    CamerIcDrvHandle_t  handle
);



/*****************************************************************************/
/**
 *          CamerIcMiStart()
 *
 * @brief   Initialize and start MI interrupt handling
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcMiStart
(
    CamerIcDrvHandle_t  handle
);



/*****************************************************************************/
/**
 *          CamerIcMiStop()
 *
 * @brief   Stop and release MI interrupt handling
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcMiStop
(
    CamerIcDrvHandle_t  handle
);



/*****************************************************************************/
/**
 *          CamerIcMiStartLoadPicture()
 *
 * @brief
 *
 * @return              Return the result of the function call.
 * @retval              RET_SUCCESS
 * @retval              RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcMiStartLoadPicture
(
    CamerIcDrvHandle_t      handle,
    PicBufMetaData_t        *pPicBuffer
);




/*****************************************************************************/
/**
 * @brief   Set/Program picture buffer addresses to CamerIc registers.
 *
 * @param   handle          CamerIc driver handle
 * @param   path            Path Index (main- or selfpath)
 * @param   pPicBuffer      Pointer to picture buffer
 *
 * @return                  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
extern RESULT CamerIcMiSetBuffer
(
    CamerIcDrvHandle_t      handle,
    const CamerIcMiPath_t   path,
    const PicBufMetaData_t  *pPicBuffer
);

/*****************************************************************************/
/**
 * @brief   Dump MP picture buffer data to file, must be configured by CamEngineMiM(S)pDumpBufCfg()
 *
 * @param   handle          CamerIc driver handle
  * @param   path            Path Index (main- or selfpath)
 * @param   pPicBuffer      Pointer to picture buffer
 *
 * @return                  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
extern RESULT CamerIcMiDumpBuffer
(
    CamerIcDrvHandle_t      handle,
    const CamerIcMiPath_t   path,
    const PicBufMetaData_t  *pPicBuffer
);

int32_t CamerIcMiIrq
(
    void *pArg
);

/*****************************************************************************/
/**
 *          CamerIcSetupPictureBuffer()
 *
 * @brief   CamerIcSetupPictureBuffer
 *
 * @return              Return the result of the function call.
 *
 *****************************************************************************/
RESULT CamerIcSetupPictureBuffer
(
    CamerIcDrvHandle_t      handle,
    const CamerIcMiPath_t   path,
    ScmiBuffer             *pBuffer
);


/*****************************************************************************/
/**
 *          CamerIcUpdateMetaBuffer()
 *
 * @brief   CamerIcUpdateMetaBuffer
 *
 * @return              Return the result of the function call.
 *
 *****************************************************************************/
RESULT CamerIcUpdateMetaBuffer
(
    CamerIcDrvHandle_t      handle,
    const CamerIcMiPath_t   path,
    ScmiBuffer             *pBuffer
);

/*****************************************************************************/
/**
 *          CamerIcRequestAndSetupBuffers()
 *
 * @brief   CamerIcRequestAndSetupBuffers
 *
 * @return              Return the result of the function call.
 * @retval              0
 *
 *****************************************************************************/
RESULT CamerIcRequestAndSetupBuffers
(
    CamerIcDrvHandle_t      handle,
    const CamerIcMiPath_t   path
);

/******************************************************************************
 * CamerIcEnableScaling()
 *****************************************************************************/
RESULT CamerIcEnableScaling
(
    void     *ctx,
    const CamerIcMiPath_t   path
);

/******************************************************************************
 * CamerIcDisableScaling()
 *****************************************************************************/
RESULT CamerIcDisableScaling
(
    void     *ctx,
    const CamerIcMiPath_t   path
);

/*****************************************************************************/
/**
 *          CamerIcFlushAllBuffers()
 *
 * @brief   CamerIcFlushAllBuffers
 *
 * @return              Return the result of the function call.
 * @retval              0
 *
 *****************************************************************************/
RESULT CamerIcFlushAllBuffers
(
    const CamerIcMiPath_t   path,
    CamerIcMiContext_t      *ctx
);


/*****************************************************************************/
/**
 * @brief   Configuration of MP picture buffer Dump functions
 *
 * @param   handle          CamerIc driver handle
  *
 * @return                  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
extern RESULT CamerIcMiMpDumpCfgRuntime
(
    CamerIcDrvHandle_t       hCamerIcDrv
);


/*****************************************************************************/
/**
 * @brief   MP picture buffer Dump functions
 *
 * @param   handle          CamerIc driver handle
 * @param   pPicBuffer      Pointer to picture buffer
 *
 * @return                  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
/*Runtime capture MI output dump file*/
extern RESULT CamerIcMiMpDumpRuntime
(
    CamerIcDrvHandle_t       hCamerIcDrv,
    const PicBufMetaData_t  *pPicBuffer

);


/*****************************************************************************/
/**
 * @brief   Configuration of SP picture buffer Dump functions
 *
 * @param   handle          CamerIc driver handle
  *
 * @return                  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
extern RESULT CamerIcMiSpDumpCfgRuntime
(
    CamerIcDrvHandle_t       hCamerIcDrv
);

/*****************************************************************************/
/**
 * @brief   Configuration of SP2 picture buffer Dump functions
 *
 * @param   handle          CamerIc driver handle
  *
 * @return                  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
extern RESULT CamerIcMiSp2DumpCfgRuntime
(
    CamerIcDrvHandle_t       hCamerIcDrv
);


/*****************************************************************************/
/**
 * @brief   SP picture buffer Dump functions
 *
 * @param   handle          CamerIc driver handle
 * @param   pPicBuffer      Pointer to picture buffer
 *
 * @return                  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
/*Runtime capture MI output dump file*/
extern RESULT CamerIcMiSpDumpRuntime
(
    CamerIcDrvHandle_t       hCamerIcDrv,
    const PicBufMetaData_t  *pPicBuffer

);


/*****************************************************************************/
/**
 * @brief   SP2 picture buffer Dump functions
 *
 * @param   handle          CamerIc driver handle
 * @param   pPicBuffer      Pointer to picture buffer
 *
 * @return                  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
/*Runtime capture MI output dump file*/
extern RESULT CamerIcMiSp2DumpRuntime
(
    CamerIcDrvHandle_t       hCamerIcDrv,
    const PicBufMetaData_t  *pPicBuffer

);

/*****************************************************************************/
/**
 * @brief  internal functions for dump
 *
 * @param   pbuf_slice            output data
 * @param   pPicBuffer            Pointer to picture buffer
 *
 * @return                        Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT CamerIcMiGetSliceFromBuf(
    CamerIcDrvHandle_t      handle,
    CamerIcMiBufSlice_t * pbuf_slice,
    const PicBufMetaData_t  *pPicBuffer
);

/*****************************************************************************/
/**
 * @brief  internal functions for dump
 *
 * @param   name  file name
 * @param   cnt   dump counter
 * @param   skip  skip counter
 * @return                    Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
int GetMpMiCfg(char * name, unsigned int * cnt, unsigned int * skip);

/*****************************************************************************/
/**
 * @brief  internal functions for dump
 *
 * @param   name  file name
 * @param   cnt   dump counter
 * @param   skip  skip counter
 * @return                    Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
int GetSpMiCfg(char * name, unsigned int * cnt, unsigned int * skip);

/*****************************************************************************/
/**
 * @brief  internal functions for dump
 *
 * @param   name  file name
 * @param   cnt   dump counter
 * @param   skip  skip counter
 * @return                    Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
int GetSp2MiCfg(char * name, unsigned int * cnt, unsigned int * skip);
/* @} cameric_mi_drv */

void clearExtBuffer();

#endif /* __MRV_MI_DRV_H__ */
