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

#ifndef FILE__h
#define FILE__h

#ifdef __cplusplus
extern "C" {
#endif

#include <ebase/types.h>
#include <stdio.h>

FILE *FLE_Open(const char *pFileName, const char *pMode);
int32_t FLE_Close(FILE *pF);
int32_t FLE_OpenCount();

int32_t FLE_CompareBuffer(FILE *, FILE *, uint32_t *);
int32_t FLE_BinaryCompare(FILE *, FILE *);
void FLE_ComposeYuv(const char *pY, const char *pCb, const char *pCr,
                    const char *pYuv);
int32_t FLE_Size(const char *, uint64_t *);
int32_t FLE_Lines(const char *, uint64_t *);
int32_t FLE_GetBufferBaseAddress(const char *, uint64_t *);
int32_t FLE_GetBufferLastAddress(const char *, uint64_t *);
int32_t FLE_GetBufferLength(const char *, uint64_t *);
int32_t FLE_GetBufferStride(const char *pFile, uint8_t *pStride);
int32_t FLE_CopyFile(const char *sourceFileNameWithPath,
                     const char *targetFileNameWithPath);
uint64_t FLE_LoadTxtBuffer(const char *pFileName, char *pBuffer,
                           uint64_t bufferSize);

#ifdef __cplusplus
}
#endif

#endif
