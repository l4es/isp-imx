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

#include "file/file.h"
#include <assert.h>
#include <ebase/types.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REPORT_DETAIL 0

static void ComposeYuvPlanar(const char *pY, uint64_t lengthY, const char *pCb,
                             uint64_t lengthCb, const char *pCr,
                             uint64_t lengthCr, const char *pYuv);
static void ComposeYuvSemiPlanar(const char *pY, uint64_t lengthY,
                                 const char *pCb, uint64_t lengthCb,
                                 const char *pCr, uint64_t lengthCr,
                                 const char *pYuv);
static void ComposeYuvInterleave(const char *pY, uint64_t length,
                                 const char *pYuv);
static void OutputBin(const char *pFileName, uint64_t length, FILE *pBin);

static int32_t openCount = 0;

FILE *FLE_Open(const char *pFileName, const char *pMode) {
  FILE *pF = fopen(pFileName, pMode);
  if (pF) {
    openCount++;
    // printf("open file: %d \n", openCount);
  } else {
    perror(pFileName);
  }

  return pF;
}

int32_t FLE_Close(FILE *pF) {
  int32_t ret = fclose(pF);
  if (!ret) {
    openCount--;
  } else {
    perror("Close file failed");
  }

  return ret;
}

int32_t FLE_OpenCount() { return openCount; }

int32_t FLE_CompareBuffer(FILE *pF, FILE *pF2, uint32_t *pPercent) {
  assert(pF);
  assert(pF2);

  int32_t ret = 0, ret2 = 0;

  uint64_t addr = 0, addr2 = 0;
  uint32_t value = 0, value2 = 0;

  bool isFirstReport = true;
  bool isLastEqual = true;

  uint64_t line = 0;
  uint64_t equalCount = 0;

  for (;; line++) {
    ret = fscanf(pF, "%lx %x", &addr, &value);
    if (ret <= 0) {
      break;
    }

    ret2 = fscanf(pF2, "%lx %x", &addr2, &value2);
    if (ret2 <= 0) {
      break;
    }

    if (value != value2) {
#if REPORT_DETAIL
      if (isFirstReport || isLastEqual) {
        isFirstReport = false;

        printf("%d: 0x%08X 0x%02X != 0x%08X 0x%02X \n", line + 1, addr, value,
               addr2, value2);
      }

      isLastEqual = false;
#endif
    } else {
      equalCount++;

#if REPORT_DETAIL
      if (isFirstReport || !isLastEqual) {
        isFirstReport = false;

        printf("%d: 0x%08X 0x%02X == 0x%08X 0x%02X \n", line + 1, addr, value,
               addr2, value2);
      }

      isLastEqual = true;
#endif
    }
  }

  if (!line) {
    return -ENOENT;
  } else if (pPercent) {
    *pPercent = equalCount * 100 / line;
  }

  if (equalCount == line) {
    return 0;
  } else {
    return -1;
  }
}

int32_t FLE_BinaryCompare(FILE *pF, FILE *pF2) {
  assert(pF);
  assert(pF2);

  size_t result = 0, result2 = 0;
  char c = 0, c2 = 0;

  for (;;) {
    result = fread(&c, 1, 1, pF);
    result2 = fread(&c2, 1, 1, pF2);

    if (result != result2) {
      return -1;
    } else if (result < 1) {
      return 0;
    } else if (c != c2) {
      return -1;
    }
  }

  return 0;
}

int32_t FLE_Size(const char *pFile, uint64_t *pSize) {
  assert(pFile);

  FILE *pF = FLE_Open(pFile, "r");
  if (!pF) {
    return -ENFILE;
  }

  fseek(pF, 0L, SEEK_END);

  if (pSize) {
    *pSize = ftell(pF);
  }

  FLE_Close(pF);

  return 0;
}

