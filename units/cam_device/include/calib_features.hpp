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

#pragma once

#include "calib_element.hpp"
#include <cam_engine/cam_engine_api.h>
#include <vector>

#include <algorithm>
#include <cam_engine/cam_engine_aaa_api.h>
#include <cam_engine/cam_engine_cproc_api.h>
#include <cam_engine/cam_engine_imgeffects_api.h>
#include <cam_engine/cam_engine_isp_api.h>
#include <cam_engine/cam_engine_jpe_api.h>
#include <cam_engine/cam_engine_mi_api.h>
#include <cam_engine/cam_engine_simp_api.h>

#include <cameric_drv/cameric_drv_api.h>
#include <cameric_drv/cameric_ie_drv_api.h>

#include <string>
#include <tinyxml2/tinyxml2.h>

#include <cameric_drv/cameric_isp_cac_drv_api.h>
#include "base64/base64.hpp"
#include "macros.hpp"

#include "calib_defs.hpp"

#if defined CTRL_DEWARP
#include "DewarpDriver.h" //to do
#endif

#define DEWARP_CORE_MAX 2

namespace camdev {

#define AVS_NUM_ITP_POINTS_DEFAULT 17
#define AVS_THETA_DEFAULT 0.5
#define AVS_BASE_GAIN_DEFAULT 1.0
#define AVS_FALL_OFF_DEFAULT 0.5
#define AVS_ACCELERATION_DEFAULT 4.0

#define AVS_DAMP_DATA_MAX 50

#define WDR_CURVE_ARRAY_SIZE 33
struct CalibAe : Element {
    CalibAe(XMLDocument &);

    void composeAttributes(XMLElement &) override;
    void composeSubElements(XMLElement &) override;

    void parseAttributes(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    bool isEnable = true;

    struct Config {
        CamEngineAecSemMode_t mode = CAM_ENGINE_AEC_SCENE_EVALUATION_DISABLED;
        float dampingOver = 0.3;
        float dampingUnder = 0.2;
        float setPoint = 80;
        float tolerance = 20;
    };

    Config config;

    struct Ecm {
        CamEngineFlickerPeriod_t flickerPeriod = CAM_ENGINE_FLICKER_100HZ;

        bool isAfps = false;
    };

    Ecm ecm;

    struct Status {
        CamEngineAecHistBins_t histogram;
        CamEngineAecMeanLuma_t luminance;
        CamEngineAecMeanLuma_t objectRegion;
    };

    Status status;
};

struct CalibAf : Element {
    CalibAf(XMLDocument &);

    void composeAttributes(XMLElement &) override;
    void composeSubElements(XMLElement &) override;

    void parseAttributes(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    bool isEnable = true;

    struct Config {
        bool isOneshot = false;

        CamEngineAfSearchAlgorithm_t searchAlgorithm =
                CAM_ENGINE_AUTOFOCUS_SEARCH_ALGORITHM_ADAPTIVE_RANGE;
    };

    Config config;
};

typedef enum CamEngineGammaOutXScale_e {
    CAM_ENGINE_GAMMAOUT_XSCALE_INVALID =
            0, /**< lower border (only for an internal evaluation) */
    CAM_ENGINE_GAMMAOUT_XSCALE_LOG =
            1, /**< logarithmic segmentation from 0 to 4095
                            (64,64,64,64,128,128,128,128,256,256,256,512,512,512,512,512) */
    CAM_ENGINE_GAMMAOUT_XSCALE_EQU = 2, /**< equidistant segmentation from 0 to
                                                                                 4095 (256, 256, ... ) */
    CAM_ENGINE_GAMMAOUT_XSCALE_MAX            /**< upper border (only for an internal
                                                                                 evaluation) */
} CamEngineGammaOutXScale_t;

typedef struct CamEngineGammaOutCurve_s {
    CamEngineGammaOutXScale_t xScale;
    uint16_t GammaY[CAMERIC_ISP_GAMMA_CURVE_SIZE];
} CamEngineGammaOutCurve_t;

const CamEngineGammaOutCurve_t standardCurve = {
    CAM_ENGINE_GAMMAOUT_XSCALE_LOG,
    {0x000, 0x049, 0x089, 0x0B7, 0x0DF, 0x11F, 0x154, 0x183, 0x1AD, 0x1F6,
     0x235, 0x26F, 0x2D3, 0x32A, 0x378, 0x3BF, 0x3FF}};

struct CalibGc : Element {
    CalibGc(XMLDocument &);

