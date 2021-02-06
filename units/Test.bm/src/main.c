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

#include "bm/base.h"
#include "parse.h"
#include "test.h"
#include <assert.h>

int32_t main(int32_t argc, char *argv[]) {
  TestCtx_t *pCtx = calloc(1, sizeof(TestCtx_t));
  assert(pCtx);

  pCtx->hHal = HalOpen();
  assert(pCtx->hHal);

  TST_Do(pCtx, BM_LIST_FILE);

  HalClose(pCtx->hHal);
  pCtx->hHal = NULL;

  free(pCtx);
  pCtx = NULL;

  return 0;
}