uint64_t FLE_LoadTxtBuffer(const char *pFileName, char *pBuffer,
                           uint64_t bufferSize) {
  assert(pFileName);
  assert(pBuffer);
  assert(bufferSize);

  uint8_t stride = 0;

  int32_t ret = FLE_GetBufferStride(pFileName, &stride);
  if (ret < 0) {
    return 0;
  }

  FILE *pF = FLE_Open(pFileName, "r");
  assert(pF);

  uint64_t addressOffset = 0;
  uint64_t addressStart = 0;
  uint64_t address = 0;
  uint32_t data = 0;
  uint64_t lineNumber = 0;

  for (; fscanf(pF, "%lx %x", &address, &data) > 0 &&
         lineNumber * stride < bufferSize;
       lineNumber++) {
    if (lineNumber == 0) {
      addressStart = address;
    }

    addressOffset = address - addressStart;

    for (uint8_t j = 0; j < stride; j++) {
      pBuffer[addressOffset + j] = data;

      if (stride > 1) {
        data = data >> 8;
      }
    }
  }

  FLE_Close(pF);

  return lineNumber * stride;
}

int32_t FLE_Lines(const char *pFile, uint64_t *pCount) {
  assert(pFile);

  FILE *pF = FLE_Open(pFile, "r");
  if (!pF) {
    return -ENFILE;
  }

  assert(pCount);
  *pCount = 0;

  signed char ch = 0;

  do {
    ch = fgetc(pF);

    if (ch == '\n') {
      *pCount += 1;
    }
  } while (ch != EOF);

  FLE_Close(pF);

  return 0;
}

int32_t FLE_GetBufferBaseAddress(const char *pFile, uint64_t *pAddr) {
  assert(pFile);

  FILE *pF = FLE_Open(pFile, "r");
  if (!pF) {
    return -ENFILE;
  }

  assert(pAddr);

  uint32_t value = 0;

  if (fscanf(pF, "%lx %x", pAddr, &value) <= 0) {
    FLE_Close(pF);
    return -1;
  }

  FLE_Close(pF);

  return 0;
}

int32_t FLE_GetBufferLastAddress(const char *pFile, uint64_t *pAddr) {
  assert(pFile);

  FILE *pF = FLE_Open(pFile, "r");
  if (!pF) {
    return -ENFILE;
  }

  assert(pAddr);

  uint32_t value = 0;

  while (fscanf(pF, "%lx %x", pAddr, &value) > 0) {
  }

  FLE_Close(pF);

  return 0;
}

int32_t FLE_GetBufferLength(const char *pFile, uint64_t *pLength) {
  int32_t ret = 0;

  uint64_t baseAddr = 0;

  ret = FLE_GetBufferBaseAddress(pFile, &baseAddr);
  if (ret != 0) {
    return ret;
  }

  uint64_t lastAddr = 0;

  ret = FLE_GetBufferLastAddress(pFile, &lastAddr);
  if (ret != 0) {
    return ret;
  }

  if (lastAddr <= baseAddr) {
    return -1;
  }

  assert(pLength);

  uint8_t stride = 0;

  ret = FLE_GetBufferStride(pFile, &stride);
  if (ret != 0) {
    return ret;
  }

  *pLength = lastAddr - baseAddr + stride;

  return ret;
}

int32_t FLE_GetBufferStride(const char *pFile, uint8_t *pStride) {
  assert(pFile);

  FILE *pF = FLE_Open(pFile, "r");
  if (!pF) {
    return -ENFILE;
  }

  uint64_t address = 0, address2 = 0;
  uint32_t value = 0;

  if (fscanf(pF, "%lx %x", &address, &value) > 0) {
    if (fscanf(pF, "%lx %x", &address2, &value) > 0) {
      *pStride = address2 - address;
    } else {
      FLE_Close(pF);

      return -1;
    }
  } else {
    FLE_Close(pF);

    return -2;
  }

  FLE_Close(pF);

  return 0;
}

