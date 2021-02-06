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
 * @file registerwidget.h
 *
 * @brief
 *   Widget for register access.
 *
 *****************************************************************************/
#ifndef __REGISTER_WIDGET_H__
#define __REGISTER_WIDGET_H__


#include "VirtualCamera.h"
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>


//Todo: dummy, change to API
/******************************************************************************/
/**
 *          RegDescription_t
 *
 * @brief   Type for defining a register description
 *
 *****************************************************************************/
typedef struct RegDescription_s
{
    unsigned int        Address;            /**< adddress of the register */
#if 0
    Permissions_t   Permission;         /**< access mode to the register */
    RegName_t       Name;               /**< name string of the register */
    RegHint_t       Hint;               /**< hint string of the register */
    uint32_t        ResetValue;         /**< reset value of the register */
    uint32_t        UsedBits;           /**< bitmask of used bits */
    uint32_t        WriteAbleBits;      /**< bitmask of writeable bits */
#endif
} RegDescription_t;

/******************************************************************************/
/**
 *          CamerIcModuleId_t
 *
 * @brief   CamerIc Module Identifier
 *
 *****************************************************************************/
typedef enum CamerIcModuleId_e
{
    CAMERIC_MODULE_INVALID      =  0,   /* invalid module identifier (for range check) */
    CAMERIC_MODULE_MAIN_CONTROL =  1,   /* CamerIc Sub-Module: MAIN_CONTROL */
    CAMERIC_MODULE_IMG_EFFECTS  =  2,   /* CamerIc Sub-Module: IMAGE_EFFECTS */
    CAMERIC_MODULE_SUPER_IMPOSE =  3,   /* CamerIc Sub-Module: SUPER_IMPOSE */
    CAMERIC_MODULE_ISP_MAIN     =  4,   /* CamerIc Sub-Module: ISP_MAIN */
    CAMERIC_MODULE_ISP_FLASH    =  5,   /* CamerIc Sub-Module: ISP_FLASH */
    CAMERIC_MODULE_ISP_SHUTTER  =  6,   /* CamerIc Sub-Module: ISP_SHUTTER */
    CAMERIC_MODULE_CPROC        =  7,   /* CamerIc Sub-Module: COLOR_PROCESSING */
    CAMERIC_MODULE_MRSZ         =  8,   /* CamerIc Sub-Module: MAIN_PATH_RESIZER */
    CAMERIC_MODULE_SRSZ         =  9,   /* CamerIc Sub-Module: SELF_PATH_RESIZER */
    CAMERIC_MODULE_MI           = 10,   /* CamerIc Sub-Module: MEMORY_INTERFACE */
    CAMERIC_MODULE_JPE          = 11,   /* CamerIc Sub-Module: JPEG_ENCODER */
    CAMERIC_MODULE_SMIA         = 12,   /* CamerIc Sub-Module: SMIA */
    CAMERIC_MODULE_MIPI         = 13,   /* CamerIc Sub-Module: MIPI */
    CAMERIC_MODULE_MIPI2        = 14,   /* CamerIc Sub-Module: MIPI */
    CAMERIC_MODULE_ISP_AFM      = 15,   /* CamerIc Sub-Module: ISP_AUTOFOCUS_MEASUREMENT */
    CAMERIC_MODULE_ISP_BP       = 16,   /* CamerIc Sub-Module: ISP_BAD_PIXEL (normally replaced by DPCC) */
    CAMERIC_MODULE_ISP_LSC      = 17,   /* CamerIc Sub-Module: ISP_LENSE_SHADE_CORRECTION */
    CAMERIC_MODULE_ISP_IS       = 18,   /* CamerIc Sub-Module: ISP_IMAGE_STABILIZATION */
    CAMERIC_MODULE_ISP_HIST     = 19,   /* CamerIc Sub-Module: ISP_HISTOGRAM */
    CAMERIC_MODULE_ISP_FILTER   = 20,   /* CamerIc Sub-Module: ISP_FILTER */
    CAMERIC_MODULE_ISP_CAC      = 21,   /* CamerIc Sub-Module: ISP_CHROMATIC_ABERRATION_CORRECTION */
    CAMERIC_MODULE_ISP_CNR      = 22,   /* CamerIc Sub-Module: ISP_CHROMA_NOISE_REDUCTION */
    CAMERIC_MODULE_ISP_EXP      = 23,   /* CamerIc Sub-Module: ISP_EXPOSURE */
    CAMERIC_MODULE_ISP_BLS      = 24,   /* CamerIc Sub-Module: ISP_BLACK_LEVEL_SUBSTRACTION */
    CAMERIC_MODULE_ISP_DPF      = 25,   /* CamerIc Sub-Module: ISP_DENOISING_PRE_FFILTER */
    CAMERIC_MODULE_ISP_DPCC     = 26,   /* CamerIc Sub-Module: ISP_DEFECT_PIXEL_CLUSTER_CORRECTION */
    CAMERIC_MODULE_ISP_WDR      = 27,   /* CamerIc Sub-Module: ISP_WHITE_DYNAMIC_RANGE */
    CAMERIC_MODULE_ISP_AWB      = 28,   /* CamerIc Sub-Module: ISP_AUTO_WHITE_BALANCE_MEASUREMENT */
    CAMERIC_MODULE_ISP_VSM      = 29,   /* CamerIc Sub-Module: ISP_AUTO_VIDEO_STABILIZATION_MEASUREMENT */
    CAMERIC_MODULE_MAX                  /* max module identifier (for range check) */
} CamerIcModuleId_t;



