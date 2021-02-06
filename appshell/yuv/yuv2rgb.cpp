
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
#ifdef APPMODE_V4L2
# include <linux/videodev2.h>
#endif

#define CLIP_COLOR(x)  x > 255 ? 255 :  x < 0  ? 0 : x;

void convert422spToBGRX(unsigned char* yuv, unsigned char* bgr, int w, int h, int dw, int dh) {
    unsigned char* puv = yuv + w*h;
    int U = 0;
    int V = 0;
    float xscale = dw;
    xscale /= w;
    float yscale = dh;
    yscale /= h;

    for (int i = 0; i < h; i++) {
        int dy = i*yscale;
        for (int j = 0; j < w; j++) {
            int dx = j*xscale;
            int dpos = (dy*dw + dx) << 2;
            int Y = yuv[i*w+j];
            if (j % 2 == 0) {
                U = *puv++;
                V = *puv++;
                U -= 128;
                V -= 128;
            }
            int R = (int)( Y + 1.403f * V);
            int G = (int)( Y - 0.344f * U - 0.714f * V);
            int B = (int)( Y + 1.770f * U);
            bgr[dpos + 0] = CLIP_COLOR(B);
            bgr[dpos + 1] = CLIP_COLOR(G);
            bgr[dpos + 2] = CLIP_COLOR(R);
            bgr[dpos + 3] = 255;
        }
    }
}

void convertYUYVToBGRX(unsigned char* yuv, unsigned char* bgr, int w, int h, int dw, int dh) {
    unsigned char* py = yuv + 0;
    unsigned char* pu = yuv + 1;
    unsigned char* pv = yuv + 3;
    int U = 0;
    int V = 0;
    float xscale = dw;
    xscale /= w;
    float yscale = dh;
    yscale /= h;

    for (int i = 0; i < h; i++) {
        int dy = i*yscale;
        for (int j = 0; j < w; j++) {
            int dx = j*xscale;
            int dpos = (dy*dw + dx) << 2;
            int Y = *py;
            py += 2;
            if (j % 2 == 0) {
                U = *pu;
                V = *pv;
                U -= 128;
                V -= 128;
                pu +=4;
                pv +=4;
            }
            int R = (int)( Y + 1.403f * V);
            int G = (int)( Y - 0.344f * U - 0.714f * V);
            int B = (int)( Y + 1.770f * U);
            bgr[dpos + 0] = CLIP_COLOR(B);
            bgr[dpos + 1] = CLIP_COLOR(G);
            bgr[dpos + 2] = CLIP_COLOR(R);
            bgr[dpos + 3] = 255;
        }
    }
}

void convertNV12ToBGRX(unsigned char* yuv, unsigned char* bgr, int w, int h, int dw, int dh) {
    unsigned char* puv = yuv + w*h;
    int U = 0;
    int V = 0;
    float xscale = dw;
    xscale /= w;
    float yscale = dh;
    yscale /= h;

    for (int i = 0; i < h; i++) {
        int dy = i*yscale;
        for (int j = 0; j < w; j++) {
            int dx = j*xscale;
            int dpos = (dy*dw + dx) << 2;
            int Y = yuv[i*w+j];
            U = puv[i/2*w + j/2*2];
            V = puv[i/2*w + j/2*2+1];
            U -= 128;
            V -= 128;
            int R = (int)( Y + 1.403f * V);
            int G = (int)( Y - 0.344f * U - 0.714f * V);
            int B = (int)( Y + 1.770f * U);
            bgr[dpos + 0] = CLIP_COLOR(B);
            bgr[dpos + 1] = CLIP_COLOR(G);
            bgr[dpos + 2] = CLIP_COLOR(R);
            bgr[dpos + 3] = 255;
        }
    }
}

#ifdef APPMODE_V4L2
# define _init8 \
do { \
    dx = j * xscale; \
    dpos = (dy * dw + dx) << 2; \
    pix = dat[i * w + j]; \
} while (0)

# define _init \
do { \
    dx = j * xscale; \
    dpos = (dy * dw + dx) << 2; \
    pix = dat[(i * w + j) * 2]; \
} while (0)

