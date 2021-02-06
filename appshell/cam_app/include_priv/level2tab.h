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


#ifndef __LEVEL_2_TAB_H__
#define __LEVEL_2_TAB_H__

#include <QtGui>
#include <QTabWidget>
#include <QGroupBox>
#include <QtGui/QMovie>
#include <QtWidgets/QWidget>
#include <QtCore/QRect>
#include <QtGui/QImage>
#include <QtMultimedia/QAbstractVideoSurface>
#include <QtMultimedia/QVideoFrame>
#include <QDialog>
#include <QPushButton>
#include <QString>
#include <QAbstractItemModel>
#include <QtXml/QDomDocument>
#include <QVector>


#include <QVariant>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>

#include <QMainWindow>
#include "VirtualCamera.h"

#include "cac/cac-plot.h"
//#include "cac/caccanvaspicker.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLineEdit>

class QRadioButton;
class QGroupBox;
class QPushButton;
class WdrPlot;
class QMainWindow;
class CacPlot;

class EcTab : public QWidget
{
    Q_OBJECT

    public:
        EcTab(VirtualCamera *camDevice,  QWidget *parent = 0);

    public slots:
        void updateStatus();

        void refreshGain( float value = -1.0f );
        void refreshIntegrationTime( float value = -1.0f );
        void refreshExposure( float value = -1.0f );

        void updateGain( double value );
        void updateIntegrationTime( double value );
        void updateExposure( double value );

        void changeGain( int value );
        void changeIntegrationTime( int value );
        void changeExposure( int value );

    private:
        bool getGain( float &gain );
        bool getGainLimits( float &minGain, float &maxGain, float &stepTime );
        bool setGain( float newGain, float &setGain );

        bool getIntegrationTime( float &time );
        bool getIntegrationTimeLimits( float &minTime, float &maxTime, float &stepTime );
        bool setIntegrationTime( float newTime, float &setTime );

        bool getExposure( float &exp );
        bool getExposureLimits( float &minExp, float &maxExp, float &stepExp );
        bool setExposure( float newExp, float &setExp );

    private:
        VirtualCamera *m_camDevice;

        float m_minGain;
        float m_maxGain;
        float m_stepGain;

        float m_minItime;
        float m_maxItime;
        float m_stepItime;

        float m_minExp;
        float m_maxExp;
        float m_stepExp;

    private:
        QGroupBox *EcBlockGroupBox;

        QGroupBox *gainGroupBox;
        QGridLayout *gainGridLayout;
        QSlider *gainHorizontalSlider;
        QDoubleSpinBox *gainSpinBox;
        QLabel *gainMinText;
        QLabel *gainMaxText;

        QGroupBox *itimeGroupBox;
        QGridLayout *itimeGridLayout;
        QSlider *itimeHorizontalSlider;
        QDoubleSpinBox *itimeSpinBox;
        QLabel *itimeMinText;
        QLabel *itimeMaxText;

        QGroupBox *expGroupBox;
        QGridLayout *expGridLayout;
        QSlider *expHorizontalSlider;
        QDoubleSpinBox *expSpinBox;
        QLabel *expMinText;
        QLabel *expMaxText;

        QPushButton *EcRefreshButton;

    private:
        void createEcBlockGroupBox();
        void createGainGroupBox();
        void createItimeGroupBox();
        void createExpGroupBox();
};

class BlsTab : public QWidget
{
    Q_OBJECT

    public:
        BlsTab(VirtualCamera *camDevice, QWidget *parent = 0);

    public slots:
        void BlsSetValues();

        void BlsLoadFromFileButtonClicked();
        void BlsSaveToFileButtonClicked();

        void BlsMeasuringMeasureButtonClicked();
        void BlsMeasuringTakeOverValuesButtonClicked();

        void updateStatus();

    private:
        VirtualCamera *m_camDevice;
        
    
        QGroupBox *BlsBlockGroupBox;
        void createBlsBlockGroupBox();
        QErrorMessage *BlsConfigErrorMessageDialog;
        void createBlsSubtractionGroupBox();
        void createBlsMeasuringGroupBox();

        void createBlsWindowGroupBox();
        void createBlsResultsGroupBox();
        void createBlsMeasuringButtonGroupBox();

        void createBlsWindow1GroupBox();
        void createBlsWindow2GroupBox();

        QGroupBox *BlsSubtractionGroupBox;
        QGroupBox *BlsMeasuringGroupBox;

        QLabel *BlsValueRedLabel;
        QLabel *BlsValueGreenRLabel;
        QLabel *BlsValueGreenBLabel;
        QLabel *BlsValueBlueLabel;

        QSpinBox *BlsValueRedText;
        QSpinBox *BlsValueGreenRText;
        QSpinBox *BlsValueGreenBText;
        QSpinBox *BlsValueBlueText;

