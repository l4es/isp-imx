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
#ifndef _APPSHELL_V4L2DRMTEST_IDISPLAY_H_
#define _APPSHELL_V4L2DRMTEST_IDISPLAY_H_

#include "../yuv/yuv2rgb.h"

enum {
    VIV_DISPLAY_TYPE_EMPTY,
    VIV_DISPLAY_TYPE_DRM,
    VIV_DISPLAY_TYPE_FILE,
    VIV_DISPLAY_TYPE_FB,
};

class IDisplay {
public:
    virtual ~IDisplay() {}
    virtual void showBuffer(unsigned char* yuvData, int width, int height, int format, int size) = 0;
    static IDisplay* createObject(int type);
    int frameid = 0;
};

class EmptySink : public IDisplay {
public:
    EmptySink() {}
    void showBuffer(unsigned char* yuvData, int width, int height, int format, int size) {}
};

#endif  // _APPSHELL_V4L2DRMTEST_IDISPLAY_H_