//TODO: move to VirtualCamera_api
/******************************************************************************
 * HAL device base addresses
 *****************************************************************************/
// HalRegs:
#define HAL_BASEADDR_MARVIN    0x00000000 //!< Base address of MARVIN module.
#define HAL_BASEADDR_MARVIN_2  0x00010000 //!< Base address of MARVIN module of 2nd channel.
#define HAL_BASEADDR_MIPI_1    0x00000000 //!< Base address of MIPI module.
#define HAL_BASEADDR_MIPI_2    0x00010000 //!< Base address of MIPI module of 2nd channel.
#define HAL_BASEADDR_VDU       0x00008000 //!< Base address of VDU module.
#define HAL_BASEADDR_MARVIN_DEC 0x00600000 //!< Base address of DEC module.

// HalSysRegs:
#define HAL_BASEADDR_SYSCTRL   0x00000000 //!< HAL internal use only
#define HAL_BASEADDR_I2C_0     0x00001000 //!< HAL internal use only
#define HAL_BASEADDR_I2C_1     0x00001800 //!< HAL internal use only
#define HAL_BASEADDR_I2C_2     0x00000800 //!< HAL internal use only


typedef int RESULT;
#define RET_SUCCESS             0   //!< this has to be 0, if clauses rely on it


#include <QtGui>
#include <QTabWidget>
#include <QGroupBox>
#include <QtGui/QMovie>
#include <QtWidgets/QWidget>
#include <QtCore/QRect>
#include <QtGui/QImage>
#include <QtMultimedia/QAbstractVideoSurface>
#include <QtMultimedia/QVideoFrame>
#include <QVector>
#include <QList>

#include <QString>
#include <QAbstractItemModel>
#include <QtXml/QDomDocument>

#include <QVariant>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>

#include <QMainWindow>

class QAction;
class QLabel;
class QMenu;
class QTreeView;
class QAbstractButton;
class QAbstractVideoSurface;
class QSlider;
class VideoWidgetSurface;


class RegisterWidget : public QWidget
{
    Q_OBJECT

    public:
        RegisterWidget( VirtualCamera* camDevice, QWidget *parent, bool main_notsub );

    public slots:
        void mainControlButtonClicked();
        void superImposeButtonClicked();
        void imageEffectsButtonClicked();
        void ispMainButtonClicked();
        void ispFlashButtonClicked();
        void ispShutterButtonClicked();
        void ispCprogButtonClicked();
        void mainResizeButtonClicked();
        void selfResizeButtonClicked();
        void miButtonClicked();
        void jpegButtonClicked();
        void smiaButtonClicked();
        void mipiButtonClicked();
        void afmButtonClicked();
        void badPixButtonClicked();
        void lscButtonClicked();
        void imageStabButtonClicked();
        void ispHistButtonClicked();
        void ispFiltButtonClicked();
        void ipsCACButtonClicked();
        void expMeasureButtonClicked();
        void ispBlsButtonClicked();
        void ispDpfButtonClicked();
        void ispDpccButtonClicked();
        void ispWdrButtonClicked();
        void ispAwbButtonClicked();
        void readRegFileButtonClicked(QString fileName);
        void createRegFileButtonClicked(QString fileName);
        void replaceRegFileButtonClicked(QString fileName);
        void changeRegModuleFile(QString fileName);
        void insertRegModuleFile(QString fileName);
        void displayLogContent(QString fileName);
        void refreshRegFileButtonClicked();
        void setButtonClicked();
        void selectChanged(const QItemSelection & index1,const QItemSelection & index2);
        //void selectChanged(const QModelIndex & index);
        void slotHexTextChanged();
        void slotDecTextChanged();
        void slotBinTextChanged();
        void readRegFileOpen();
        void writeRegFileOpen();
        void logReplayButtonClicked();
        void logResetButtonClicked();
        void logBrowse();
        void logReplay();
        void paintFontAccordingPermission(RegDescription_t *RegistersMap,uint32_t i);
        void setFontAccordingPermission(RegDescription_t *RegistersMap,uint32_t i);
        QString getRegPermissionValue(RegDescription_t *RegistersMap,uint32_t i);
        void dumpButtonClicked();
        void dumpAllButtonClicked();