void FLE_ComposeYuv(const char *pY, const char *pCb, const char *pCr,
                    const char *pYuv) {
  assert(pY);
  assert(pCb);
  assert(pCr);
  assert(pYuv);

  uint64_t lengthY = 0, lengthCb = 0, lengthCr = 0;

  FLE_GetBufferLength(pY, &lengthY);
  FLE_GetBufferLength(pCb, &lengthCb);
  FLE_GetBufferLength(pCr, &lengthCr);

  if (lengthY) {
    if (lengthCb) {
      if (lengthCr) {
        ComposeYuvPlanar(pY, lengthY, pCb, lengthCb, pCr, lengthCr, pYuv);
      } else {
        ComposeYuvSemiPlanar(pY, lengthY, pCb, lengthCb, pCr, lengthCr, pYuv);
      }
    } else {
      if (lengthCr) {
        ComposeYuvSemiPlanar(pY, lengthY, pCb, lengthCb, pCr, lengthCr, pYuv);
      } else {
        ComposeYuvInterleave(pY, lengthY, pYuv);
      }
    }
  } else {
    return;
  }
}

/*
Y:  X X X X
Cb: X X X X
Cr: X X X X
*/

static void ComposeYuvPlanar(const char *pY, uint64_t lengthY, const char *pCb,
                             uint64_t lengthCb, const char *pCr,
                             uint64_t lengthCr, const char *pYuv) {
  FILE *pBin = FLE_Open(pYuv, "wb");
  assert(pBin);

  OutputBin(pY, lengthY, pBin);
  OutputBin(pCb, lengthCb, pBin);
  OutputBin(pCr, lengthCr, pBin);

  FLE_Close(pBin);
}

/*
Y:  X X X X   X X X X
Cb: X X X X   - - - -
Cr: - - - -   X X X X
*/

static void ComposeYuvSemiPlanar(const char *pY, uint64_t lengthY,
                                 const char *pCb, uint64_t lengthCb,
                                 const char *pCr, uint64_t lengthCr,
                                 const char *pYuv) {
  FILE *pBin = FLE_Open(pYuv, "wb");
  assert(pBin);

  OutputBin(pY, lengthY, pBin);

  const char *pFileName = pCr;
  uint64_t length = lengthCr;

  if (lengthCb && !lengthCr) {
    pFileName = pCb;
    length = lengthCb;
  }

  char *pBuffer = calloc(length, 1);
  assert(pBuffer);

  assert(FLE_LoadTxtBuffer(pFileName, pBuffer, length) == length);

  for (uint64_t i = 0; i < length; i += 2) {
    fwrite(pBuffer + i, 1, 1, pBin);
  }

  for (uint64_t i = 1; i < length; i += 2) {
    fwrite(pBuffer + i, 1, 1, pBin);
  }

  free(pBuffer);

  FLE_Close(pBin);
}

/*
Y:  Y Cb Y Cr X X X X
Cb: - - - - - - - - -
Cr: - - - - - - - - -
*/

static void ComposeYuvInterleave(const char *pY, uint64_t length,
                                 const char *pYuv) {
  FILE *pBin = FLE_Open(pYuv, "wb");
  assert(pBin);

  char *pBuffer = calloc(length, 1);
  assert(pBuffer);

  assert(FLE_LoadTxtBuffer(pY, pBuffer, length) == length);

  for (uint64_t i = 0; i < length; i += 2) {
    fwrite(pBuffer + i, 1, 1, pBin);
  }

  for (uint64_t i = 1; i < length; i += 3) {
    fwrite(pBuffer + i, 1, 1, pBin);
  }

  for (uint64_t i = 3; i < length; i += 3) {
    fwrite(pBuffer + i, 1, 1, pBin);
  }

  free(pBuffer);

  FLE_Close(pBin);
}

static void OutputBin(const char *pFileName, uint64_t length, FILE *pBin) {
  char *pBuffer = calloc(length, 1);
  assert(pBuffer);

  assert(FLE_LoadTxtBuffer(pFileName, pBuffer, length) == length);

  fwrite(pBuffer, 1, length, pBin);
  free(pBuffer);
}