        QPushButton *BlsRefreshButton;
        QPushButton *BlsLoadFromFileButton;
        QPushButton *BlsSaveToFileButton;

        QGroupBox *BlsWindowGroupBox;
        QGroupBox *BlsResultsGroupBox;
        QGroupBox *BlsMeasuringButtonGroupBox;

        QGroupBox *BlsWindow1GroupBox;
        QGroupBox *BlsWindow2GroupBox;

        QLabel *BlsWindow1XLabel;
        QLabel *BlsWindow1YLabel;
        QLabel *BlsWindow1widthLabel;
        QLabel *BlsWindow1heightLabel;

        QLineEdit *BlsWindow1XText;
        QLineEdit *BlsWindow1YText;
        QLineEdit *BlsWindow1WidthText;
        QLineEdit *BlsWindow1HeightText;

        QLabel *BlsWindow2XLabel;
        QLabel *BlsWindow2YLabel;
        QLabel *BlsWindow2widthLabel;
        QLabel *BlsWindow2heightLabel;

        QLineEdit *BlsWindow2XText;
        QLineEdit *BlsWindow2YText;
        QLineEdit *BlsWindow2WidthText;
        QLineEdit *BlsWindow2HeightText;

        QLabel *BlsResultsValueALabel;
        QLabel *BlsResultsValueBLabel;
        QLabel *BlsResultsValueCLabel;
        QLabel *BlsResultsValueDLabel;

        QLineEdit *BlsResultsValueAText;
        QLineEdit *BlsResultsValueBText;
        QLineEdit *BlsResultsValueCText;
        QLineEdit *BlsResultsValueDText;

        QPushButton *BlsMeasuringMeasureButton;
        QPushButton *BlsMeasuringTakeOverValuesButton;
};




class WbTab : public QWidget
{
    Q_OBJECT

    public:
        WbTab(VirtualCamera *camDevice, QWidget *parent = 0);

    public slots:
        void WBSetGains();
        void WBSetCcMatrix();
        void WBSetCcOffset();

        void WBLoadFromFileButtonClicked();
        void WBSaveToFileButtonClicked();

        void updateStatus();

    private:
        VirtualCamera *m_camDevice;

        QGroupBox *WbBlockGroupBox;
        void createWbBlockGroupBox();
        QErrorMessage *WbConfigErrorMessageDialog;
        QGroupBox *WBGainsGroupBox;
        QGroupBox *XTalkMatrixGroupBox;

        void createWBGainsGroupBox();
        void createXTalkMatrixGroupBox();

        QLabel *WbRedLabel;
        QLabel *WbGreenRLabel;
        QLabel *WbGreenBLabel;
        QLabel *WbBlueLabel;

        QDoubleSpinBox *WbRedText;
        QDoubleSpinBox *WbGreenRText;
        QDoubleSpinBox *WbGreenBText;
        QDoubleSpinBox *WbBlueText;

        QLabel *XTalkRedLabel;
        QLabel *XTalkGreenLabel;
        QLabel *XTalkBlueLabel;

        QDoubleSpinBox *XTalkRed1Text;
        QDoubleSpinBox *XTalkRed2Text;
        QDoubleSpinBox *XTalkRed3Text;
        QDoubleSpinBox *XTalkGreen1Text;
        QDoubleSpinBox *XTalkGreen2Text;
        QDoubleSpinBox *XTalkGreen3Text;
        QDoubleSpinBox *XTalkBlue1Text;
        QDoubleSpinBox *XTalkBlue2Text;
        QDoubleSpinBox *XTalkBlue3Text;

        QLabel *XTalkOffsetLabel;

        QSpinBox *XTalkOffsetRText;
        QSpinBox *XTalkOffsetGText;
        QSpinBox *XTalkOffsetBText;

        QLabel *XTalkRedSpaceLabel;
        QLabel *XTalkGreenSpaceLabel;
        QLabel *XTalkBlueSpaceLabel;

        QPushButton *WBRefreshButton;
        QPushButton *WBLoadFromFileButton;
        QPushButton *WBSaveToFileButton;
};




class BpTab : public QWidget
{
    Q_OBJECT

    public:
        BpTab(QWidget *parent = 0);
};

class DegammaTab : public QWidget
{
    Q_OBJECT

    public:
        DegammaTab(QWidget *parent = 0);
};

class ColCorTab : public QWidget
{
    Q_OBJECT

    public:
        ColCorTab(QWidget *parent = 0);
};


#if 0
class WdrTab : public QWidget
{
    Q_OBJECT

