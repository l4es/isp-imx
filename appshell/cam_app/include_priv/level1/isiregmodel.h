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

/**
 * @file isiregmodel.h
 *
 * @brief
 *   Table model of the ISI register description.
 *
 *****************************************************************************/
/**
 *
 * @mainpage Module Documentation
 *
 *
 * Doc-Id: xx-xxx-xxx-xxx (NAME Implementation Specification)\n
 * Author: NAME
 *
 * DESCRIBE_HERE
 *
 *
 * The manual is divided into the following sections:
 *
 * -@subpage module_name_api_page \n
 * -@subpage module_name_page \n
 *
 * @page module_name_api_page Module Name API
 * This module is the API for the NAME. DESCRIBE IN DETAIL / ADD USECASES...
 *
 * for a detailed list of api functions refer to:
 * - @ref module_name_api
 *
 * @defgroup module_name_api Module Name API
 * @{
 */

#ifndef __ISIREGMODEL_H__
#define __ISIREGMODEL_H__

#include <QtCore/QAbstractTableModel>
#include "VirtualCamera.h"


//TODO: move to VirtualCamera_api



/*****************************************************************************/
/**
 *          IsiRegisterFlags_t
 *
 * @brief   Register permission enumeration type
 */
/*****************************************************************************/
typedef enum IsiRegisterFlags_e
{
    // basic features
    eTableEnd           = 0x00,                                                 /**< special flag for end of register table */
    eReadable           = 0x01,
    eWritable           = 0x02,
    eVolatile           = 0x04,                                                 /**< register can change even if not written by I2C */
    eDelay              = 0x08,                                                 /**< wait n ms */
    eReserved           = 0x10,
    eNoDefault          = 0x20,                                                 /**< no default value specified */
    eTwoBytes           = 0x40,                                                 /**< SMIA sensors use 8-, 16- and 32-bit registers */
    eFourBytes          = 0x80,                                                 /**< SMIA sensors use 8-, 16- and 32-bit registers */

    // combined features
    eReadOnly           = eReadable,
    eWriteOnly          = eWritable,
    eReadWrite          = eReadable | eWritable,
    eReadWriteDel       = eReadable | eWritable | eDelay,
    eReadWriteVolatile  = eReadable | eWritable | eVolatile,
    eReadWriteNoDef     = eReadable | eWritable | eNoDefault,
    eReadWriteVolNoDef  = eReadable | eWritable | eVolatile | eNoDefault,
    eReadVolNoDef       = eReadable | eVolatile | eNoDefault,
    eReadOnlyVolNoDef   = eReadOnly | eVolatile | eNoDefault,

    // additional SMIA features
    eReadOnly_16            = eReadOnly          | eTwoBytes,
    eReadWrite_16           = eReadWrite         | eTwoBytes,
    eReadWriteDel_16        = eReadWriteDel      | eTwoBytes,
    eReadWriteVolatile_16   = eReadWriteVolatile | eTwoBytes,
    eReadWriteNoDef_16      = eReadWriteNoDef    | eTwoBytes,
    eReadWriteVolNoDef_16   = eReadWriteVolNoDef | eTwoBytes,
    eReadOnlyVolNoDef_16    = eReadOnly_16 | eVolatile | eNoDefault,
    eReadOnly_32            = eReadOnly          | eFourBytes,
    eReadWrite_32           = eReadWrite         | eFourBytes,
    eReadWriteVolatile_32   = eReadWriteVolatile | eFourBytes,
    eReadWriteNoDef_32      = eReadWriteNoDef    | eFourBytes,
    eReadWriteVolNoDef_32   = eReadWriteVolNoDef | eFourBytes
} IsiRegisterFlags_t;



/*****************************************************************************/
/**
 *          IsiRegDescription_t
 *
 * @brief   Sensor register description struct
 */
/*****************************************************************************/
typedef struct IsiRegisterFlags_s
{
    unsigned int    Addr;
    unsigned int    DefaultValue;
    const char*     pName;
    unsigned int    Flags;
} IsiRegDescription_t;


