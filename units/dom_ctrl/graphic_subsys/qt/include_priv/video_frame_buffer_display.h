#ifndef __VIDEO_FRAME_BUFFER_H__
#define __VIDEO_FRAME_BUFFER_H__
#include <ebase/trace.h>

struct ImageTypeBGR32{
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char rgbReserved;
};
struct ImageTypeRGB32{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char rgbReserved;
};

typedef struct VideoFbConfig_s
{
    int32_t             fb_fd;
    uint32_t            xres;
    uint32_t            yres;
    uint32_t            bits_per_pixel;
    uint64_t            screensize;
    void *              FrameBuffer;
}VideoFbConfig_t;

int32_t VideoFbInit(VideoFbConfig_t *pFbConfig);
int32_t VideoFbrSetRgbDisplay(VideoFbConfig_t *pFbConfig, void *data, uint32_t Width, uint32_t Height);
void    VideoFbRelease(VideoFbConfig_t *pFbConfig);

#endif
