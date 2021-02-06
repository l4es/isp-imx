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
 * @file    picture_buffer.c
 *
 * @brief   Implements helper functions around picture buffer meta data structure.
 *
 *****************************************************************************/

#include <common/return_codes.h>
#include <common/picture_buffer.h>
#include <ebase/trace.h>

/******************************************************************************
 * local macro definitions
 *****************************************************************************/

CREATE_TRACER(PIC_LOAD_INFO , "PIC-INFO: ", INFO,  0);
CREATE_TRACER(PIC_LOAD_ERROR , "PIC-ERROR: ", ERROR,  1);

/******************************************************************************
 * local type definitions
 *****************************************************************************/


/******************************************************************************
 * local variable declarations
 *****************************************************************************/


/******************************************************************************
 * local function prototypes
 *****************************************************************************/


/******************************************************************************
 * API Implementation below.
 * See header file for detailed comments.
 *****************************************************************************/

/*****************************************************************************
 *          PicBufIsConfigValid()
 *****************************************************************************/
RESULT PicBufIsConfigValid( PicBufMetaData_t *pPicBufMetaData)
{
    switch (pPicBufMetaData->Type)
    {
        case PIC_BUF_TYPE_DATA:
            switch (pPicBufMetaData->Layout)
            {
                case PIC_BUF_LAYOUT_COMBINED:
                    return RET_SUCCESS;
                case PIC_BUF_LAYOUT_SEMIPLANAR:
                case PIC_BUF_LAYOUT_PLANAR:
                    return RET_NOTSUPP;
                default:
                    return RET_OUTOFRANGE;
            };
            break;
        case PIC_BUF_TYPE_RAW8:
        case PIC_BUF_TYPE_RAW10:
        case PIC_BUF_TYPE_RAW12:
        case PIC_BUF_TYPE_RAW14:
        case PIC_BUF_TYPE_RAW16:
            switch (pPicBufMetaData->Layout)
            {
                case PIC_BUF_LAYOUT_BAYER_RGRGGBGB:
                case PIC_BUF_LAYOUT_BAYER_GRGRBGBG:
                case PIC_BUF_LAYOUT_BAYER_GBGBRGRG:
                case PIC_BUF_LAYOUT_BAYER_BGBGGRGR:
                case PIC_BUF_LAYOUT_COMBINED:
                    return RET_SUCCESS;
                case PIC_BUF_LAYOUT_SEMIPLANAR:
                case PIC_BUF_LAYOUT_PLANAR:
                    return RET_NOTSUPP;
                default:
                    return RET_OUTOFRANGE;
            };
            break;
        case PIC_BUF_TYPE_JPEG:
            switch (pPicBufMetaData->Layout)
            {
                case PIC_BUF_LAYOUT_COMBINED:
                    return RET_SUCCESS;
                case PIC_BUF_LAYOUT_SEMIPLANAR:
                case PIC_BUF_LAYOUT_PLANAR:
                    return RET_NOTSUPP;
                default:
                    return RET_OUTOFRANGE;
            };
            break;
        case PIC_BUF_TYPE_YCbCr444:
            switch (pPicBufMetaData->Layout)
            {
                case PIC_BUF_LAYOUT_COMBINED:
                case PIC_BUF_LAYOUT_PLANAR:
                    return RET_SUCCESS;
                case PIC_BUF_LAYOUT_SEMIPLANAR:
                    return RET_NOTSUPP;
                default:
                    return RET_OUTOFRANGE;
            };
            break;
        case PIC_BUF_TYPE_YCbCr422:
            switch (pPicBufMetaData->Layout)
            {
                case PIC_BUF_LAYOUT_COMBINED:
                case PIC_BUF_LAYOUT_SEMIPLANAR:
                case PIC_BUF_LAYOUT_PLANAR:
                    return RET_SUCCESS;
                default:
                    return RET_OUTOFRANGE;
            };
            break;
        case PIC_BUF_TYPE_YCbCr420:
            switch (pPicBufMetaData->Layout)
            {
                case PIC_BUF_LAYOUT_SEMIPLANAR:
                case PIC_BUF_LAYOUT_PLANAR:
                    return RET_SUCCESS;
                case PIC_BUF_LAYOUT_COMBINED:
                    return RET_NOTSUPP;
                default:
                    return RET_OUTOFRANGE;
            };
            break;
        case PIC_BUF_TYPE_RGB565:
        case PIC_BUF_TYPE_RGB666:
        case PIC_BUF_TYPE_RGB888:
            switch (pPicBufMetaData->Layout)
            {
                case PIC_BUF_LAYOUT_COMBINED:
                    return RET_SUCCESS;
                case PIC_BUF_LAYOUT_PLANAR:
                case PIC_BUF_LAYOUT_SEMIPLANAR:
                    return RET_NOTSUPP;
                default:
                    return RET_OUTOFRANGE;
            };
            break;
        default:
            break;
    }

    // invalid configuration
    return RET_OUTOFRANGE;
}