    void composeAttributes(XMLElement &) override;
    void composeSubElements(XMLElement &) override;

    void parseAttributes(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    bool isEnable = true;

    struct Config {
        CamEngineGammaOutCurve_t curve;
    };

    Config config;
};

#define CLB_HDR_SENSOR_CURVE_POINT 64
struct CalibHdr : Element {
    CalibHdr(XMLDocument &document);

    void composeAttributes(XMLElement &) override;
    void composeSubElements(XMLElement &) override;

    void parseAttributes(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    bool isEnable = false;

    struct Config {
        float exposureRatio = 16.0f;
        uint8_t extensionBit = 3;
    };

    Config config;

    struct Sensor {
        float curve[CLB_HDR_SENSOR_CURVE_POINT * 2];
    } sensor;
};

struct CalibImage : Element {
    CalibImage(XMLDocument &document) : Element(document) {}

    void composeSubElements(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    struct Config {
        std::string fileName;
    } config;
};

struct CalibImages : Element {
    CalibImages(XMLDocument &);

    void composeSubElements(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    std::vector<CalibImage> images;
};

struct CalibLsc : Element {
    CalibLsc(XMLDocument &);

    void composeAttributes(XMLElement &) override;
    void composeSubElements(XMLElement &) override;

    void parseAttributes(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    bool isEnable = true;

    struct Config {
      bool isAdaptive = false;
    } config;

    struct Status {
      CamEngineLscConfig_t config;
    };
};

struct CalibCnr : Element {
    CalibCnr(XMLDocument &);

    void composeAttributes(XMLElement &) override;
    void composeSubElements(XMLElement &) override;

    void parseAttributes(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    bool isEnable = true;

    struct Config {
        uint32_t tc1 = 10;
        uint32_t tc2 = 50;
        uint32_t line_width=1920;
    };

    Config config;
};

struct CalibNr2d : Element {
    CalibNr2d(XMLDocument &);

    void composeSubElements(XMLElement &) override;

    void parseSubElements(XMLElement &) override;

    enum Generation {
        Nr2Dv1,
        Nr2Dv2,
        Nr2Dv3,
        Nr2DSum,
    };

    union Config {
      struct Nr2Dv1 {
        void reset();

        bool isAuto;

        int denoisePregamaStrength;
        int denoiseStrength;
        double sigma;
      } v1;

      struct Nr2Dv2 {
        void reset();

        int32_t dummy;
      } v2;

      struct Nr2Dv3 {
        void reset();

        int32_t dummy;
      } v3;
    };

    struct Status {
      double gain;
      double intergrationTime;
    };

    struct Table {
      enum ColumnV1 {
        Hdr,
        Gain,
        IntergrationTime,
        PregammaStrength,
        DenoiseStrength,
        Sigma,
      };

      void reset(Generation generation);

      Json::Value jTable;
    };

    struct Holder {
      bool isEnable = false;

      Config config;
      Table table;
    };

    std::vector<Holder> holders;
};

struct CalibNr3d : Element {
    CalibNr3d(XMLDocument &);

    void composeSubElements(XMLElement &) override;

    void parseSubElements(XMLElement &) override;

    enum Generation {
        Nr3Dv1,
        Nr3Dv2,
        Nr3Dv3,
        Nr3DSum,
    };

    union Config {
      struct Nr3Dv1 {
        void reset();

        bool isAuto;

        int32_t deltaFactor;
        int32_t motionFactor;
        int32_t strength;
      } v1;

      struct Nr3Dv2 {
        void reset();

        int32_t dummy;
      } v2;

      struct Nr3Dv3 {
        void reset();

        int32_t dummy;
      } v3;
    };

    struct Status {
      double gain;
      double intergrationTime;
    };

    struct Table {
      enum ColumnV1 {
        Hdr,
        Gain,
        IntergrationTime,
        Strength,
        MotionFactor,
        DeltaFactor,
      };

      void reset(Generation Generation);

      Json::Value jTable;
    };

    struct Holder {
      bool isEnable = false;

      Config config;
      Table table;
    };

    std::vector<Holder> holders;
};

struct CalibSimp : Element {
    CalibSimp(XMLDocument &document);

    void composeAttributes(XMLElement &) override;
    void composeSubElements(XMLElement &) override;

    void parseAttributes(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    bool isEnable = true;

    struct Config {
        Config() { REFSET(config, 0); }

        std::string fileName;

        CamEngineSimpConfig_t config;
    };

    Config config;
};

struct CalibIe : Element {
    CalibIe(XMLDocument &);