    public slots:
        void loadButtonCurve();
        void editButtonCurve();
        void WdrEnableApply();
        void updateCurve0();
        void updateCurve1();
        void updateCurve2();
        void updateCurve3();
        void updateCurve4();
        void updateCurve5();
        void editButtonCurveOpen();
        void editButtonCurveSave();
        void editButtonCurveApply();
        void editButtonCurveCancel();
        void updateDY(QStandardItem * item);

    public:
    WdrTab(QWidget *parent = 0);

    private:
    int sumdY;
    int sumYm;
    bool bCurveApply_MaxYm;
    bool bCurveApply_dY;
    bool bCurveApply_Ym;
    bool bSetSumValue;
    bool bDefault;
    bool bDefaultCurve;
        bool bLoadingFromFile;
        QErrorMessage *curveApply_Ym_ErrorMessageDialog;
        QErrorMessage *curveApply_MaxYm_ErrorMessageDialog;
        QErrorMessage *curveApply_dY_ErrorMessageDialog;
        QErrorMessage *readCurveFileErrorMessageDialog;
        void readCurveInforFile(QString fileName);
        void saveCurveInforFile(QString fileName);
        void updateLoadXmlCurve();
        void checkWdrCurveValue(CamEngineWdrCurve_t wdrCurve);
        void LoadButtonCurveApply(CamEngineWdrCurve_t camDeviceWdrCurve);

        QGroupBox *WdrBlockGroupBox;
        QCheckBox *wdrEnableCheckBox;
        QGroupBox *wdrCurveGroupBox;
        CamEngineWdrCurve_t camDeviceWdrCurve;

        WdrPlot   *plot;
        bool bWdrCurveInit;
        QPointer<QDialog> editWDRCurvedialog;
        QTreeView * wdrParamView;
        QStandardItemModel * model;
        QErrorMessage *wdrConfigErrorMessageDialog;

        QDialog *buttonCurveOpenDialog;

        void createWdrBlockGroupBox();
        void createWdrGroupBox();
        void createWdrCurveGroupBox();
        void curveApply();
        QAbstractItemModel *createWDRParamModel(QObject *parent);
        QGroupBox       *CurveGroupBox;
        QRadioButton    *CurveRadioButton[8];
        QPushButton     *loadButton;
        QPushButton     *editButton;
        QRadioButton    *createRadioButton( const QString &text ,  const int buttonIndex);

        QVector<QDomElement> *pCamericCurveSubItem  /*must define here , the last one*/;
};
#endif


class DpfTab: public QWidget
{
    Q_OBJECT

    public:
        DpfTab(VirtualCamera *camDevice, QWidget *parent = 0);

    private:
        VirtualCamera *m_camDevice;

        QGroupBox *m_dpfGroupBox;
        QGroupBox *m_strengthGroupBox;
        QGroupBox *m_weightGroupBox;

        QCheckBox *m_enableCheckBox;
        QDoubleSpinBox *m_gradientSpinBox;
        QDoubleSpinBox *m_offsetSpinBox;
        QDoubleSpinBox *m_minSpinBox;
        QDoubleSpinBox *m_divSpinBox;
        QSpinBox *m_greenSpinBox;
        QSpinBox *m_redBlueSpinBox;

        void createDpfBlockGroupBox();
        QErrorMessage *dpfConfigErrorMessageDialog;



    public slots:
        void DpfEnableApply();
        void updateStatus();
        void configure();
};

class DpccTab : public QWidget
{
    Q_OBJECT

    public:
        DpccTab(VirtualCamera *camDevice, QWidget *parent = 0);

    private:
        VirtualCamera *m_camDevice;

        QGroupBox *DpccBlockGroupBox;
        //QGroupBox *DpccHandleGroupBox;
        QCheckBox *DpccEnableCheckBox;

        void createDpccBlockGroupBox();
        QErrorMessage *dpccConfigErrorMessageDialog;

    public slots:
        void DpccEnableApply();
        void updateStatus();
};

class LscTab : public QWidget
{
    Q_OBJECT

    public:
        LscTab(QWidget *parent = 0);
};

class CacTab : public QWidget
{
    Q_OBJECT

    public:
        CacTab(VirtualCamera *camDevice, QWidget *parent = 0);

    private:
        VirtualCamera *m_camDevice;

        QGroupBox *CacBlockGroupBox;
        QGroupBox *CacHandleGroupBox;
        QGroupBox *SubCacCurveGroupBox;
        QGroupBox *SubCacHandleGroupBox;
        QCheckBox *CacEnableCheckBox;

        void createCacBlockGroupBox();
        void createCacHandleGroupBox();
        void createSubCacHandleGroupBox();
        void createSubCacCurveGroupBox();

        QErrorMessage *CacConfigErrorMessageDialog;

        QVBoxLayout *AllBox;

