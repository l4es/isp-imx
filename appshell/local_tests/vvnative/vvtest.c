/******************************************************************************\
|* Copyright 2010, Dream Chip Technologies GmbH. used with permission by      *|
|* VeriSilicon.                                                               *|
|* Copyright (c) <2020> by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/* VeriSilicon 2020 */

#include<sys/types.h>
#include<unistd.h>
#include<sys/stat.h>
#include<stdio.h>
#include<fcntl.h>
#include<string.h>

#include <sys/ioctl.h>

#include<sys/mman.h>

#define VVISP_OPEN_LIMITATION 1

#define IOC_MAGIC  'v'
#define IOCINIT    _IO(IOC_MAGIC, 0)


#define D_NAME0 "/dev/vivisp0"
#define D_NAME1 "/dev/vivisp1"
//#define D_NAME "freg"

#define PAGE_SIZE (4*1024)
#define BUF_SIZE (16*PAGE_SIZE)
#define OFFSET (0)



int main()
{
    int fd,i, ret;
    char msg[101];
    char *addr = NULL;
    char *addr1 = NULL;

    fd= open(D_NAME0, O_RDWR);
    if(fd!=-1)
    {
       printf("device %s opened\n", D_NAME0);

       ret = ioctl(fd, IOCINIT);
       if (ret) {
           perror("ioctl init:");
           return -1;
       }


       addr = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, OFFSET);
       printf("##addr is 0x%lx \n", (unsigned long)addr);
       //sprintf(addr, "I am writer\n");
       printf("addr data is: %s\n", addr);

       addr1 = mmap(NULL, BUF_SIZE*4, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, OFFSET);
       printf("##addr1 is 0x%lx \n", (unsigned long)addr1);
       printf("addr1 data is: %s\n", addr1);


#if 0
        while(1)
        {
            for(i=0;i<101;i++)
                msg[i]='\0';
            read(fd,msg,100);
            printf("%s\n",msg);
            if(strcmp(msg,"quit")==0)
            {
                close(fd);
                break;
            }
        }
#endif
    }
    else
    {
        printf("device %s open failure,%d\n", D_NAME0, fd);
        //close(fd);
    }

//open it again
    fd= open(D_NAME0, O_RDWR);
    if(fd!=-1)
    {
       printf("device %s opened\n", D_NAME0);
#if 0
        while(1)
        {
            for(i=0;i<101;i++)
                msg[i]='\0';
            read(fd,msg,100);
            printf("%s\n",msg);
            if(strcmp(msg,"quit")==0)
            {
                close(fd);
                break;
            }
        }
#endif
    }
    else
    {
        printf("device %s open failure,%d\n", D_NAME0, fd);
        //close(fd);
    }



    fd= open(D_NAME1, O_RDWR);
    if(fd!=-1)
    {
       printf("device %s opened\n", D_NAME1);
#if 0
        while(1)
        {
            for(i=0;i<101;i++)
                msg[i]='\0';
            read(fd,msg,100);
            printf("%s\n",msg);
            if(strcmp(msg,"quit")==0)
            {
                close(fd);
                break;
            }
        }
#endif
    ret = ioctl(fd, IOCINIT);
    if (ret) {
        perror("ioctl init:");
        return -1;
    }


    }
    else
    {
        printf("device %s open failure,%d\n", D_NAME1, fd);
        close(fd);
    }


    fd= open(D_NAME1, O_RDWR);
    if(fd!=-1)
    {
       printf("device %s opened\n", D_NAME1);
#if 0
        while(1)
        {
            for(i=0;i<101;i++)
                msg[i]='\0';
            read(fd,msg,100);
            printf("%s\n",msg);
            if(strcmp(msg,"quit")==0)
            {
                close(fd);
                break;
            }
        }
#endif
    }
    else
    {
        printf("device %s open failure,%d\n", D_NAME1, fd);
        close(fd);
    }


    printf("open done, exit\n");
    return 0;
}
