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

// NOTICE: This file is designed can be extend into driver in future. C Style is better.
#ifndef DEVELOPER_DEWARP_INCLUDE_DEWARPDEV_H_
#define DEVELOPER_DEWARP_INCLUDE_DEWARPDEV_H_
#include <sys/ioctl.h>
#include <stdint.h>

#ifndef MIN
#define MIN(a, b) ((a) < (b)) ? (a) : (b)
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b)) ? (a) : (b)
#endif

#undef ALIGN_UP
#undef ALIGN_DOWN
#undef BLOCK_SIZE
#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align)-1))
#define ALIGN_DOWN(x, align) ((x) & ~((align)-1))

#define ALIGN_SIZE_1K               ( 0x400 )
#define ALIGN_UP_1K(addr)   ( ALIGN_UP(addr, ALIGN_SIZE_1K) )

#define BLOCK_SIZE 16
#define VS_PI   3.1415926535897932384626433832795
#define VS_2PI 6.283185307179586476925286766559

#define DEWARP_BUFFER_ALIGNMENT 16
#define PHYSICAL_ADDR_SHIFT  4  // for 34bit address
#define MAX_MAP_SIZE 262144

// correction types, also used as capabilities
enum {
    DEWARP_MODEL_LENS_DISTORTION_CORRECTION = 1 << 0,
    DEWARP_MODEL_FISHEYE_EXPAND             = 1 << 1,
    DEWARP_MODEL_SPLIT_SCREEN               = 1 << 2,
    DEWARP_MODEL_FISHEYE_DEWARP             = 1 << 3,
};

struct dewarp_buffer_size {
    uint32_t width;
    uint32_t height;
};

struct dewarp_single_pixel {
    unsigned char Y, U, V;
};

struct dewarp_parameters {
    struct dewarp_buffer_size image_size;
    struct dewarp_buffer_size image_size_dst;
    struct dewarp_buffer_size roi_start;
    struct dewarp_single_pixel boundary_pixel;
    int scale_factor;  // [1.0, 4.0] << 8
    uint32_t split_horizon_line;
    uint32_t split_vertical_line_up;
    uint32_t split_vertical_line_down;
    uint32_t pix_fmt_in;
    uint32_t pix_fmt_out;
    uint32_t dewarp_type;
    int buffer_queue_type;  // src buffer queue type 0: user ptr,  1: addr
    bool hflip;
    bool vflip;
    bool bypass;
};

struct dewarp_capability {
    char name[32];
    char version_number[32];
    char type[4];
    uint32_t capabilities;
};

struct dewarp_distortion_map {
    uint32_t index;  // two map, select as 0 or 1.
    uint32_t userMapSize;   //  0: set camera matrix,  calculate map at driver
                            // !0: set distortion map,  calculated by user.
    double camera_matrix[9];
    double perspective_matrix[9];
    double distortion_coeff[8];
    uint32_t* pUserMap;
};

#endif  // DEVELOPER_DEWARP_INCLUDE_DEWARPDEV_H_
