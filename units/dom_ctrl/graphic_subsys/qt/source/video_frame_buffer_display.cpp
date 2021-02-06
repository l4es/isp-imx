#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "video_frame_buffer_display.h"

CREATE_TRACER(FRAME_BUFFER_ERROR, "FRAME_BUFFER: ", ERROR, 1);

int32_t VideoFbInit(VideoFbConfig_t *pFbConfig)
{
    struct fb_var_screeninfo vinfo;

    if (!pFbConfig)
    {
        TRACE(FRAME_BUFFER_ERROR, "%s error: Input Null pointer!\n", __func__);
        return (-1);
    }

    memset(pFbConfig,0,sizeof(VideoFbConfig_t));

    pFbConfig->fb_fd = open(FB_DEV , O_RDWR);
    if (pFbConfig->fb_fd <= 0)
    {
        TRACE(FRAME_BUFFER_ERROR, "%s open %s error!\n", __func__,FB_DEV);
        return -1;
    }

    if (ioctl(pFbConfig->fb_fd , 0x4600, &vinfo))
    {
        TRACE(FRAME_BUFFER_ERROR,"% Error reading variable information.\n", __func__);
        close(pFbConfig->fb_fd);
        pFbConfig->fb_fd = -1;
        return -1;
    }

    pFbConfig->xres           = vinfo.xres;
    pFbConfig->yres           = vinfo.yres;
    pFbConfig->bits_per_pixel = vinfo.bits_per_pixel;
    pFbConfig->screensize     = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    pFbConfig->FrameBuffer    = mmap(0, pFbConfig->screensize, PROT_READ | PROT_WRITE , MAP_SHARED , pFbConfig->fb_fd , 0);

    if(pFbConfig->FrameBuffer == (void *)-1)
    {
        TRACE(FRAME_BUFFER_ERROR,"% Error mmap FbFrameBuffer.\n", __func__);
        close(pFbConfig->fb_fd);
        pFbConfig->fb_fd = -1;
        return -1;
    }
    return 0;
}

int32_t VideoFbrSetRgbDisplay(VideoFbConfig_t *pFbConfig, void *data, uint32_t Width, uint32_t Height)
{
    uint32_t DisPlay_Width,DisPlay_Height;

    if (!pFbConfig)
    {
        TRACE(FRAME_BUFFER_ERROR, "%s error: Input Null pointer!\n", __func__);
        return (-1);
    }

    if ((pFbConfig->FrameBuffer == NULL) || (pFbConfig->FrameBuffer == (void *)-1))
    {
        TRACE(FRAME_BUFFER_ERROR, "%s error: FbFrameBuffer Null pointer!\n", __func__);
        return (-1);
    }

    DisPlay_Width = Width;
    if (DisPlay_Width > pFbConfig->xres)
    {
        DisPlay_Width = pFbConfig->xres;
    }
    DisPlay_Height = Height;
    if (DisPlay_Height > pFbConfig->yres)
    {
        DisPlay_Height = pFbConfig->yres;
    }

    struct ImageTypeBGR32 *pFrameBufffer = (struct ImageTypeBGR32 *)pFbConfig ->FrameBuffer;
    struct ImageTypeRGB32 *pImgBufffer   = (struct ImageTypeRGB32 *)data;
    uint32_t row, column;
    uint32_t index;
    for(row = 0; row < DisPlay_Height; row++)
    {
        index = row * DisPlay_Width;
        for(column = 0; column < DisPlay_Width; column++)
        {
            pFrameBufffer[DisPlay_Width * row + column].r           =  pImgBufffer[index].r;
            pFrameBufffer[DisPlay_Width * row + column].g           =  pImgBufffer[index].g;
            pFrameBufffer[DisPlay_Width * row + column].b           =  pImgBufffer[index].b;
            pFrameBufffer[DisPlay_Width * row + column].rgbReserved = 0;
            index++;
        }
    }
    return 0;
}

void VideoFbRelease(VideoFbConfig_t *pFbConfig)
{
    if (pFbConfig == NULL)
    {
        return;
    }
    if (pFbConfig->FrameBuffer != NULL)
    {
        munmap(pFbConfig->FrameBuffer, pFbConfig->screensize);
    }
    if (pFbConfig->fb_fd > 0)
    {
        close(pFbConfig->fb_fd);
        pFbConfig->fb_fd = -1;
    }
    return;
}