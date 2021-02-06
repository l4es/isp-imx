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

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include <map>
#include <string>
#include <thread>
#include <fstream>
#include <iostream>
#include "dwe_regs.h"
#include "dewarpdev.h"
#include "DewarpDriver.h"
#include "highwayhash.h"
#include "DewarpConfigParser.h"
#include "IMemoryAllocator.h"
#include "BufferManager.h"

#include "log.h"

#define LOGTAG "DEWARP_TEST"
using namespace std;

DewarpDriver gDriver;
int dewarpFd = -1;
string inputImageFile, outputImageFile, verifyHashFile;
vector<string>  gFailedCaseList;

uint64_t goldenHashKey = 0;
static int nTotalCaseCount = 0;
static int nPassedCaseCount = 0;
struct dewarp_parameters params;

bool bUpdateHash = false;
bool bFullCase = false;
bool bClearYUV = false;

ofstream os;
void initReport() {
    os.open("test_report.csv");
    os << "Case," << "In Res," << "Out Res," << "FISHEYE_EXPAND," << "LENS_CORRECTION,"
        << "SPLIT_SCREEN," << "SCALEUP," << "CROP," << "Result" <<endl;
}

void dumpReport(const char* pCaseName, const dewarp_parameters& dp, bool bPass) {
    os << pCaseName << ",";
    os << dp.image_size.width << "x" << dp.image_size.height << ",";
    os << dp.image_size_dst.width << "x" << dp.image_size_dst.height << ",";
    if (dp.dewarp_type == DEWARP_MODEL_FISHEYE_EXPAND) {
        os <<"ON,,,";
    } else if (dp.dewarp_type == DEWARP_MODEL_LENS_DISTORTION_CORRECTION) {
        os <<",ON,,";
    } else if (dp.dewarp_type == DEWARP_MODEL_SPLIT_SCREEN) {
         os <<",,ON,";
    }
    if (dp.scale_factor > 4096) {
         os << "ON,";
    } else {
       os << ",";
    }
    if (dp.roi_start.width > 0) {
         os << "ON,";
    } else {
       os << ",";
    }
    os << (bPass ? "PASS" : "FAILED");
    os << endl;
}

struct dewarp_distortion_map dmap[2];
std::vector<uint32_t> userMap;
MediaAddrBuffer srcBuffer, dstBuffer;

int readFileThead(const char* pFileName) {
    int rows = params.image_size.height * 2;
    if (MEDIA_PIX_FMT_YUV420SP == params.pix_fmt_in)
        rows =  params.image_size.height * 3 / 2;
    int yuvSizeIn = params.image_size.width * rows;
    FILE * pfin = fopen(inputImageFile.c_str(), "rb");

    if (fread(GMAP(srcBuffer.baseAddress, yuvSizeIn), yuvSizeIn, 1, pfin) != 0) {
         ALOGI("queue one src buffer w:%d h:%d s:%d size:%d  y_base: %p",
            srcBuffer.mWidth, srcBuffer.mHeight, srcBuffer.mStride, srcBuffer.mSize,
            srcBuffer.getBuffer());

        GPUSH(&gDriver, 0, 0, srcBuffer.baseAddress);
    }

    fclose(pfin);
    return 0;
}


bool VerifyHash(const char* pFileName) {
    FILE* pFile = fopen(outputImageFile.c_str(), "rb");
    if (!pFile)
        return false;

    fseek(pFile, 0L, SEEK_END);
    uint64_t size = ftell(pFile);
    fseek(pFile, 0L, SEEK_SET);

    unsigned char* data = new unsigned char[size];
    fread(data, 1, size, pFile);
    static const uint64_t kTestKey1[4] = {
      0x0706050403020100ull, 0x0F0E0D0C0B0A0908ull,
      0x1716151413121110ull, 0x1F1E1D1C1B1A1918ull
    };
    uint64_t ret = HighwayHash64(data, size, kTestKey1);
    delete data;
    fclose(pFile);
    char sz[256];
    snprintf(sz, sizeof(sz), "%lu", ret);
    if (bUpdateHash) {
        updateJsonHash(pFileName, sz);
    }
    if (goldenHashKey == ret || bUpdateHash) {
        ALOGI("%s test PASS", pFileName);
        nPassedCaseCount++;
    } else {
        ALOGE("%s test FAILED: expect %lu,  key %lu", pFileName, goldenHashKey, ret);
        gFailedCaseList.push_back(pFileName);
        return false;
    }
    return true;
}

