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

/**
 * @file readpgmraw.cpp
 *
 * @brief
 *     Implementation of read DCT PGM Raw format.
 *
 *****************************************************************************/
#include "cam_device_api.hpp"

#include "macros.hpp"
#include <ebase/trace.h>
#include <fstream>
#include <iostream>

CREATE_TRACER(CAM_API_READPGMRAW_INF, "CAM_API_READPGMRAW: ", INFO, 0);
CREATE_TRACER(CAM_API_READPGMRAW_ERR, "CAM_API_READPGMRAW: ", ERROR, 1);

static bool readHeader(std::ifstream &file, uint32_t &width, uint32_t &height,
                        uint32_t &lineSize, PicBufType_t &type,
                        PicBufLayout_t &layout, int64_t &timeStamp) {
    std::string line;
    std::string delimit = " \t\n";
    std::string word;
    size_t pos = 0;
    int32_t step = 0;
    while (true) {
        std::getline(file, line);
        pos = 0;

        if (file.eof()) {
            TRACE(CAM_API_READPGMRAW_ERR, "%s (end of file)\n", __func__);
            return false;
        }

        if (line[0] == '#') {
            // check for DCT Raw
            {
                size_t n = line.find("<Type>");
                if (std::string::npos != n) {
                    size_t k = line.find("</Type>");
                    if (std::string::npos != k) {
                        n += 6;
                        DCT_ASSERT(n < k);
                        word = line.substr(n, k - n);
                        type = (PicBufType_t)atoi(word.c_str());
                    }
                }
            }
            {
                size_t n = line.find("<Layout>");
                if (std::string::npos != n) {
                    size_t k = line.find("</Layout>");
                    if (std::string::npos != k) {
                        n += 8;
                        DCT_ASSERT(n < k);
                        word = line.substr(n, k - n);
                        layout = (PicBufLayout_t)atoi(word.c_str());
                    }
                }
            }
            {
                size_t n = line.find("<TimeStampUs>");
                if (std::string::npos != n) {
                    size_t k = line.find("</TimeStampUs>");
                    if (std::string::npos != k) {
                        n += 13;
                        DCT_ASSERT(n < k);
                        word = line.substr(n, k - n);
                        timeStamp = (int64_t)atoll(word.c_str());
                    }
                }
            }

            continue;
        }

        if (step == 0) {
            // find word
            size_t n = line.find_first_not_of(delimit, pos);
            if (std::string::npos == n) {
                continue;
            }
            size_t k = line.find_first_of(delimit, n);
            word = line.substr(n, k - n);
            pos = k;

            if (word != "P5") {
                TRACE(CAM_API_READPGMRAW_ERR, "%s (wrong magic number)\n",
                            __func__);
                return false;
            }
            step = 1;
        }

        if (step == 1) {
            // find word
            size_t n = line.find_first_not_of(delimit, pos);
            if (std::string::npos == n) {
                continue;
            }
            size_t k = line.find_first_of(delimit, n);
            word = line.substr(n, k - n);
            pos = k;

            width = atoi(word.c_str());
            step = 2;
        }

        if (step == 2) {
            // find word
            size_t n = line.find_first_not_of(delimit, pos);
            if (std::string::npos == n) {
                continue;
            }
            size_t k = line.find_first_of(delimit, n);
            word = line.substr(n, k - n);
            pos = k;

            height = atoi(word.c_str());
            step = 3;
        }

        if (step == 3) {
            // find word
            size_t n = line.find_first_not_of(delimit, pos);
            if (std::string::npos == n) {
                continue;
            }
            size_t k = line.find_first_of(delimit, n);
            word = line.substr(n, k - n);
            pos = k;

            lineSize = atoi(word.c_str()) > 255 ? width << 1 : width;
            break;
        }
    }

    return true;
}

static bool readData(std::ifstream &file, uint32_t width, uint32_t height, uint32_t lineSize, uint8_t *pBuffer) {
    UNUSED_PARAM(width);

    uint8_t *pStart = pBuffer;

    for (uint32_t j = 0; j < height; ++j, pStart += lineSize) {
        file.read((char *)pStart, lineSize);

        if (file.eof()) {
            TRACE(CAM_API_READPGMRAW_ERR, "%s (end of file)\n", __func__);
            return false;
        }
    }

    return true;
}

bool PGM_ReadRaw(std::string fileName, PicBufMetaData_t *pPicBuf) {
    MEMSET(pPicBuf, 0, sizeof(PicBufMetaData_t));

    std::ifstream input;
    input.open(fileName, std::ios::binary);

    if (!input.good()) {
        TRACE(CAM_API_READPGMRAW_ERR, "%s (can't open the input file %s)\n", __func__, fileName.c_str());
        return false;
    }

    // read header
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t lineSize = 0;
    PicBufType_t type = PIC_BUF_TYPE_INVALID;
    PicBufLayout_t layout = PIC_BUF_LAYOUT_INVALID;
    int64_t timeStamp = 0;

    if (!readHeader(input, width, height, lineSize, type, layout, timeStamp)) {
        TRACE(CAM_API_READPGMRAW_ERR, "%s (can't read the image header)\n", __func__);
        input.close();
        return false;
    }

    // allocate memory
    uint8_t *pBuffer = (uint8_t *)malloc(lineSize * height);
    if (NULL == pBuffer) {
        TRACE(CAM_API_READPGMRAW_ERR, "%s (can't allocate memory)\n", __func__);
        input.close();
        return false;
    }

    // read data
    switch (type) {
    case PIC_BUF_TYPE_RAW8:
    case PIC_BUF_TYPE_RAW16: {
        if (!readData(input, width, height, lineSize, pBuffer)) {
            TRACE(CAM_API_READPGMRAW_ERR, "%s (can't read the image data)\n",
                        __func__);
            input.close();
            free(pBuffer);
            return false;
        }
        break;
    }
    default: {
        TRACE(CAM_API_READPGMRAW_ERR, "%s (unknown image format)\n", __func__);
        input.close();
        free(pBuffer);
        return false;
    }
    }

    input.close();

    pPicBuf->Type = type;
    pPicBuf->Layout = layout;
    pPicBuf->TimeStampUs = timeStamp;

    pPicBuf->Data.raw.pData = pBuffer;
    pPicBuf->Data.raw.PicWidthPixel = width;
    pPicBuf->Data.raw.PicWidthBytes = lineSize;
    pPicBuf->Data.raw.PicHeightPixel = height;

    return true;
}
