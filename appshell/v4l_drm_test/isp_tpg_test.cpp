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
#include "isp_ioctl.h"
#include "isp_user_types.h"
#include "isp_types.h"

#include "log.h"

#define LOGTAG "isp_drv_test"

#define TEST_RES_WIDTH 1920
#define TEST_RES_HEIGHT 1080

int ispDevice;

int configInput() {
    struct isp_context isp_ctx;
    memset(&isp_ctx, 0, sizeof(isp_ctx));
    isp_ctx.mode = MRV_ISP_ISP_MODE_RGB;
    isp_ctx.sample_edge = 0;
    isp_ctx.hSyncLowPolarity = false;
    isp_ctx.vSyncLowPolarity = false;
    isp_ctx.bayer_pattern = MRV_ISP_BAYER_PAT_RG;
    isp_ctx.sub_sampling = MRV_ISP_CONV_422_CO;
    isp_ctx.input_selection = MRV_ISP_INPUT_SELECTION_8MSB;
    isp_ctx.latency_fifo = MRV_ISP_LATENCY_FIFO_SELECTION_DMA_READ;
    isp_ctx.acqWindow.x = 0;
    isp_ctx.acqWindow.y = 0;
    isp_ctx.acqWindow.width = TEST_RES_WIDTH;
    isp_ctx.acqWindow.height = TEST_RES_HEIGHT;
    isp_ctx.ofWindow.x = 0;
    isp_ctx.ofWindow.y = 0;
    isp_ctx.ofWindow.width = TEST_RES_WIDTH;
    isp_ctx.ofWindow.height = TEST_RES_HEIGHT;
    isp_ctx.isWindow.x = 0;
    isp_ctx.isWindow.y = 0;
    isp_ctx.isWindow.width = TEST_RES_WIDTH;
    isp_ctx.isWindow.height = TEST_RES_HEIGHT;
    isp_ctx.bypass_mode = false;
    isp_ctx.demosaic_threshold = 0;
    return ioctl(ispDevice, ISPIOC_S_INPUT, &isp_ctx);
}

int configDataPath() {
    struct isp_mux_context mux;
    mux.chan_mode = 1; // MP
    mux.mp_mux = MRV_VI_MP_MUX_MP;
    mux.ie_mux = MRV_VI_DMA_IEMUX_CAM;
    mux.if_select = MRV_IF_SELECT_PAR;
    return ioctl(ispDevice, ISPIOC_S_MUX, &mux);
}

int configAwb() {
    struct isp_awb_context awb;
    awb.enable = true;
    awb.mode = MRV_ISP_AWB_MEAS_MODE_RGB;
    awb.gain_r = awb.gain_gr = awb.gain_gb = awb.gain_b = 0x100;
    awb.window.x = awb.window.y = 0;
    awb.window.width  = TEST_RES_WIDTH;
    awb.window.height = TEST_RES_HEIGHT;
    awb.refcb_max_b   = 0x80;
    awb.refcr_max_r   = 0x80;
    awb.max_y         = 0xE9;
    awb.max_c_sum     = 0x10;
    awb.min_y_max_g   = 0xC0;
    awb.min_c         = 0x10;
    return ioctl(ispDevice, ISPIOC_S_AWB, &awb);
}

int configTpg() {
    struct isp_tpg_context tpg;
    tpg.image_type = 0;//CAMERIC_ISP_TPG_IMAGE_TYPE_COLORBAR;
    tpg.bayer_pattern = 0;//CAMERIC_ISP_TPG_BAYER_PATTERN_BGGR;
    tpg.color_depth = 0;//CAMERIC_ISP_TPG_8BIT;
    tpg.resolution = 0;//CAMERIC_ISP_TPG_1080P;
    tpg.pixleGap = 320;
    tpg.lineGap = 240;
    int rc = ioctl(ispDevice, ISPIOC_S_TPG, &tpg);
    if (rc < 0)
        return rc;
    return ioctl(ispDevice, ISPIOC_ENABLE_TPG, 1);
}