std::string jsonFileName;
int singleCaseTest(const char* pFileName) {
    memset(&params, 0, sizeof(params));
    memset(&dmap, 0, sizeof(dmap));
    parseJsonFile(pFileName, inputImageFile, outputImageFile, userMap, dmap, params,
                    goldenHashKey, verifyHashFile);

    if (!gDriver.setParams(&params)) {
        ALOGE("failed to set params");
        return -1;
    }

    if (!gDriver.setDistortionMap(dmap)) {
        ALOGE("failed to set distortion map");
        return -1;
    }
    if (!gDriver.start()) {
        return -1;
    }
    srcBuffer.release();
    dstBuffer.release();
    srcBuffer.create(params.image_size.width, params.image_size.height, params.pix_fmt_in);
    int dw, dh;
    gDriver.getDstSize(dw, dh);
    dstBuffer.create(dw, dh, params.pix_fmt_out);
    GPUSH(&gDriver, 1, 0, dstBuffer.baseAddress);
    jsonFileName = pFileName;
    readFileThead(pFileName);
    return 0;
}

ESignal sig;
void onFrameAvailable(uint64_t addrSrc, uint64_t addr, int port) {
    uint64_t d = GPOP(&gDriver, 2, port);
    ALOGI("deque one buffer:  w:%d h:%d s:%d size:%d  y_base: %p %p %p ",
         dstBuffer.mWidth, dstBuffer.mHeight, dstBuffer.mStride, dstBuffer.mSize,
         (unsigned char*)dstBuffer.baseAddress, (unsigned char*) addr, (unsigned char*) d);
    dstBuffer.save(outputImageFile.c_str());
    dumpReport(jsonFileName.c_str(), params, VerifyHash(jsonFileName.c_str()));
    sig.post();
}

void dumpResult() {
    ALOGI("==============TEST FINISH===========================");
    ALOGI("               pass/total %d / %d                   ",
            nPassedCaseCount, nTotalCaseCount);
    ALOGD("FAILED LIST:");
    for (auto& item : gFailedCaseList) {
        ALOGE("%s", item.c_str());
    }
    ALOGI("====================================================");
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];

        if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
            printf("Usage: %s [PARAMS...]\n", argv[0]);
            printf("\n"
                "-u, update hash key for simulator\n"
                "-c, clear yuv file on disk \n"
                "-f, run case/list_full  instead of case/list.txt\n"
                "--------------------------------------\n\n");
            exit(0);
        }
        if (!strcmp(arg, "-u")) {
            bUpdateHash = true;
            continue;
        }
        if (!strcmp(arg, "-f")) {
            bFullCase = true;
            continue;
        }
        if (!strcmp(arg, "-c")) {
            bClearYUV = true;
            continue;
        }
    }
#ifdef HAL_CMODEL
    IMemoryAllocator::create(ALLOCATOR_TYPE_USER_PTR);
#else
#ifdef USE_V4L2 
    IMemoryAllocator::create(ALLOCATOR_TYPE_V4L2);
#else
    IMemoryAllocator::create(ALLOCATOR_TYPE_DIRECT_MAP);
#endif
#endif
    dewarpFd = gDriver.open();
    if (dewarpFd != 0) {
        ALOGE("failed to open dewarp hardware");
        return -1;
    }
    FILE* pFileList = NULL;
    if (bFullCase) {
        pFileList = fopen("case/list_full.txt", "r");
    } else {
        pFileList = fopen("case/list.txt", "r");
    }
    if (!pFileList) {
        ALOGE("can't open list file");
        return -1;
    }
    initReport();
    gDriver.registerBufferCallback(onFrameAvailable);
    char szCase[1024];
    while (fgets(szCase, 1024, pFileList)) {
        int len = strlen(szCase);
        if (len > 1) {
            if (szCase[len-1]  == '\n')
                szCase[len-1] = '\0';
            singleCaseTest(szCase);
            sig.wait();
            gDriver.stop();
            nTotalCaseCount++;
            if (bClearYUV)
                system("rm *.yuv -rf");
        }
    }
    fclose(pFileList);
    gDriver.close(dewarpFd);
    dumpResult();
    os.close();
    return 0;
}