    void composeAttributes(XMLElement &) override;
    void composeSubElements(XMLElement &) override;

    void parseAttributes(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    bool isEnable = true;

    struct Config {
        Config() {
            config.mode = CAMERIC_IE_MODE_COLOR;
            config.range = CAMERIC_IE_RANGE_BT601;

            if (config.mode == CAMERIC_IE_MODE_COLOR) {
                config.ModeConfig.ColorSelection.col_selection =
                        CAMERIC_IE_COLOR_SELECTION_RGB;
            }
        }

        CamerIcIeConfig_t config;
    };

    Config config;
};

struct CalibWdr : Element {
    CalibWdr(XMLDocument &);

    void composeSubElements(XMLElement &) override;

    void parseSubElements(XMLElement &) override;

    enum Generation { Wdr1, Wdr2, Wdr3, WdrMax };

    union Config {
        struct Wdr1 {
            void reset();
            CamEngineWdrCurve_t curve;
        } wdr1;

        struct Wdr2 {
            void reset();
            float strength;
        } wdr2;

        struct Wdr3 {
            void reset();

            bool isAuto;

            int strength;
            int gainMax;
            int strengthGlobal;
        } wdr3;
    };

    struct Status {
        double gain;
        double intergrationTime;
    };

    struct Table {
        enum ColumnV3 {
            Hdr,
            Gain,
            IntergrationTime,
            Strength,
            MaxGain,
            GlobalCurve,
        };

        void reset(Generation generation);

        Json::Value jTable;
    };

    struct Holder {
       // Holder() { REFSET(config, 0); }

        bool isEnable = true;
        Config config;
        Table table;
    };
    std::vector<Holder> holders;
};

struct CalibEe : Element {
    CalibEe(XMLDocument &document);

    void composeAttributes(XMLElement &) override;
    void composeSubElements(XMLElement &) override;

    void parseAttributes(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    bool isEnable = false;

    struct Config {
        bool isAuto = true;
        CamEngineEeConfig_t config;

        void reset();
    };

    Config config;

    struct Status {
      double gain;
      double intergrationTime;
    };

    struct Table {
      enum Column {
        Hdr,
        Gain,
        IntergrationTime,
        EdgeGain,
        Strength,
        UvGain,
        YGainDown,
        YGainUp,
      };

      void reset();

      Json::Value jTable;
    } table;
};


struct CalibInput : Element {
    CalibInput(XMLDocument &document) : Element(document) {}

    enum Type { Invalid, Sensor, Image, Tpg, Max };

    void composeSubElements(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    struct Config {
        Type type;
    } config;
};

struct CalibInputs : Element {
    CalibInputs(XMLDocument &);

    void composeSubElements(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    struct Config {
        int32_t index = 0;
    } config;

    CalibInput &input() { return inputs[config.index]; }

    std::vector<CalibInput> inputs;
};

struct CalibJpe : Element {
    CalibJpe(XMLDocument &document);

    void composeAttributes(XMLElement &) override;
    void composeSubElements(XMLElement &) override;

    void parseAttributes(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    bool isEnable = false;

    struct Config {
        uint16_t width = 0;
        uint16_t height = 0;
    };

    Config config;
};

struct CalibEc : Element {
    CalibEc(XMLDocument &);

    void composeSubElements(XMLElement &) override;

    void parseSubElements(XMLElement &) override;

    struct Config {
        float gain = 0;
        float integrationTime = 0;
        float hdrRatio = 0;
        float minGain;
        float maxGain;
    } config;

    struct Status {
        struct Unit {
            float max = 0;
            float min = 0;
            float step = 0;
        };

        Unit gain;
        Unit integrationTime;
    };
};


struct CalibAwb : Element {
    CalibAwb(XMLDocument &);

    void composeAttributes(XMLElement &) override;
    void composeSubElements(XMLElement &) override;

    void parseAttributes(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    bool isEnable = true;

    struct Config {
        uint32_t index = 1;
        bool isDamping = true;
        CamEngineAwbMode_t mode = CAM_ENGINE_AWB_MODE_AUTO;
    };

    Config config;

    struct Status {
        CamEngineAwbRgProj_t rgProj;
    };
};


struct CalibWb : Element {
    CalibWb(XMLDocument &document);

    void composeSubElements(XMLElement &) override;

    void parseSubElements(XMLElement &) override;