# define _to_bgr(x, b, g, r) \
do { \
    *((x)) = (b); \
    *((x) + 1) = (g); \
    *((x) + 2) = (r); \
    *((x) + 3) = 255; \
} while (0)

# define _grbg8 \
do { \
    for (int j = 0; j < w; j++) { \
        _init8; \
        _to_bgr(&bgr[dpos], 0, pix, 0); \
        j++; \
        _init8; \
        _to_bgr(&bgr[dpos], 0, 0, pix); \
    } \
    i++; \
    dy = i * yscale; \
    for (int j = 0; j < w; j++) { \
        _init8; \
        _to_bgr(&bgr[dpos], pix, 0, 0); \
        j++; \
        _init8; \
        _to_bgr(&bgr[dpos], 0, pix, 0); \
    } \
} while (0)

# define _rggb8 \
do { \
    for (int j = 0; j < w; j++) { \
        _init8; \
        _to_bgr(&bgr[dpos], 0, 0, pix); \
        j++; \
        _init8; \
        _to_bgr(&bgr[dpos], 0, pix, 0); \
    } \
    i++; \
    dy = i * yscale; \
    for (int j = 0; j < w; j++) { \
        _init8; \
        _to_bgr(&bgr[dpos], 0, pix, 0); \
        j++; \
        _init8; \
        _to_bgr(&bgr[dpos], pix, 0, 0); \
    } \
} while (0)

# define _bggr8 \
do { \
    for (int j = 0; j < w; j++) { \
        _init8; \
        _to_bgr(&bgr[dpos], pix, 0, 0); \
        j++; \
        _init8; \
        _to_bgr(&bgr[dpos], 0, pix, 0); \
    } \
    i++; \
    dy = i * yscale; \
    for (int j = 0; j < w; j++) { \
        _init8; \
        _to_bgr(&bgr[dpos], 0, pix, 0); \
        j++; \
        _init8; \
        _to_bgr(&bgr[dpos], 0, 0, pix); \
    } \
} while (0)

# define _gbrg8 \
do { \
    for (int j = 0; j < w; j++) { \
        _init8; \
        _to_bgr(&bgr[dpos], 0, pix, 0); \
        j++; \
        _init8; \
        _to_bgr(&bgr[dpos], pix, 0, 0); \
    } \
    i++; \
    dy = i * yscale; \
    for (int j = 0; j < w; j++) { \
        _init8; \
        _to_bgr(&bgr[dpos], 0, 0, pix); \
        j++; \
        _init8; \
        _to_bgr(&bgr[dpos], 0, pix, 0); \
    } \
} while (0)

# define _grbg \
do { \
    for (int j = 0; j < w; j++) { \
        _init; \
        _to_bgr(&bgr[dpos], 0, pix, 0); \
        j++; \
        _init; \
        _to_bgr(&bgr[dpos], 0, 0, pix); \
    } \
    i++; \
    dy = i * yscale; \
    for (int j = 0; j < w; j++) { \
        _init; \
        _to_bgr(&bgr[dpos], pix, 0, 0); \
        j++; \
        _init; \
        _to_bgr(&bgr[dpos], 0, pix, 0); \
    } \
} while (0)

# define _rggb \
do { \
    for (int j = 0; j < w; j++) { \
        _init; \
        _to_bgr(&bgr[dpos], 0, 0, pix); \
        j++; \
        _init; \
        _to_bgr(&bgr[dpos], 0, pix, 0); \
    } \
    i++; \
    dy = i * yscale; \
    for (int j = 0; j < w; j++) { \
        _init; \
        _to_bgr(&bgr[dpos], 0, pix, 0); \
        j++; \
        _init; \
        _to_bgr(&bgr[dpos], pix, 0, 0); \
    } \
} while (0)

