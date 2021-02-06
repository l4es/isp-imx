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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <assert.h>
#include <memory.h>
#include <linux/videodev2.h>
#include <semaphore.h>

#include "vsi_v4l2.h"
#include "vsi_fb.h"
#include "log.h"

int init_FrameBuffer(void)  
{  
    struct fb_var_screeninfo vinfo;

    Frame_fd = open("/dev/fb0" , O_RDWR);  
    if(-1 == Frame_fd) {  
        perror("open frame buffer dev/fb0 fail");  
        return -1 ;   
    }  
    
    // Get variable screen information
    if (ioctl(Frame_fd, FBIOGET_VSCREENINFO, &vinfo)) {            
        printf("Error reading variable information.\n");
        exit(0);
    }

    if ((Width > vinfo.xres) || (Height > vinfo.yres)) {
    	printf("Copy size from (%dx%d) to framebuffer resolution (%dx%d)!!!\n",
    	    Width, Height, vinfo.xres, vinfo.yres);
    	Width = vinfo.xres;
    	Height = vinfo.yres;
    }

    screensize = vinfo.xres_virtual * vinfo.yres_virtual * vinfo.bits_per_pixel / 8;
    Framebpp = vinfo.bits_per_pixel;
    printf("%dx%d, %dbpp  screensize is %d\n", vinfo.xres_virtual, vinfo.yres_virtual, vinfo.bits_per_pixel,screensize);
    
    FrameBuffer = mmap(0, screensize, PROT_READ | PROT_WRITE , MAP_SHARED , Frame_fd ,0 );  
    if(FrameBuffer == (void *)-1) {  
        perror("memory map fail");  
        return -2 ;  
    }

    return 0 ;   
}

int exit_Framebuffer(void)  
{  
    munmap(FrameBuffer , screensize);  
    close(Frame_fd);  
    return 0 ;   
}

int write_data_to_fb(void *fbp, int fbfd, void *img_buf, unsigned int img_width, unsigned int img_height, unsigned int img_bits)
{   
    int row, column;
    int num = 0;
    unsigned int Framex = vinfo.xres_virtual;
    rgb32_frame *rgb32_fbp = (rgb32_frame *)fbp;
    rgb32 *rgb32_img_buf = (rgb32 *)img_buf;    
    
    if(screensize < img_width * img_height * img_bits / 8)
    {
        printf("the imgsize is too large\n");
        return -1;
    }
        
    switch (img_bits)
    {
        case 32:
            for(row = 0; row < img_height; row++)
                {
                    for(column = 0; column < img_width; column++)
                    {
                        rgb32_fbp[row * Framex + column].r = rgb32_img_buf[num].r;
                        rgb32_fbp[row * Framex + column].g = rgb32_img_buf[num].g;
                        rgb32_fbp[row * Framex + column].b = rgb32_img_buf[num].b;

                        num++;
                    }        
                }    
            break;
        default:
            break;
    }
    
    return 0;
}
