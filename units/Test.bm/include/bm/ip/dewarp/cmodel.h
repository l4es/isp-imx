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

#if !defined __DEWARP_CMODEL_H__
#define __DEWARP_CMODEL_H__

#include "bm/base.h"

void CMDA_Create();

void CMDA_Free();

void CMDA_Input(const char *pDataFileName, const char *pDataFileName1,
                const char *pMapFileName, const char *pMapFileName1);

void CMDA_Output();

int32_t CMDA_Refresh(int32_t ispIndex, char *pCmdFile, cJSON *pReport);

#endif