    struct Config {
        Config() {
            int32_t i = 0;

            ccMatrix.Coeff[i++] = 1.586;
            ccMatrix.Coeff[i++] = -0.070;
            ccMatrix.Coeff[i++] = -0.461;
            ccMatrix.Coeff[i++] = -0.609;
            ccMatrix.Coeff[i++] = 1.984;
            ccMatrix.Coeff[i++] = -0.234;
            ccMatrix.Coeff[i++] = 0.008;
            ccMatrix.Coeff[i++] = -1.219;
            ccMatrix.Coeff[i++] = 2.508;

            REFSET(ccOffset, 0);

            wbGains.Red = 1.367;
            wbGains.GreenB = 1;
            wbGains.GreenB = 1;
            wbGains.Blue = 2.930;
        }

        CamEngineCcMatrix_t ccMatrix;
        CamEngineCcOffset_t ccOffset;
        CamEngineWbGains_t wbGains;
    };

    Config config;
};


struct CalibDpf : Element {
    CalibDpf(XMLDocument &);

    void composeAttributes(XMLElement &) override;
    void composeSubElements(XMLElement &) override;

    void parseAttributes(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    bool isEnable = true;

    struct Config {
        bool isAdaptive;

        float gradient = 0.15;
        float offset = 0;
        float minimumBound = 2;
        float divisionFactor = 64;

        uint8_t sigmaGreen = 4;
        uint8_t sigmaRedBlue = 4;
    };

    Config config;
};

struct CalibDpcc : Element {
    CalibDpcc(XMLDocument &document);

    void composeAttributes(XMLElement &) override;

    void parseAttributes(XMLElement &) override;

    bool isEnable = true;
};

struct CalibDemosaic : Element {
    CalibDemosaic(XMLDocument &);

    void composeAttributes(XMLElement &) override;
    void composeSubElements(XMLElement &) override;

    void parseAttributes(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    bool isEnable = true;

    struct Config {
        enum Mode {
            Normal = 1,
            Bypass,
        } mode;

        int32_t threshold = 0;
    };

    Config config;
};

struct CalibCproc : Element {
    CalibCproc(XMLDocument &);

    void composeAttributes(XMLElement &) override;
    void composeSubElements(XMLElement &) override;

    void parseAttributes(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    bool isEnable = false;

    struct Config {
        Config() {
            config.ChromaOut = CAMERIC_CPROC_CHROM_RANGE_OUT_BT601;
            config.LumaOut = CAMERIC_CPROC_LUM_RANGE_OUT_BT601;
            config.LumaIn = CAMERIC_CPROC_LUM_RANGE_IN_BT601;

            config.contrast = 1;
            config.brightness = 0;
            config.saturation = 1;
            config.hue = 0;
        }

        CamEngineCprocConfig_t config;
    };

    Config config;
};

struct CalibCac : Element {
    CalibCac(XMLDocument &);

    void composeAttributes(XMLElement &) override;
    void composeSubElements(XMLElement &) override;

    void parseAttributes(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    bool isEnable = true;

    struct Config {
        CamerIcIspCacConfig_t config;
    };

    Config config;
};

struct CalibBls : Element {
    CalibBls(XMLDocument &);

    void composeSubElements(XMLElement &) override;

    void parseSubElements(XMLElement &) override;

    struct Config {
        uint16_t red = 64;
        uint16_t greenB = 64;
        uint16_t greenR = 64;
        uint16_t blue = 64;
    };

    Config config;
};

struct CalibAvs : Element {
    CalibAvs(XMLDocument &);

    void composeAttributes(XMLElement &) override;
    void composeSubElements(XMLElement &) override;

    void parseAttributes(XMLElement &) override;
    void parseSubElements(XMLElement &) override;

    bool isEnable = false;

    struct Config {
        void reset() {
          acceleration = AVS_ACCELERATION_DEFAULT;
          baseGain = AVS_BASE_GAIN_DEFAULT;
          fallOff = AVS_FALL_OFF_DEFAULT;
          numItpPoints = AVS_NUM_ITP_POINTS_DEFAULT;
          theta = AVS_THETA_DEFAULT;
        }
        bool isUseParams = false;

        float acceleration = 0;
        float baseGain = 0;
        float fallOff = 0;
        uint16_t numItpPoints = 0;
        float theta = 0;

        std::vector<double> xes;
        std::vector<double> ys;

        int32_t dampCount;
        double x[AVS_DAMP_DATA_MAX];
        double y[AVS_DAMP_DATA_MAX];
    };

    Config config;

    struct Status {
        std::pair<int16_t, int16_t> displacement;
        std::pair<int16_t, int16_t> offset;
    };
};


struct CalibPaths : Element {
    CalibPaths(XMLDocument &);