RESULT PicBufDump(PicBufMetaData_t *pPicBuffer, const uint8_t *logical, bool_t inputSource)
{
    FILE *result;
    uint32_t x, y;
    uint32_t planei;
    uint32_t totalPlanes;
    uint32_t imageBytes[3] = {0, 0, 0};
    uint32_t imageWidthBytes[3] = {0, 0, 0};
    uint32_t imageHeight[3] = {0, 0, 0};
    uint8_t *lineBaseAddress[3] = {NULL, NULL, NULL};
    uint8_t *baseAddress[3] = {NULL, NULL, NULL};

    TRACE(PIC_LOAD_INFO, "%s (enter)\n", __func__);

    if (inputSource)
    {
        result = fopen("./dump/input_buf.txt", "w+");
    }
    else
    {
        result = fopen("./dump/output_buf.txt", "w+");
    }

    if (NULL == result)
    {
        TRACE(PIC_LOAD_ERROR, "open file for write buffer content failed.\n");
        return RET_FAILURE;
    }

    switch (pPicBuffer->Type)
    {
    case PIC_BUF_TYPE_RAW8:
        {
            if ((pPicBuffer->Layout == PIC_BUF_LAYOUT_BAYER_RGRGGBGB)
                    || (pPicBuffer->Layout == PIC_BUF_LAYOUT_BAYER_GRGRBGBG)
                    || (pPicBuffer->Layout == PIC_BUF_LAYOUT_BAYER_GBGBRGRG)
                    || (pPicBuffer->Layout == PIC_BUF_LAYOUT_BAYER_BGBGGRGR))
            {
                imageBytes[0] = pPicBuffer->Data.raw.PicWidthBytes * pPicBuffer->Data.raw.PicHeightPixel;
                imageWidthBytes[0] = pPicBuffer->Data.raw.PicWidthBytes;
                imageHeight[0] = pPicBuffer->Data.raw.PicHeightPixel;
                baseAddress[0] = (uint8_t*)logical;
                totalPlanes = 1;

                for (planei = 0; planei < totalPlanes; planei++)
                {
                    for (y = 0; y < imageHeight[planei]; y++)
                    {
                       lineBaseAddress[0] = (baseAddress[planei] + y * imageWidthBytes[planei]);

                        for (x = 0; x < imageWidthBytes[planei]; x++)
                        {
                            fprintf(result, "%08x\n", *(lineBaseAddress[0] + x));
                        }
                    }
                }

            }
            else
            {
                fclose(result);
                return RET_NOTSUPP;
            }
            break;
        }

    case PIC_BUF_TYPE_RAW16:
    case PIC_BUF_TYPE_RAW10:
        {
            if ((pPicBuffer->Layout == PIC_BUF_LAYOUT_BAYER_RGRGGBGB)
                    || (pPicBuffer->Layout == PIC_BUF_LAYOUT_BAYER_GRGRBGBG)
                    || (pPicBuffer->Layout == PIC_BUF_LAYOUT_BAYER_GBGBRGRG)
                    || (pPicBuffer->Layout == PIC_BUF_LAYOUT_BAYER_BGBGGRGR))
            {
                imageBytes[0] = pPicBuffer->Data.raw.PicWidthBytes * pPicBuffer->Data.raw.PicHeightPixel;
                imageWidthBytes[0] = pPicBuffer->Data.raw.PicWidthBytes;
                imageHeight[0] = pPicBuffer->Data.raw.PicHeightPixel;
                baseAddress[0] = (uint8_t*)logical;
                totalPlanes = 1;
                for (planei = 0; planei < totalPlanes; planei++)
                {
                    for (y = 0; y < imageHeight[planei]; y++)
                    {
                        lineBaseAddress[0] = (baseAddress[planei] + y * imageWidthBytes[planei]);

                        /* need 2 vytes to store one piexl. */
                        for (x = 0; x < imageWidthBytes[planei]; x += 2)
                        {
                            fprintf(result, "%08x\n", *(lineBaseAddress[0] + x) <<8 | *(lineBaseAddress[0] + x + 1) );
                        }
                    }
                }

            }
            else
            {
                fclose(result);
                return RET_NOTSUPP;
            }
            break;
        }

    case PIC_BUF_TYPE_JPEG:
        {
            if (pPicBuffer->Layout == PIC_BUF_LAYOUT_COMBINED)
            {
                imageBytes[0] = pPicBuffer->Data.jpeg.DataSize;
                baseAddress[0] = (uint8_t*)logical;
                totalPlanes = 1;
                for (planei = 0; planei < totalPlanes; planei++)
                    {
                        for (x = 0; x < imageBytes[planei] / 4; x++)
                        {
                            fprintf(result, "%08x\n", *(baseAddress[planei] + x));
                        }
                    }

            }
            else
            {
                fclose(result);
                return RET_NOTSUPP;
            }
            break;
        }

    case PIC_BUF_TYPE_YCbCr444:
        {
            if (pPicBuffer->Layout == PIC_BUF_LAYOUT_PLANAR)
            {
                imageWidthBytes[0] = pPicBuffer->Data.YCbCr.semiplanar.Y.PicWidthBytes;
                imageHeight[0] = pPicBuffer->Data.YCbCr.semiplanar.Y.PicHeightPixel;
                imageWidthBytes[1] = pPicBuffer->Data.YCbCr.semiplanar.CbCr.PicWidthBytes;
                imageHeight[1] = pPicBuffer->Data.YCbCr.semiplanar.CbCr.PicHeightPixel;
                imageBytes[0] = pPicBuffer->Data.YCbCr.semiplanar.Y.PicWidthBytes * pPicBuffer->Data.YCbCr.semiplanar.Y.PicHeightPixel;
                baseAddress[0] = (uint8_t*)logical;
                baseAddress[1] = baseAddress[0] + imageBytes[0];
                totalPlanes = 2;

                for (planei = 0; planei < totalPlanes; planei++)
                {
                    for (y = 0; y < imageHeight[planei]; y++)
                    {
                        lineBaseAddress[planei] = baseAddress[planei] + y * imageWidthBytes[planei];
                        for (x = 0; x < imageWidthBytes[planei]; x++)
                        {
                            fprintf(result, "%08x\n", *(lineBaseAddress[planei] + x));
                        }
                    }
                }
            }
            else
            {
                fclose(result);
                return RET_NOTSUPP;
            }

            break;
        }

    case PIC_BUF_TYPE_YCbCr422:
        {
            if (pPicBuffer->Layout == PIC_BUF_LAYOUT_SEMIPLANAR)
            {
                imageWidthBytes[0] = pPicBuffer->Data.YCbCr.semiplanar.Y.PicWidthBytes;
                imageHeight[0] = pPicBuffer->Data.YCbCr.semiplanar.Y.PicHeightPixel;
                imageWidthBytes[1] = pPicBuffer->Data.YCbCr.semiplanar.CbCr.PicWidthBytes;
                imageHeight[1] = pPicBuffer->Data.YCbCr.semiplanar.CbCr.PicHeightPixel;
                imageBytes[0] = pPicBuffer->Data.YCbCr.semiplanar.Y.PicWidthBytes * pPicBuffer->Data.YCbCr.semiplanar.Y.PicHeightPixel;
                baseAddress[0] = (uint8_t*)logical;
                baseAddress[1] = baseAddress[0] + imageBytes[0];
                totalPlanes = 2;

                for (planei = 0; planei < totalPlanes; planei++)
                {
                    for (y = 0; y < imageHeight[planei]; y++)
                    {
                        lineBaseAddress[planei] = baseAddress[planei] + y * imageWidthBytes[planei];
                        for (x = 0; x < imageWidthBytes[planei]; x++)
                        {
                            fprintf(result, "%08x\n", *(baseAddress[planei] + x));
                        }
                    }
                }

            }
            else
            {
                fclose(result);
                return RET_NOTSUPP;
            }
            break;
        }

    case PIC_BUF_TYPE_YCbCr420:
    case PIC_BUF_TYPE_YCbCr400:
        {
            fclose(result);
            return RET_NOTSUPP;
        break;
        }

    default:
        {
            fclose(result);
            return RET_NOTSUPP;
        }

    }
    fclose(result);
    TRACE(PIC_LOAD_INFO, "%s (exit)\n", __func__);

    return RET_SUCCESS;
}
