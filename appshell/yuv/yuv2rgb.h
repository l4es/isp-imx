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
#ifndef _APPSHELL_YUV_YUV2RGB_H_
#define _APPSHELL_YUV_YUV2RGB_H_

void convert422spToBGRX(unsigned char* yuv, unsigned char* bgr, int w, int h, int dw, int dh);

void convertYUYVToBGRX(unsigned char* yuv, unsigned char* bgr, int w, int h, int dw, int dh);

void convertNV12ToBGRX(unsigned char* yuv, unsigned char* bgr, int w, int h, int dw, int dh);

#ifdef APPMODE_V4L2
void convertRawToBGRX(unsigned char* dat, unsigned char* bgr, int w, int h, int dw, int dh, int fmt);
#endif
#endif  // _APPSHELL_YUV_YUV2RGB_H_