/**<  BusWidth */
#define ISI_BUSWIDTH_8BIT_ZZ                0x00000001U     /**< to expand to a (possibly higher) resolution in marvin, the LSBs will be set to zero */
#define ISI_BUSWIDTH_8BIT_EX                0x00000002U     /**< to expand to a (possibly higher) resolution in marvin, the LSBs will be copied from the MSBs */
#define ISI_BUSWIDTH_10BIT_EX               0x00000004U     /**< /formerly known as ISI_BUSWIDTH_10BIT (at times no marvin derivative was able to process more than 10 bit) */
#define ISI_BUSWIDTH_10BIT_ZZ               0x00000008U
#define ISI_BUSWIDTH_12BIT                  0x00000010U

#define ISI_BUSWIDTH_10BIT      ( ISI_BUSWIDTH_10BIT_EX )


/**< Mode, operating mode of the image sensor in terms of output data format and timing data transmission */
#define ISI_MODE_BT601                      0x00000001      /**< YUV-Data with separate h/v sync lines (ITU-R BT.601) */
#define ISI_MODE_BT656                      0x00000002      /**< YUV-Data with sync words inside the datastream (ITU-R BT.656) */
#define ISI_MODE_BAYER                      0x00000004      /**< Bayer data with separate h/v sync lines */
#define ISI_MODE_DATA                       0x00000008      /**< Any binary data without line/column-structure, (e.g. already JPEG encoded) h/v sync lines act as data valid signals */
#define ISI_MODE_PICT                       0x00000010      /**< RAW picture data with separate h/v sync lines */
#define ISI_MODE_RGB565                     0x00000020      /**< RGB565 data with separate h/v sync lines */
#define ISI_MODE_SMIA                       0x00000040      /**< SMIA conform data stream (see SmiaMode for details) */
#define ISI_MODE_MIPI                       0x00000080      /**< MIPI conform data stream (see MipiMode for details) */
#define ISI_MODE_BAY_BT656                  0x00000100      /**< Bayer data with sync words inside the datastream (similar to ITU-R BT.656) */
#define ISI_MODE_RAW_BT656                  0x00000200      /**< Raw picture data with sync words inside the datastream (similar to ITU-R BT.656) */

/**< SmiaMode */
#define ISI_SMIA_MODE_COMPRESSED            0x00000001      //!< compression mode
#define ISI_SMIA_MODE_RAW_8_TO_10_DECOMP    0x00000002      //!< 8bit to 10 bit decompression
#define ISI_SMIA_MODE_RAW_12                0x00000004      //!< 12 bit RAW Bayer Data
#define ISI_SMIA_MODE_RAW_10                0x00000008      //!< 10 bit RAW Bayer Data
#define ISI_SMIA_MODE_RAW_8                 0x00000010      //!< 8 bit RAW Bayer Data
#define ISI_SMIA_MODE_RAW_7                 0x00000020      //!< 7 bit RAW Bayer Data
#define ISI_SMIA_MODE_RAW_6                 0x00000040      //!< 6 bit RAW Bayer Data
#define ISI_SMIA_MODE_RGB_888               0x00000080      //!< RGB 888 Display ready Data
#define ISI_SMIA_MODE_RGB_565               0x00000100      //!< RGB 565 Display ready Data
#define ISI_SMIA_MODE_RGB_444               0x00000200      //!< RGB 444 Display ready Data
#define ISI_SMIA_MODE_YUV_420               0x00000400      //!< YUV420 Data
#define ISI_SMIA_MODE_YUV_422               0x00000800      //!< YUV422 Data
#define ISI_SMIA_OFF                        0x80000000      //!< SMIA is disabled

