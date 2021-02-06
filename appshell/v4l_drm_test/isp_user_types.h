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

#ifndef _ISP_USER_TYPES_H_
#define _ISP_USER_TYPES_H_

#define MRV_ISP_ISP_MODE_RAW                    0   // 000 - RAW picture with BT.601 sync (ISP bypass)
#define MRV_ISP_ISP_MODE_656                    1   // 001 - ITU-R BT.656 (YUV with embedded sync)
#define MRV_ISP_ISP_MODE_601                    2   // 010 - ITU-R BT.601 (YUV input with H and Vsync signals)
#define MRV_ISP_ISP_MODE_RGB                    3   // 011 - Bayer RGB processing with H and Vsync signals
#define MRV_ISP_ISP_MODE_DATA                   4   // 100 - data mode (ISP bypass, sync signals interpreted as data enable)
#define MRV_ISP_ISP_MODE_RGB656                 5   // 101 - Bayer RGB processing with BT.656 synchronization
#define MRV_ISP_ISP_MODE_RAW656                 6   // 110 - RAW picture with ITU-R BT.656 synchronization (ISP bypass)

#define MRV_ISP_BAYER_PAT_RG                    0   // 00 - first line: RGRG, second line: GBGB, etc.
#define MRV_ISP_BAYER_PAT_GR                    1   // 01 - first line: GRGR, second line: BGBG, etc.
#define MRV_ISP_BAYER_PAT_GB                    2   // 10 - first line: GBGB, second line: RGRG, etc.
#define MRV_ISP_BAYER_PAT_BG                    3   // 11 - first line: BGBG, second line: GRGR, etc.

#define MRV_ISP_CONV_422_CO                     0   // 00- co-sited color subsampling Y0Cb0Cr0 - Y1
#define MRV_ISP_CONV_422_INTER                  1   // 01- interleaved color subsampling Y0Cb0 - Y1Cr1 (not recommended)
#define MRV_ISP_CONV_422_NONCO                  2   // 10- non-cosited color subsampling Y0Cb(0+1)/2 - Y1Cr(0+1)/2

#define MRV_ISP_INPUT_SELECTION_12EXT           0   // 000- 12Bit external Interface
#define MRV_ISP_INPUT_SELECTION_10ZERO          1   // 001- 10Bit Interface, append 2 zeroes as LSBs
#define MRV_ISP_INPUT_SELECTION_10MSB           2   // 010- 10Bit Interface, append 2 MSBs as LSBs
#define MRV_ISP_INPUT_SELECTION_8ZERO           3   // 011- 8Bit Interface, append 4 zeroes as LSBs
#define MRV_ISP_INPUT_SELECTION_8MSB            4   // 100- 8Bit Interface, append 4 MSBs as LSBs

#define MRV_ISP_LATENCY_FIFO_SELECTION_INPUT_FORMATTER  0
#define MRV_ISP_LATENCY_FIFO_SELECTION_DMA_READ         1

#define MRV_VI_MP_MUX_JPGDIRECT                 0x00    // 00: reserved (future: data from DMA read port to JPEG encoder)
#define MRV_VI_MP_MUX_MP                        0x01    // 01: data from main resize to MI, uncompressed
#define MRV_VI_MP_MUX_RAW                       0x01    //     (now also used for RAW data bypass)
#define MRV_VI_MP_MUX_JPEG                      0x02    // 10: data from main resize to JPEG encoder
#define MRV_VI_DMA_SPMUX_CAM                    0   // 0: data from camera interface to self resize
#define MRV_VI_DMA_SPMUX_DMA                    1   // 1: data from DMA read port to self resize

#define MRV_VI_DMA_IEMUX_CAM                    0   // 0: data from camera interface to image effects
#define MRV_VI_DMA_IEMUX_DMA                    1   // 1: data from DMA read port to image effects

#define MRV_IF_SELECT_PAR                       0   // 0: parallel interface
#define MRV_IF_SELECT_SMIA                      1   // 1: SMIA-interface
#define MRV_IF_SELECT_MIPI                      2   // 2: MIPI-interface

#define MRV_ISP_AWB_MODE_MEAS                   2U  // 10: white balance measurement of YCbCr or RGB means (dependent on MRV_ISP_AWB_MEAS_MODE)
#define MRV_ISP_AWB_MODE_NOMEAS                 0U  // 00: manual white balance (gain adjust possible), no measurement
#define MRV_ISP_AWB_MEAS_MODE_RGB               1U  // 1: RGB based measurement mode 
#define MRV_ISP_AWB_MEAS_MODE_YCBCR             0U  // 0: near white discrimination mode using YCbCr color space

#define MRV_MI_BURST_LEN_CHROM_4                0   // 0: 4-beat bursts
#define MRV_MI_BURST_LEN_CHROM_8                1   // 1: 8-beat bursts
#define MRV_MI_BURST_LEN_CHROM_16               2   // 2: 16-beat bursts

#endif  // _ISP_USER_TYPES_H_