    void composeSubElements(XMLElement &) override;

    void parseSubElements(XMLElement &) override;

    void pathComposeSubElements(XMLElement &element, CamEnginePathConfig_t &config);

    void pathParseSubElements(XMLElement &element, CamEnginePathConfig_t &config);

    void reset();

    struct Config {
        CamEnginePathConfig_t config[CAMERIC_MI_PATH_MAX];
    } config;
};



struct CalibSensor : Element {
    CalibSensor(XMLDocument &document) : Element(document), config(document) {}

    void composeSubElements(XMLElement &) override;

    void parseSubElements(XMLElement &) override;

    struct Config {
        Config(XMLDocument &document) : ec(document) {}

        std::string calibFileName;
        std::string driverFileName;
        bool isTestPattern = false;

        CalibEc ec;
    };

    Config config;
};

struct CalibSensors : Element {
    CalibSensors(XMLDocument &);

    void composeSubElements(XMLElement &) override;

    void parseSubElements(XMLElement &) override;

    std::vector<CalibSensor> sensors;
};

struct CalibDehaze : Element {
  CalibDehaze(XMLDocument &);

  struct Config {
    CalibWdr::Config wdr3Config;
    CalibCproc::Config cprocConfig;
  } config;

  struct Enable {
    bool isCproc;
    bool isGwdr;
    bool isWdr3;
  } enable;
};

struct CalibFilter : Element {
  CalibFilter(XMLDocument &);

  void composeAttributes(XMLElement &) override;
  void composeSubElements(XMLElement &) override;

  void parseAttributes(XMLElement &) override;
  void parseSubElements(XMLElement &) override;

  bool isEnable = true;

  struct Config {
    void reset();

    bool isAuto;

    int32_t denoise;
    int32_t sharpen;
  } config;

  struct Status {
    double gain;
    double intergrationTime;
  };

  struct Table {
    enum Column {
      Hdr,
      Gain,
      IntergrationTime,
      Denoising,
      Sharpening,
    };

    void reset();

    Json::Value jTable;
  } table;
};

struct CalibDewarp : Element {
  CalibDewarp(XMLDocument &);
  ~CalibDewarp();

  void composeAttributes(XMLElement &) override;
  void composeSubElements(XMLElement &) override;

  void parseAttributes(XMLElement &) override;
  void parseSubElements(XMLElement &) override;

  bool isEnable = false;

  struct Config {
#if defined CTRL_DEWARP
    Config() {
      REFSET(params, 0);

      params.pix_fmt_in = MEDIA_PIX_FMT_YUV422SP;
      params.pix_fmt_out = MEDIA_PIX_FMT_YUV422SP;
      params.scale_factor = 4096;
      params.boundary_pixel.Y = 0;
      params.boundary_pixel.U = 128;
      params.boundary_pixel.V = 128;
      params.image_size.width = 1920;
      params.image_size.height = 1080;
      params.is_split = 0;
      params.split_horizon_line = 716;
      params.split_vertical_line_up = 716;
      params.split_vertical_line_down = 716;
      params.dewarp_type = DEWARP_MODEL_LENS_DISTORTION_CORRECTION;

      distortionMap[0] = {
          0,
          0,
          {6.5516074404594690e+002, 0.0, 9.6420599053623062e+002, 0.0,
           6.5552406676868952e+002, 5.3203601317192908e+002, 0.0, 0.0, 1.0},
          {1.0, 0, 0, 0, 1, 0, 0, 0, 1},
          {-2.2095698671518085e-002, 3.8543889520066955e-003,
           -5.9060355970132873e-003, 1.9007362178503509e-003, 0.0, 0.0, 0.0,
           0.0},
          nullptr};

      distortionMap[1] = {
          1,
          0,
          {6.5516074404594690e+002, 0.0, 9.6420599053623062e+002, 0.0,
           6.5552406676868952e+002, 5.3203601317192908e+002, 0.0, 0.0, 1.0},
          {1.0, 0, 0, 0, 1, 0, 0, 0, 1},
          {-2.2095698671518085e-002, 3.8543889520066955e-003,
           -5.9060355970132873e-003, 1.9007362178503509e-003, 0.0, 0.0, 0.0,
           0.0},
          nullptr};
    }

    Config &operator=(const Config &);

    void freeUserMap();

    dewarp_parameters params;
    dewarp_distortion_map distortionMap[DEWARP_CORE_MAX];
#endif
  } config;
};

} // namespace camdev