/**< MipiMode */
#define ISI_MIPI_MODE_YUV420_8              0x00000001      //!< YUV 420  8-bit
#define ISI_MIPI_MODE_YUV420_10             0x00000002      //!< YUV 420 10-bit
#define ISI_MIPI_MODE_LEGACY_YUV420_8       0x00000004      //!< Legacy YUV 420 8-bit
#define ISI_MIPI_MODE_YUV420_CSPS_8         0x00000008      //!< YUV 420 8-bit (CSPS)
#define ISI_MIPI_MODE_YUV420_CSPS_10        0x00000010      //!< YUV 420 10-bit (CSPS)
#define ISI_MIPI_MODE_YUV422_8              0x00000020      //!< YUV 422 8-bit
#define ISI_MIPI_MODE_YUV422_10             0x00000040      //!< YUV 422 10-bit
#define ISI_MIPI_MODE_RGB444                0x00000080      //!< RGB 444
#define ISI_MIPI_MODE_RGB555                0x00000100      //!< RGB 555
#define ISI_MIPI_MODE_RGB565                0x00000200      //!< RGB 565
#define ISI_MIPI_MODE_RGB666                0x00000400      //!< RGB 666
#define ISI_MIPI_MODE_RGB888                0x00000800      //!< RGB 888
#define ISI_MIPI_MODE_RAW_6                 0x00001000      //!< RAW_6
#define ISI_MIPI_MODE_RAW_7                 0x00002000      //!< RAW_7
#define ISI_MIPI_MODE_RAW_8                 0x00004000      //!< RAW_8
#define ISI_MIPI_MODE_RAW_10                0x00008000      //!< RAW_10
#define ISI_MIPI_MODE_RAW_12                0x00010000      //!< RAW_12
#define ISI_MIPI_OFF                        0x80000000      //!< MIPI is disabled

/**< FieldSelection */
#define ISI_FIELDSEL_BOTH                   0x00000001      /**< sample all field (don't care about fields */
#define ISI_FIELDSEL_EVEN                   0x00000002      /**< sample only even fields */
#define ISI_FIELDSEL_ODD                    0x00000004      /**< sample only odd fields */

/**< YCSeq */
#define ISI_YCSEQ_YCBYCR                    0x00000001
#define ISI_YCSEQ_YCRYCB                    0x00000002
#define ISI_YCSEQ_CBYCRY                    0x00000004
#define ISI_YCSEQ_CRYCBY                    0x00000008

/**< Conv422 */
#define ISI_CONV422_COSITED                 0x00000001
#define ISI_CONV422_INTER                   0x00000002
#define ISI_CONV422_NOCOSITED               0x00000004

/**< BayerPatttern */
#define ISI_BPAT_RGRGGBGB                   0x00000001
#define ISI_BPAT_GRGRBGBG                   0x00000002
#define ISI_BPAT_GBGBRGRG                   0x00000004
#define ISI_BPAT_BGBGGRGR                   0x00000008

/**< HPolarity */
#define ISI_HPOL_SYNCPOS                    0x00000001      /**< sync signal pulses high between lines */
#define ISI_HPOL_SYNCNEG                    0x00000002      /**< sync signal pulses low between lines */
#define ISI_HPOL_REFPOS                     0x00000004      /**< reference signal is high as long as sensor puts out line data */
#define ISI_HPOL_REFNEG                     0x00000008      /**< reference signal is low as long as sensor puts out line data */

/**< VPolarity */
#define ISI_VPOL_POS                        0x00000001
#define ISI_VPOL_NEG                        0x00000002

/**< Edge */
#define ISI_EDGE_RISING                     0x00000001
#define ISI_EDGE_FALLING                    0x00000002

/**< Bls (Black Level Substraction) */
#define ISI_BLS_OFF                         0x00000001      /**< turns on/off additional black lines at frame start */
#define ISI_BLS_TWO_LINES                   0x00000002
#define ISI_BLS_FOUR_LINES                  0x00000004      /**< two lines top and two lines bottom */
#define ISI_BLS_SIX_LINES                   0x00000008      /**< six lines top */

/**< Gamma */
#define ISI_GAMMA_ON                        0x00000001      /**< turns on/off gamma correction in the sensor ISP */
#define ISI_GAMMA_OFF                       0x00000002

/**< ColorConv */
#define ISI_CCONV_ON                        0x00000001      /**< turns on/off color conversion matrix in the sensor ISP */
#define ISI_CCONV_OFF                       0x00000002