    private:
        VirtualCamera* m_camDevice;
        bool m_main_notsub;
        int m_addrOffset;
        void createMainSelectionTabsGroupBox();
        void createMainWindowDisplayGroupBox();
        void createSubWindowRegisterGroupBox();
        void createSubWindowButtonGroupBox();
        void createLogDisplayGroupBox();
        void refreshRegisterEditLogBox(QString textStringModuleName,QString textStringAddress , QString textStringValue);
        void createLogReplayHandleButtonGroupBox();
        void createSubLogReplayHandleButtonGroupBox();
        void saveLogInforToXmlFile();
        void logTimeinformation();

        void dumpModuleRegisterValuesHelper( FILE* pFile, CamerIcModuleId_t moduleId );
        void retrieveRegisterValuesAndDump( const CamerIcModuleId_t moduleId, QString &moduleName, QString &fileName );
        void retrieveAllRegisterValuesAndDump( QString &fileName );
        void retrieveRegisterValuesAndDisplay(const CamerIcModuleId_t moduleId);
        QAbstractItemModel *createRegisterModel(QObject *parent);
        void updateRegisterValues(const CamerIcModuleId_t moduleId);
        void updateRegister(const QString &registerCurrentValue,uint32_t i);
        void addRegister(QAbstractItemModel *model, const QString &accessMode,
                const QString &registerAddress, const QString &registerName,
                const QString &registerDefaultValue, const QString &registerCurrentValue);

        QGroupBox* MainSelectionTabsGroupBox;
        QGroupBox* MainWindowDisplayGroupBox;
        QGroupBox* SubWindowRegisterGroupBox;
        QGroupBox* SubWindowButtonGroupBox;
        QGroupBox* LogDisplayGroupBox;
        QGroupBox* LogReplayHandleButtonGroupBox;
        QGroupBox* SubLogReplayHandleButtonGroupBox;

        QTextEdit* registerValueEditLogBox;
        QTextEdit* logInformationDisplayBox;

        QStandardItemModel * model;

        QTreeView * registerView;

        uint16_t registerSelectedNo ;
        uint16_t row;
        uint32_t NumRegisters;
        QString lineHexEditText;
        QString lineDecEditText;
        QString lineBinEditText;
        QString linePreStringValue;
        QString saveLogXmlFileName;
        QString openLogXmlFileName;

        QLineEdit *lineEditRegisterAddress;
        QLineEdit *hexValue ;
        QLineEdit *decValue ;
        QLineEdit *binValue ;
        bool IsValueEdited;
        QString stringAddress ;
        QString stringRegHexValue ;
        QString stringRegDecValue ;
        QString stringRegBinValue ;
        RegDescription_t *RegistersMap;
        QString stringModuleName;
        QString xmlStringModuleName;

        QPushButton *setButton;
        QPushButton *popupButton;
        QPushButton *logReplayButton;
        QPushButton *logResetButton;
        QPushButton *logReplayConfirmButton;
        QPushButton *logBrowseButton;

        QTreeView *view;

        QErrorMessage *readRegFileErrorMessageDialog;
        QErrorMessage *hexTextErrorMessageDialog;
        QErrorMessage *decTextErrorMessageDialog;
        QErrorMessage *binTextErrorMessageDialog;

        QVector<QDomElement> *pCamericSubItem  /*must define here , the last one*/;
};

#endif /* __REGISTER_WIDGET_H__ */