# define _bggr \
do { \
    for (int j = 0; j < w; j++) { \
        _init; \
        _to_bgr(&bgr[dpos], pix, 0, 0); \
        j++; \
        _init; \
        _to_bgr(&bgr[dpos], 0, pix, 0); \
    } \
    i++; \
    dy = i * yscale; \
    for (int j = 0; j < w; j++) { \
        _init; \
        _to_bgr(&bgr[dpos], 0, pix, 0); \
        j++; \
        _init; \
        _to_bgr(&bgr[dpos], 0, 0, pix); \
    } \
} while (0)

# define _gbrg \
do { \
    for (int j = 0; j < w; j++) { \
        _init; \
        _to_bgr(&bgr[dpos], 0, pix, 0); \
        j++; \
        _init; \
        _to_bgr(&bgr[dpos], pix, 0, 0); \
    } \
    i++; \
    dy = i * yscale; \
    for (int j = 0; j < w; j++) { \
        _init; \
        _to_bgr(&bgr[dpos], 0, 0, pix); \
        j++; \
        _init; \
        _to_bgr(&bgr[dpos], 0, pix, 0); \
    } \
} while (0)

void convertRawToBGRX(unsigned char* dat, unsigned char* bgr, int w, int h, int dw, int dh, int fmt) {
    int dx, dy, dpos, pix;
    float xscale = dw;
    float yscale = dh;
    xscale /= w;
    yscale /= h;

# ifdef RAW_TO_GRAY
    switch (fmt) {
    case V4L2_PIX_FMT_SGRBG8:
    case V4L2_PIX_FMT_SRGGB8:
    case V4L2_PIX_FMT_SBGGR8:
    case V4L2_PIX_FMT_SGBRG8:
        for (int i = 0; i < h; i++) {
            dy = i * yscale;
            for (int j = 0; j < w; j++) {
                _init8;
                _to_bgr(&bgr[dpos], pix, pix, pix);
            }
        }
        break;
    case V4L2_PIX_FMT_SGRBG10:
    case V4L2_PIX_FMT_SRGGB10:
    case V4L2_PIX_FMT_SBGGR10:
    case V4L2_PIX_FMT_SGBRG10:
    case V4L2_PIX_FMT_SGRBG12:
    case V4L2_PIX_FMT_SRGGB12:
    case V4L2_PIX_FMT_SBGGR12:
    case V4L2_PIX_FMT_SGBRG12:
        for (int i = 0; i < h; i++) {
            dy = i * yscale;
            for (int j = 0; j < w; j++) {
                _init;
                _to_bgr(&bgr[dpos], pix, pix, pix);
            }
        }
        break;
    }
# else
    switch (fmt) {
    case V4L2_PIX_FMT_SGRBG8:
        for (int i = 0; i < h; i++) {
            dy = i * yscale;
            _grbg8;
        }
        break;
    case V4L2_PIX_FMT_SRGGB8:
        for (int i = 0; i < h; i++) {
            dy = i * yscale;
            _rggb8;
        }
        break;
    case V4L2_PIX_FMT_SBGGR8:
        for (int i = 0; i < h; i++) {
            dy = i * yscale;
            _bggr8;
        }
        break;
    case V4L2_PIX_FMT_SGBRG8:
        for (int i = 0; i < h; i++) {
            dy = i * yscale;
            _gbrg8;
        }
        break;
    case V4L2_PIX_FMT_SGRBG10:
    case V4L2_PIX_FMT_SGRBG12:
        for (int i = 0; i < h; i++) {
            dy = i * yscale;
            _grbg;
        }
        break;
    case V4L2_PIX_FMT_SRGGB10:
    case V4L2_PIX_FMT_SRGGB12:
        for (int i = 0; i < h; i++) {
            dy = i * yscale;
            _rggb;
        }
        break;
    case V4L2_PIX_FMT_SBGGR10:
    case V4L2_PIX_FMT_SBGGR12:
        for (int i = 0; i < h; i++) {
            dy = i * yscale;
            _bggr;
        }
        break;
    case V4L2_PIX_FMT_SGBRG10:
    case V4L2_PIX_FMT_SGBRG12:
        for (int i = 0; i < h; i++) {
            dy = i * yscale;
            _gbrg;
        }
        break;
    }
# endif
}
#endif
