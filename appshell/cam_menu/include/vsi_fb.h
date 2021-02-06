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

#ifndef CAM_MENU_INCLUDE_VSI_FB_H_
#define CAM_MENU_INCLUDE_VSI_FB_H_

#define FBIOGET_VSCREENINFO 0x4600

static int Frame_fd = -1;
unsigned char *FrameBuffer;
static int Framebpp = 0;
static int screensize = 0;

struct fb_bitfield {
    unsigned int offset;                /* beginning of bitfield */
    unsigned int length;                /* length of bitfield */
    unsigned int msb_right;             /* !=0: Most significant bit is right */
};

struct fb_var_screeninfo {
    unsigned int xres;                  /* visible resolution */
    unsigned int yres;
    unsigned int xres_virtual;          /* virtual resolution */
    unsigned int yres_virtual;
    unsigned int xoffset;               /* offset from virtual to visible */
    unsigned int yoffset;               /* resolution */

    unsigned int bits_per_pixel;
    unsigned int grayscale;             /* !=0 Graylevels instead of colors */

    struct fb_bitfield red;         /* bitfield in fb mem if true color, */
    struct fb_bitfield green;       /* else only length is significant */
    struct fb_bitfield blue;
    struct fb_bitfield transp;      /* transparency */

    unsigned int nonstd;                /* !=0 Non standard pixel format */

    unsigned int activate;              /* see FB_ACTIVATE_x */

    unsigned int height;                /* height of picture in mm */
    unsigned int width;                 /* width of picture in mm */

    unsigned int accel_flags;           /* acceleration flags (hints) */

    /* Timing: All values in pixclocks, except pixclock (of course) */
    unsigned int pixclock;              /* pixel clock in ps (pico seconds) */
    unsigned int left_margin;           /* time from sync to picture */
    unsigned int right_margin;          /* time from picture to sync */
    unsigned int upper_margin;          /* time from sync to picture */
    unsigned int lower_margin;
    unsigned int hsync_len;             /* length of horizontal sync */
    unsigned int vsync_len;             /* length of vertical sync */
    unsigned int sync;                  /* see FB_SYNC_x */
    unsigned int vmode;                 /* see FB_VMODE_x */
    unsigned int reserved[6];           /* Reserved for future compatibility */
} vinfo;

// rgb
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char rgbReserved;
} rgb32;

// frame cache
typedef struct {
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char rgbReserved;
} rgb32_frame;

extern int init_FrameBuffer(void);
extern int write_data_to_fb(void *fbp,
                            int fbfd,
                            void *img_buf,
                            unsigned int img_width,
                            unsigned int img_height,
                            unsigned int img_bits);

#endif  // CAM_MENU_INCLUDE_VSI_FB_H_