/**< Resolution */
#define ISI_RES_VGA                         0x00000001      /**<  1  640x480    */
#define ISI_RES_2592_1944                   0x00000002      /**<  2 2592x1944   */
#define ISI_RES_3264_2448                   0x00000004      /**<  3 3264x2448   */
#define ISI_RES_4416_3312                   0x00000010      /**<  5 4416x3312   */
#define ISI_RES_TV720P5                     0x00010000      /**< 17 1280x720    */
#define ISI_RES_TV720P10                    0x00020000      /**< 18 1280x720    */

#define ISI_RES_TV720P15                    0x00040000      /**< 19 1280x720    */
#define ISI_RES_TV720P30                    0x00080000      /**< 20 1280x720    */
#define ISI_RES_TV720P60                    0x00100000      /**< 21 1280x720    */
#define ISI_RES_TV1080P5                    0x00200000      /**< 22 1920x1080   */
#define ISI_RES_TV1080P6                    0x00400000      /**< 23 1920x1080   */
#define ISI_RES_TV1080P10                   0x00800000      /**< 24 1920x1080   */
#define ISI_RES_TV1080P12                   0x01000000      /**< 25 1920x1080   */
#define ISI_RES_TV1080P15                   0x02000000      /**< 26 1920x1080   */
#define ISI_RES_TV1080P20                   0x04000000      /**<    1920x1080   */
#define ISI_RES_TV1080P24                   0x08000000      /**< 28 1920x1080   */
#define ISI_RES_TV1080P25                   0x10000000      /**< 29 1920x1080   */
#define ISI_RES_TV1080P30                   0x20000000      /**< 20 1920x1080   */
#define ISI_RES_TV1080P50                   0x40000000      /**< 31 1920x1080   */
#define ISI_RES_TV1080P60                   0x80000000      /**< 32 1920x1080   */

/**< DwnSz */
#define ISI_DWNSZ_SUBSMPL                   0x00000001      //!< Use subsampling to downsize output window
#define ISI_DWNSZ_SCAL_BAY                  0x00000002      //!< Use scaling with Bayer sampling to downsize output window
#define ISI_DWNSZ_SCAL_COS                  0x00000004      //!< Use scaling with co-sited sampling to downsize output window

/**< BLC */
#define ISI_BLC_AUTO                        0x00000001      /**< Camera BlackLevelCorrection on */
#define ISI_BLC_OFF                         0x00000002      /**< Camera BlackLevelCorrection off */

/**< AGC */
#define ISI_AGC_AUTO                        0x00000001      /**< Camera AutoGainControl on */
#define ISI_AGC_OFF                         0x00000002      /**< Camera AutoGainControl off */

/**< AWB */
#define ISI_AWB_AUTO                        0x00000001      /**< Camera AutoWhiteBalance on */
#define ISI_AWB_OFF                         0x00000002      /**< Camera AutoWhiteBalance off */

/**< AEC */
#define ISI_AEC_AUTO                        0x00000001      /**< Camera AutoExposureControl on */
#define ISI_AEC_OFF                         0x00000002      /**< Camera AutoExposureControl off */

/**< DPCC */
#define ISI_DPCC_AUTO                       0x00000001      /**< Camera DefectPixelCorrection on */
#define ISI_DPCC_OFF                        0x00000002      /**< Camera DefectPixelCorrection off */

/**< AFPS */
#define ISI_AFPS_NOTSUPP                    0x00000000      /**< Auto FPS mode not supported; or ISI_RES_XXX bitmask of all resolutions being part of any AFPS series */



/**
 * @brief IsiRegModel class declaration.
 */
class IsiRegModel
  : public QAbstractTableModel
{
    Q_OBJECT

public:
    /**
     * @brief Standard constructor for the IsiRegModel object.
     *
     * @param   parent      Qt parent object.
     * @param   flags       Qt window flags.
     */
    IsiRegModel( const IsiRegDescription_t* regDescription = NULL, QObject *parent = 0 );

public:
    virtual int rowCount( const QModelIndex &parent ) const;
    virtual int columnCount( const QModelIndex &parent ) const;
    virtual QVariant data( const QModelIndex &index, int role ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role ) const;

private:
    const IsiRegDescription_t* m_regDescription;
};


/* @} module_name_api*/

#endif /*__ISIREGMODEL_H__*/
