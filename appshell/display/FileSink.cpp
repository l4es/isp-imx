/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#ifdef APPMODE_V4L2
# include <linux/videodev2.h>
#endif

#include "log.h"

#include "FileSink.h"

#define LOGTAG "FileSink"

void FileSink::showBuffer(unsigned char* data, int width, int height, int format, int size) {
    if (!data)
        return;

    char szFile[32];
#ifdef APPMODE_V4L2
    if (format == V4L2_PIX_FMT_SBGGR8 ||
        format == V4L2_PIX_FMT_SGBRG8 ||
        format == V4L2_PIX_FMT_SGRBG8 ||
        format == V4L2_PIX_FMT_SRGGB8) {
        sprintf(szFile, "rawdump_%dx%d_%d.pgm", width, height, frameid++);
        FILE* pFile = fopen(szFile, "wb");
        if (pFile) {
            fprintf(pFile, "P5\n%d %d\n4095\n", width, height);
            fwrite(data, 1, width * height, pFile);
            fclose(pFile);
        }
    } else if (format == V4L2_PIX_FMT_SBGGR10 ||
        format == V4L2_PIX_FMT_SGBRG10 ||
        format == V4L2_PIX_FMT_SGRBG10 ||
        format == V4L2_PIX_FMT_SRGGB10 ||
        format == V4L2_PIX_FMT_SBGGR12 ||
        format == V4L2_PIX_FMT_SGBRG12 ||
        format == V4L2_PIX_FMT_SGRBG12 ||
        format == V4L2_PIX_FMT_SRGGB12) {
        sprintf(szFile, "rawdump_%dx%d_%d.pgm", width, height, frameid++);
        FILE* pFile = fopen(szFile, "wb");
        if (pFile) {
            fprintf(pFile, "P5\n%d %d\n4095\n", width, height);
            fwrite(data, 1, width * height * 2, pFile);
            fclose(pFile);
        }
    } else
#endif
    {
        sprintf(szFile, "test_frame%d.yuv", frameid++);
        FILE* pFile = fopen(szFile, "wb");
        if (pFile) {
            fwrite(data, 1, size, pFile);
            fclose(pFile);
        }
    }
}