int configIS() {
    struct isp_is_context is;
    is.enable = false;
    is.window.x = is.window.y = is.window.width = is.window.height = 0;
    return ioctl(ispDevice, ISPIOC_S_IS, &is);
}
int size = TEST_RES_WIDTH*TEST_RES_HEIGHT;
#define  y_addr  0xB0000000

int setOutputBuffer() {
    struct isp_buffer_context buf;
    buf.type = ISP_PICBUF_TYPE_YCbCr422;
    buf.path = 0;  // 0, 1, 2
    buf.addr_y = y_addr >> 2;
    buf.size_y = size;
    buf.addr_cb = (y_addr + buf.size_y) >> 2;
    buf.size_cb = size;
    buf.addr_cr = 0;
    buf.size_cr = 0;
    return ioctl(ispDevice, ISPIOC_SET_BUFFER, &buf);
}

int startMI() {
    struct isp_mi_context mi;
    memset(&mi, 0, sizeof(mi));
    mi.burst_len = MRV_MI_BURST_LEN_CHROM_4;
    mi.path[0].enable = true;
    mi.path[0].out_mode = IC_MI_DATAMODE_YUV422;
    mi.path[0].in_mode = IC_MI_DATAMODE_RAW8;
    mi.path[0].data_layout = IC_MI_DATASTORAGE_SEMIPLANAR;
    mi.path[0].hscale = false;
    mi.path[0].vscale = false;
    mi.path[0].in_width = mi.path[0].out_width = TEST_RES_WIDTH;
    mi.path[0].in_height = mi.path[0].out_height = TEST_RES_HEIGHT;
    return ioctl(ispDevice, ISPIOC_MI_START, &mi);
}

int stopMI() {
    return ioctl(ispDevice, ISPIOC_MI_STOP, 0);
}

int startStream() {
    uint32_t n = 1;
    return ioctl(ispDevice, ISPIOC_START_CAPTURE, &n);
}

int stopStream() {
    return ioctl(ispDevice, ISPIOC_ISP_STOP, 0);
}

int camDevice ;

void saveBuffer() {
    uint32_t addr = y_addr;

    unsigned char* data = (unsigned char*)mmap(NULL, size*2, PROT_READ | PROT_WRITE, MAP_SHARED, camDevice, addr);
    if (data) {
        FILE* pFile = fopen("test.yuv", "wb");
        fwrite(data, size*2, 1, pFile);
        fclose(pFile);
    } else {
        ALOGE("can't map data");
    }
}

void configISP() {
    configInput();
    configDataPath();
    configAwb();
    configTpg();
}

void writeReg(uint32_t reg, uint32_t val) {

    struct isp_reg_t isp_reg;
    isp_reg.offset = reg;
    isp_reg.val = val;
    ioctl(ispDevice, ISPIOC_WRITE_REG, &isp_reg);
}


void testISP() {
    struct isp_reg_t isp_reg;
    isp_reg.offset = 0x08;
    ioctl(ispDevice, ISPIOC_READ_REG, &isp_reg);
    ALOGI("read isp version 0x%08x", isp_reg.val);
    ioctl(ispDevice, ISPIOC_RESET, 0);
    configISP();
    setOutputBuffer();
    startMI();
    writeReg(0x400, 0x000008c6);
    writeReg(0x404, 0x0000405d);

    startStream();
    sleep(1);
    // read output buffer.
    saveBuffer();
    stopMI();
    stopStream();
}


int main(int argc, char* argv[]) {
    const char* deviceName = "/dev/video0";
    camDevice = open(deviceName, O_RDWR | O_NONBLOCK);
    ALOGI("open camdev ret : %d", camDevice);
    ispDevice = open("/dev/v4l-subdev0", O_RDWR | O_NONBLOCK);
    ALOGI("open subdev ret : %d", ispDevice);
    testISP();
    return 0;
}