        QHBoxLayout *hbox1;
        QHBoxLayout *hbox2;

        QGroupBox *Para1Box;
        QGridLayout *Para1Layout;
        QLabel *RedParaLabel;
        QLabel *BlueParaLabel;
        QLabel *LinearCoeffLabel;
        QLabel *SquareCoeffLabel;
        QLabel *CubicalCoeffLabel;

        QLineEdit *redLinearCoeffEdit;
        QLineEdit *redSquareCoeffEdit;
        QLineEdit *redCubicalCoeffEdit;
        QLineEdit *blueLinearCoeffEdit;
        QLineEdit *blueSquareCoeffEdit;
        QLineEdit *blueCubicalCoeffEdit;


        QGroupBox *Para2Box;
        QGridLayout *Para2Layout;
        QLabel *ratioParaLabel;
        QLabel *centerHoriOffsetParaLabel;
        QLabel *centerVertOffsetParaLabel;
        QLabel *pixelLabel;
        QLabel *linesLabel;

        QLineEdit *ratioParaEdit;
        QLineEdit *centerHoriOffsetEdit;
        QLineEdit *centerVertOffsetEdit;

        QGroupBox *Para3Box;
        QGridLayout *Para3Layout;
        QLabel *horiLimitParaLabel;
        QLabel *vertLimitParaLabel;
        QLabel *horiLimitParaEndLabel;
        QLabel *vertLimitParaEndLabel;
        QPushButton *plotButton;


        QLineEdit *horiLimitParaEdit;
        QLineEdit *vertLimitParaEdit;

        CacPlot   *plot;

    public slots:
        void CacEnableApply();


};



class DemosaicTab : public QWidget
{
    Q_OBJECT

    public:
        DemosaicTab(QWidget *parent = 0);
};

class FitterTab : public QWidget
{
    Q_OBJECT

    public:
        FitterTab(QWidget *parent = 0);
};

class GammaTab : public QWidget
{
    Q_OBJECT

    public:
        GammaTab(QWidget *parent = 0);
};

class CsmTab : public QWidget
{
    Q_OBJECT

    public:
        CsmTab(QWidget *parent = 0);
};


class Level2Tab : public QTabWidget
{
    Q_OBJECT

    public:
        Level2Tab( VirtualCamera *camDevice, QWidget *parent = 0 );
#if 0
        WdrTab *Level2TabGetWdrTab();
        WdrTab *wdrTabVar;
#endif

    signals:
        void UpdateCamEngineStatus();

    private:
        VirtualCamera *m_camDevice;
#if 0
        public slots:
            /*        void
             *        setFontAccordingPermission(RegDescription_t
             *        *RegistersMap,uint32_t
             *        i);*/
            /*        QString
             *        getRegPermissionValue(RegDescription_t
             *        *RegistersMap,uint32_t
             *        i);*/

            void MFBlsButtonClicked();
        void MFBpButtonClicked();
        void MFDegammaButtonClicked();
        void MFColCorButtonClicked();
        void MFWdrButtonClicked();
        void MFDpfButtonClicked();
        void MFDpccButtonClicked();
        void MFLSCButtonClicked();
        void MFCacButtonClicked();
        void MFDemosaicButtonClicked();
        void MFFitterButtonClicked();
        void MFGammaButtonClicked();
        void MFCsmButtonClicked();

        void createMFBlsBlockGroupBox();

    private:
        void createMainSelectionModuleGroupBox();
        void createMainWindowDisplayGroupBox();
        void createCurveDisplaySubGroupBox();
        void releaseButtonFlat();

        QGroupBox* MainSelectionModuleGroupBox;
        QGroupBox* MainWindowDisplayGroupBox;
        QGroupBox* CurveHandleButtonGroupBox;
        QGroupBox* CurveDisplayGroupBox;
        QGroupBox* MFBlsBlockGroupBox;
        QPoint imagePos(const QImage &image) const;

        QPushButton *popupButton;

        uint16_t registerSelectedNo;
        QString stringModuleName;
        QString xmlStringModuleName;

        QImage sourceImage;
        QImage destinationImage;
        QImage resultImage;
        QLabel *imageLabel;

        QPushButton *MFBlsButton;
        QPushButton *MFBpButton;
        QPushButton *MFDegammaButton;
        QPushButton *MFColCorButton;
        QPushButton *MFWdrButton;
        QPushButton *MFDpfButton;
        QPushButton *MFDpccButton;
        QPushButton *MFLSCButton;
        QPushButton *MFCacButton;
        QPushButton *MFDemosaicButton;
        QPushButton *MFFitterButton;
        QPushButton *MFGammaButton;
        QPushButton *MFCsmButton;
#endif
};

#endif /* __LEVEL_2_TAB_H__ */

