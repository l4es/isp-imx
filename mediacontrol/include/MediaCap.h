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

#ifndef DEVELOPER_MEDIACONTROL_INCLUDE_MEDIACAP_H_
#define DEVELOPER_MEDIACONTROL_INCLUDE_MEDIACAP_H_

typedef struct MediaCap {
    int width;
    int height;
    int format;
    int fps;
    MediaCap& operator=(const MediaCap& cap) {
        width = cap.width;
        height = cap.height;
        format = cap.format;
        return *this;
    }
} MediaCap;

#endif  // DEVELOPER_MEDIACONTROL_INCLUDE_MEDIACAP_H_
