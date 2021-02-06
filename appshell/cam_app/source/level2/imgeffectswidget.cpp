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

#include "level2/imgeffectswidget.h"

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QMessageBox>
#include <string.h>


ImgEffectsWidget::ImgEffectsWidget( VirtualCamera *camDevice, QWidget *parent )
    : QWidget    ( parent ),
      m_camDevice( camDevice )
{
    createImgEffectsGroupBox();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget( m_imgeffectsGroupBox );
    mainLayout->addStretch( 1 );
    setLayout( mainLayout );

    connect( parent, SIGNAL(UpdateCamEngineStatus()), SLOT(updateStatus()) );

    m_enableCheckBox->setChecked( false );
    changeEffect( 0 );

    updateStatus();
}


void ImgEffectsWidget::createImgEffectsGroupBox()
{
    m_imgeffectsGroupBox = new QGroupBox( tr("Image Effects") );

    m_enableCheckBox = new QCheckBox( tr("Enable") );

    m_effectComboBox = new QComboBox();
    m_effectComboBox->addItem( tr("Black/White (Grayscale)"),
            QVariant::fromValue<uint32_t>( CAMERIC_IE_MODE_GRAYSCALE ) );
    m_effectComboBox->addItem( tr("Negative"),
            QVariant::fromValue<uint32_t>( CAMERIC_IE_MODE_NEGATIVE ) );
    m_effectComboBox->addItem( tr("Sepia"),
            QVariant::fromValue<uint32_t>( CAMERIC_IE_MODE_SEPIA ));
    m_effectComboBox->addItem( tr("Color Selection"),
            QVariant::fromValue<uint32_t>( CAMERIC_IE_MODE_COLOR ));
    m_effectComboBox->addItem( tr("Emboss"),
            QVariant::fromValue<uint32_t>( CAMERIC_IE_MODE_EMBOSS ));
    m_effectComboBox->addItem( tr("Sketch"),
            QVariant::fromValue<uint32_t>( CAMERIC_IE_MODE_SKETCH ));
    m_effectComboBox->addItem( tr("Sharpening"),
            QVariant::fromValue<uint32_t>( CAMERIC_IE_MODE_SHARPEN ));

    QFormLayout *effectLayout = new QFormLayout();
    effectLayout->addRow( m_enableCheckBox, m_effectComboBox );

    //connect actions
    connect( m_enableCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( toggleEnable( bool ) ) );
    connect( m_effectComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( changeEffect( int ) ) );

    createRangeGroupBox();
    createTintGroupBox();
    createColorSelectionGroupBox();
    createSharpeningGroupBox();
    createMatrixGroupBox();

    QVBoxLayout *layout = new QVBoxLayout;

    layout->addLayout( effectLayout );
    layout->addWidget( m_rangeGroupBox );
    layout->addWidget( m_tintGroupBox );
    layout->addWidget( m_colorSelectionGroupBox );
    layout->addWidget( m_sharpeningGroupBox );
    layout->addWidget( m_matrixGroupBox );

    m_imgeffectsGroupBox->setLayout( layout );
}


void ImgEffectsWidget::createRangeGroupBox()
{
    m_rangeGroupBox = new QGroupBox( tr("Chrominance clipping range") );
    QVBoxLayout *rangeLayout = new QVBoxLayout();

    m_rangeComboBox = new QComboBox();
    m_rangeComboBox->addItem( tr("BT.601 (16..240)") ,
            QVariant::fromValue<uint32_t>( CAMERIC_IE_RANGE_BT601 ) );
    m_rangeComboBox->addItem( tr("FUll Range (0..255)"),
            QVariant::fromValue<uint32_t>( CAMERIC_IE_RANGE_FULL_RANGE ));

    connect( m_rangeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( changeRange() ) );

    rangeLayout->addWidget( m_rangeComboBox );

    m_rangeGroupBox->setLayout( rangeLayout );
}


void ImgEffectsWidget::createTintGroupBox()
{
    m_tintGroupBox = new QGroupBox( tr("Tint values") );
    QVBoxLayout *tintLayout = new QVBoxLayout();

    m_tintCbSpinBox = new QSpinBox();
    m_tintCbSpinBox->setMinimumSize(QSize(120, 22));
    m_tintCbSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_tintCbSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    m_tintCbSpinBox->setRange( 0, 255 );
    m_tintCbSpinBox->setSingleStep(1);

    m_tintCbSlider = new QSlider();
    m_tintCbSlider->setOrientation(Qt::Horizontal);
    m_tintCbSlider->setTracking( false );
    m_tintCbSlider->setRange( 0, 255 );

    QGridLayout *tintCbLayout = new QGridLayout();
    tintCbLayout->addWidget( m_tintCbSlider, 0, 0, 1, 2 );
    tintCbLayout->addWidget( m_tintCbSpinBox, 0, 2, 1, 1 );

    m_tintCrSpinBox = new QSpinBox();
    m_tintCrSpinBox->setMinimumSize(QSize(120, 22));
    m_tintCrSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_tintCrSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    m_tintCrSpinBox->setRange( 0, 255 );
    m_tintCrSpinBox->setSingleStep(1);

    m_tintCrSlider = new QSlider();
    m_tintCrSlider->setOrientation(Qt::Horizontal);
    m_tintCrSlider->setTracking( false );
    m_tintCrSlider->setRange( 0, 255 );

    QGridLayout *tintCrLayout = new QGridLayout();
    tintCrLayout->addWidget( m_tintCrSlider, 0, 0, 1, 2 );
    tintCrLayout->addWidget( m_tintCrSpinBox, 0, 2, 1, 1 );

    connect( m_tintCbSlider , SIGNAL( valueChanged( int ) ), m_tintCbSpinBox , SLOT( setValue( int ) ) );
    connect( m_tintCbSpinBox, SIGNAL( valueChanged( int ) ), m_tintCbSlider  , SLOT( setValue( int ) ) );
    connect( m_tintCbSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( setTintCb( int ) ) );

    connect( m_tintCrSlider , SIGNAL( valueChanged( int ) ), m_tintCrSpinBox, SLOT( setValue( int ) ) );
    connect( m_tintCrSpinBox, SIGNAL( valueChanged( int ) ), m_tintCrSlider , SLOT( setValue( int ) ) );
    connect( m_tintCrSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( setTintCr( int ) ) );

    QFormLayout *formLayout = new QFormLayout();

    formLayout->addRow( tr("Cb"), tintCbLayout );
    formLayout->addRow( tr("Cr"), tintCrLayout );

    m_tintGroupBox->setLayout( formLayout );
}


void ImgEffectsWidget::createColorSelectionGroupBox()
{
    m_colorSelectionGroupBox = new QGroupBox( tr("Color Selection") );

    m_redCheckBox   = new QCheckBox( tr("Red") );
    m_redCheckBox->setMinimumSize(QSize(120, 22));
    m_greenCheckBox = new QCheckBox( tr("Green") );
    m_greenCheckBox->setMinimumSize(QSize(120, 22));
    m_blueCheckBox  = new QCheckBox( tr("Blue") );
    m_blueCheckBox->setMinimumSize(QSize(120, 22));

    QButtonGroup *colorButtonGroup = new QButtonGroup();
    colorButtonGroup->setExclusive( false );
    colorButtonGroup->addButton( m_redCheckBox );
    colorButtonGroup->addButton( m_greenCheckBox );
    colorButtonGroup->addButton( m_blueCheckBox );

    QHBoxLayout *colorLayout = new QHBoxLayout;
    colorLayout->addWidget( m_redCheckBox );
    colorLayout->addWidget( m_greenCheckBox );
    colorLayout->addWidget( m_blueCheckBox );
    colorLayout->addStretch( 1 );

    m_colorThresholdSpinBox = new QSpinBox();
    m_colorThresholdSpinBox->setMinimumSize(QSize(120, 22));
    m_colorThresholdSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_colorThresholdSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    m_colorThresholdSpinBox->setRange( 0, 255);
    m_colorThresholdSpinBox->setSingleStep(1);

    m_colorThresholdSlider = new QSlider();
    m_colorThresholdSlider->setOrientation(Qt::Horizontal);
    m_colorThresholdSlider->setTracking( false );
    m_colorThresholdSlider->setRange( 0, 255);

    QGridLayout *thresholdLayout = new QGridLayout();
    thresholdLayout->addWidget( m_colorThresholdSlider, 0, 0, 1, 2 );
    thresholdLayout->addWidget( m_colorThresholdSpinBox, 0, 2, 1, 1 );

    connect( m_redCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( setColor() ) );
    connect( m_greenCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( setColor() ) );
    connect( m_blueCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( setColor() ) );

    connect( m_colorThresholdSlider , SIGNAL( valueChanged( int ) ), m_colorThresholdSpinBox, SLOT( setValue( int ) ) );
    connect( m_colorThresholdSpinBox, SIGNAL( valueChanged( int ) ), m_colorThresholdSlider , SLOT( setValue( int ) ) );
    connect( m_colorThresholdSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( setColorThreshold( int ) ) );

    QFormLayout *formLayout = new QFormLayout();

    formLayout->addRow( tr("Color"), colorLayout );
    formLayout->addRow( tr("Threshold"), thresholdLayout );

    m_colorSelectionGroupBox->setLayout( formLayout );
}


void ImgEffectsWidget::createSharpeningGroupBox()
{
    m_sharpeningGroupBox = new QGroupBox( tr("Sharpening") );

    m_sharpenFactorSpinBox = new QDoubleSpinBox();
    m_sharpenFactorSpinBox->setMinimumSize(QSize(120, 22));
    m_sharpenFactorSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_sharpenFactorSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    m_sharpenFactorSpinBox->setDecimals( 2 );
    m_sharpenFactorSpinBox->setRange( 0, 15.75 );
    m_sharpenFactorSpinBox->setSingleStep( 0.25 );

    m_sharpenFactorSlider = new QSlider();
    m_sharpenFactorSlider->setOrientation(Qt::Horizontal);
    m_sharpenFactorSlider->setTracking( false );
    m_sharpenFactorSlider->setRange( 0, 63 );
    
    m_sharpenFactorSpinBox->setValue( 4 );
    m_sharpenFactorSlider->setValue( 4 );

    QGridLayout *factorLayout = new QGridLayout();
    factorLayout->addWidget( m_sharpenFactorSlider, 0, 0, 1, 2 );
    factorLayout->addWidget( m_sharpenFactorSpinBox, 0, 2, 1, 1 );

    m_sharpenThresholdSpinBox = new QSpinBox();
    m_sharpenThresholdSpinBox->setMinimumSize(QSize(120, 22));
    m_sharpenThresholdSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_sharpenThresholdSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    m_sharpenThresholdSpinBox->setRange( 0, 255 );
    m_sharpenThresholdSpinBox->setSingleStep( 1 );

    m_sharpenThresholdSlider = new QSlider();
    m_sharpenThresholdSlider->setOrientation(Qt::Horizontal);
    m_sharpenThresholdSlider->setTracking( false );
    m_sharpenThresholdSlider->setRange( 0, 255 );
    
    m_sharpenThresholdSpinBox->setValue( 8 );
    m_sharpenThresholdSlider->setValue( 8 );

    QGridLayout *thresholdLayout = new QGridLayout();
    thresholdLayout->addWidget( m_sharpenThresholdSlider, 0, 0, 1, 2 );
    thresholdLayout->addWidget( m_sharpenThresholdSpinBox, 0, 2, 1, 1 );

    connect( m_sharpenFactorSpinBox,   SIGNAL( valueChanged( double ) ), this, SLOT( setSharpen( double ) ) );
    connect( m_sharpenFactorSlider,    SIGNAL( valueChanged( int    ) ), this, SLOT( changeSharpening( int ) ) );

    connect( m_sharpenThresholdSlider , SIGNAL( valueChanged( int ) ), m_sharpenThresholdSpinBox, SLOT( setValue( int ) ) );
    connect( m_sharpenThresholdSpinBox, SIGNAL( valueChanged( int ) ), m_sharpenThresholdSlider , SLOT( setValue( int ) ) );
    connect( m_sharpenThresholdSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( setSharpenThreshold( int ) ) );

    QFormLayout *formLayout = new QFormLayout();

    formLayout->addRow( tr("Factor"), factorLayout );
    formLayout->addRow( tr("Threshold"), thresholdLayout );

    m_sharpeningGroupBox->setLayout( formLayout );
}


void ImgEffectsWidget::createMatrixGroupBox()
{
    m_matrixGroupBox = new QGroupBox();

    for ( int i = 0; i < 9; ++i )
    {
        m_matrixComboBox[i] = new QComboBox( this );
        m_matrixComboBox[i]->setMinimumSize(QSize(120, 22));
        m_matrixComboBox[i]->addItem("-8", QVariant::fromValue<int32_t>( -8L ) );
        m_matrixComboBox[i]->addItem("-4", QVariant::fromValue<int32_t>( -4L ));
        m_matrixComboBox[i]->addItem("-2", QVariant::fromValue<int32_t>( -2L ));
        m_matrixComboBox[i]->addItem("-1", QVariant::fromValue<int32_t>( -1L ));
        m_matrixComboBox[i]->addItem("0", QVariant::fromValue<int32_t>( 0L ));
        m_matrixComboBox[i]->addItem("1", QVariant::fromValue<int32_t>( 1L ));
        m_matrixComboBox[i]->addItem("2", QVariant::fromValue<int32_t>( 2L ));
        m_matrixComboBox[i]->addItem("4", QVariant::fromValue<int32_t>( 4L ));
        m_matrixComboBox[i]->addItem("8", QVariant::fromValue<int32_t>( 8L ));

        connect( m_matrixComboBox[i], SIGNAL( currentIndexChanged( int ) ), this, SLOT( setMatrix() ) );
    }

    QGridLayout *matrixLayout = new QGridLayout();
    matrixLayout->setColumnStretch( 3, 1 );
    matrixLayout->addWidget(m_matrixComboBox[0] , 0, 0);
    matrixLayout->addWidget(m_matrixComboBox[1] , 0, 1);
    matrixLayout->addWidget(m_matrixComboBox[2] , 0, 2);
    matrixLayout->addWidget(m_matrixComboBox[3] , 1, 0);
    matrixLayout->addWidget(m_matrixComboBox[4] , 1, 1);
    matrixLayout->addWidget(m_matrixComboBox[5] , 1, 2);
    matrixLayout->addWidget(m_matrixComboBox[6] , 2, 0);
    matrixLayout->addWidget(m_matrixComboBox[7] , 2, 1);
    matrixLayout->addWidget(m_matrixComboBox[8] , 2, 2);

    m_matrixGroupBox->setLayout( matrixLayout );
}


void ImgEffectsWidget::toggleEnable( bool checked )
{
    if ( NULL == m_camDevice )
    {
        return;
    }

    if ( true == checked )
    {
        CamerIcIeConfig_t config;
        memset( &config, 0, sizeof( CamerIcIeConfig_t ) );

        config.mode = (CamerIcIeMode_t)m_effectComboBox->itemData(
                m_effectComboBox->currentIndex() ).toUInt();
        config.range = (CamerIcIeRange_t)m_rangeComboBox->itemData(
                m_rangeComboBox->currentIndex() ).toUInt();

        switch ( config.mode )
        {
            case CAMERIC_IE_MODE_GRAYSCALE:
            case CAMERIC_IE_MODE_NEGATIVE:
            {
                break;
            }
            case CAMERIC_IE_MODE_SEPIA:
            {
                config.ModeConfig.Sepia.TintCb = (uint8_t)m_tintCbSpinBox->value();
                config.ModeConfig.Sepia.TintCr = (uint8_t)m_tintCrSpinBox->value();
                break;
            }
            case CAMERIC_IE_MODE_COLOR:
            {
                bool red = m_redCheckBox->isChecked();
                bool green = m_greenCheckBox->isChecked();
                bool blue = m_blueCheckBox->isChecked();

                config.ModeConfig.ColorSelection.col_selection = CAMERIC_IE_COLOR_SELECTION_INVALID;
                if ( red )
                {
                    if ( green )
                    {
                        config.ModeConfig.ColorSelection.col_selection = blue ? CAMERIC_IE_COLOR_SELECTION_RGB :
                            CAMERIC_IE_COLOR_SELECTION_RG;
                    }
                    else
                    {
                        config.ModeConfig.ColorSelection.col_selection = blue ? CAMERIC_IE_COLOR_SELECTION_RB :
                            CAMERIC_IE_COLOR_SELECTION_R;
                    }
                }
                else if ( green )
                {
                    config.ModeConfig.ColorSelection.col_selection = blue ? CAMERIC_IE_COLOR_SELECTION_GB :
                        CAMERIC_IE_COLOR_SELECTION_G;
                }
                else if ( blue )
                {
                    config.ModeConfig.ColorSelection.col_selection = CAMERIC_IE_COLOR_SELECTION_B;
                }
                else
                {
                    QMessageBox::information( this, tr("Image Effects"),
                                             tr("Please select a color!") );
                	m_enableCheckBox->setChecked( false );
                    return;
                }

                config.ModeConfig.ColorSelection.col_threshold = (uint8_t)m_colorThresholdSpinBox->value();
                break;
            }
            case CAMERIC_IE_MODE_EMBOSS:
            {
                config.ModeConfig.Emboss.coeff[0] = (int8_t)m_matrixComboBox[0]->itemData ( m_matrixComboBox[0]->currentIndex() ).toInt();
                config.ModeConfig.Emboss.coeff[1] = (int8_t)m_matrixComboBox[1]->itemData ( m_matrixComboBox[1]->currentIndex() ).toInt();
                config.ModeConfig.Emboss.coeff[2] = (int8_t)m_matrixComboBox[2]->itemData ( m_matrixComboBox[2]->currentIndex() ).toInt();
                config.ModeConfig.Emboss.coeff[3] = (int8_t)m_matrixComboBox[3]->itemData ( m_matrixComboBox[3]->currentIndex() ).toInt();
                config.ModeConfig.Emboss.coeff[4] = (int8_t)m_matrixComboBox[4]->itemData ( m_matrixComboBox[4]->currentIndex() ).toInt();
                config.ModeConfig.Emboss.coeff[5] = (int8_t)m_matrixComboBox[5]->itemData ( m_matrixComboBox[5]->currentIndex() ).toInt();
                config.ModeConfig.Emboss.coeff[6] = (int8_t)m_matrixComboBox[6]->itemData ( m_matrixComboBox[6]->currentIndex() ).toInt();
                config.ModeConfig.Emboss.coeff[7] = (int8_t)m_matrixComboBox[7]->itemData ( m_matrixComboBox[7]->currentIndex() ).toInt();
                config.ModeConfig.Emboss.coeff[8] = (int8_t)m_matrixComboBox[8]->itemData ( m_matrixComboBox[8]->currentIndex() ).toInt();
                break;
            }
            case CAMERIC_IE_MODE_SKETCH:
            {
                config.ModeConfig.Sketch.coeff[0] = (int8_t)m_matrixComboBox[0]->itemData ( m_matrixComboBox[0]->currentIndex() ).toInt();
                config.ModeConfig.Sketch.coeff[1] = (int8_t)m_matrixComboBox[1]->itemData ( m_matrixComboBox[1]->currentIndex() ).toInt();
                config.ModeConfig.Sketch.coeff[2] = (int8_t)m_matrixComboBox[2]->itemData ( m_matrixComboBox[2]->currentIndex() ).toInt();
                config.ModeConfig.Sketch.coeff[3] = (int8_t)m_matrixComboBox[3]->itemData ( m_matrixComboBox[3]->currentIndex() ).toInt();
                config.ModeConfig.Sketch.coeff[4] = (int8_t)m_matrixComboBox[4]->itemData ( m_matrixComboBox[4]->currentIndex() ).toInt();
                config.ModeConfig.Sketch.coeff[5] = (int8_t)m_matrixComboBox[5]->itemData ( m_matrixComboBox[5]->currentIndex() ).toInt();
                config.ModeConfig.Sketch.coeff[6] = (int8_t)m_matrixComboBox[6]->itemData ( m_matrixComboBox[6]->currentIndex() ).toInt();
                config.ModeConfig.Sketch.coeff[7] = (int8_t)m_matrixComboBox[7]->itemData ( m_matrixComboBox[7]->currentIndex() ).toInt();
                config.ModeConfig.Sketch.coeff[8] = (int8_t)m_matrixComboBox[8]->itemData ( m_matrixComboBox[8]->currentIndex() ).toInt();
                break;
            }
            case CAMERIC_IE_MODE_SHARPEN:
            {
                int idx = (int)( (float)m_sharpenFactorSpinBox->value() / (float)m_sharpenFactorSpinBox->singleStep() );
                config.ModeConfig.Sharpen.factor    = (uint8_t)idx;
                config.ModeConfig.Sharpen.threshold = (uint8_t)m_sharpenThresholdSpinBox->value();

                config.ModeConfig.Sharpen.coeff[0] = (int8_t)m_matrixComboBox[0]->itemData ( m_matrixComboBox[0]->currentIndex() ).toInt();
                config.ModeConfig.Sharpen.coeff[1] = (int8_t)m_matrixComboBox[1]->itemData ( m_matrixComboBox[1]->currentIndex() ).toInt();
                config.ModeConfig.Sharpen.coeff[2] = (int8_t)m_matrixComboBox[2]->itemData ( m_matrixComboBox[2]->currentIndex() ).toInt();
                config.ModeConfig.Sharpen.coeff[3] = (int8_t)m_matrixComboBox[3]->itemData ( m_matrixComboBox[3]->currentIndex() ).toInt();
                config.ModeConfig.Sharpen.coeff[4] = (int8_t)m_matrixComboBox[4]->itemData ( m_matrixComboBox[4]->currentIndex() ).toInt();
                config.ModeConfig.Sharpen.coeff[5] = (int8_t)m_matrixComboBox[5]->itemData ( m_matrixComboBox[5]->currentIndex() ).toInt();
                config.ModeConfig.Sharpen.coeff[6] = (int8_t)m_matrixComboBox[6]->itemData ( m_matrixComboBox[6]->currentIndex() ).toInt();
                config.ModeConfig.Sharpen.coeff[7] = (int8_t)m_matrixComboBox[7]->itemData ( m_matrixComboBox[7]->currentIndex() ).toInt();
                config.ModeConfig.Sharpen.coeff[8] = (int8_t)m_matrixComboBox[8]->itemData ( m_matrixComboBox[8]->currentIndex() ).toInt();
                break;
            }
            default:
            {
                m_enableCheckBox->setChecked( false );
                return;
            }
        }

        //m_camDevice->imgEffectsEnable( &config );

        m_tintGroupBox->setEnabled( true );
        m_colorSelectionGroupBox->setEnabled( true );
        m_sharpeningGroupBox->setEnabled( true );
        m_matrixGroupBox->setEnabled( false );
    }
    else
    {
        //m_camDevice->imgEffectsDisable();

        m_tintGroupBox->setEnabled( true );
        m_colorSelectionGroupBox->setEnabled( true );
        m_sharpeningGroupBox->setEnabled( true );
        m_matrixGroupBox->setEnabled( true );
    }
}


void ImgEffectsWidget::changeEffect( int index )
{
    if ( true == m_enableCheckBox->isChecked() )
    {
        //disable
        toggleEnable( false );
    }

    switch ( (CamerIcIeMode_t)m_effectComboBox->itemData( index ).toUInt() )
    {
        case CAMERIC_IE_MODE_GRAYSCALE:
        {
            m_tintGroupBox->setVisible( false );
            m_colorSelectionGroupBox->setVisible( false );
            m_sharpeningGroupBox->setVisible( false );
            m_matrixGroupBox->setVisible( false );
            break;
        }
        case CAMERIC_IE_MODE_NEGATIVE:
        {
            m_tintGroupBox->setVisible( false );
            m_colorSelectionGroupBox->setVisible( false );
            m_sharpeningGroupBox->setVisible( false );
            m_matrixGroupBox->setVisible( false );
            break;
        }
        case CAMERIC_IE_MODE_SEPIA:
        {
            m_tintGroupBox->setVisible( true );
            m_colorSelectionGroupBox->setVisible( false );

            m_tintCbSpinBox->setValue( 12 );
            m_tintCrSpinBox->setValue( 136 );

            m_sharpeningGroupBox->setVisible( false );
            m_matrixGroupBox->setVisible( false );
            break;
        }
        case CAMERIC_IE_MODE_COLOR:
        {
            m_tintGroupBox->setVisible( false );
            m_colorSelectionGroupBox->setVisible( true );
            m_sharpeningGroupBox->setVisible( false );
            m_matrixGroupBox->setVisible( false );
            break;
        }
        case CAMERIC_IE_MODE_EMBOSS:
        {
            m_tintGroupBox->setVisible( false );
            m_colorSelectionGroupBox->setVisible( false );
            m_sharpeningGroupBox->setVisible( false );
            
            m_matrixComboBox[0]->setCurrentIndex( 6 );  //  2
            m_matrixComboBox[1]->setCurrentIndex( 5 );  //  1
            m_matrixComboBox[2]->setCurrentIndex( 4 );  //  0
            
            m_matrixComboBox[3]->setCurrentIndex( 5 );  //  1
            m_matrixComboBox[4]->setCurrentIndex( 4 );  //  0
            m_matrixComboBox[5]->setCurrentIndex( 3 );  // -1
            
            m_matrixComboBox[6]->setCurrentIndex( 4 );  //  0
            m_matrixComboBox[7]->setCurrentIndex( 3 );  // -1
            m_matrixComboBox[8]->setCurrentIndex( 2 );  // -2

            m_matrixGroupBox->setTitle( tr( "Laplace Filter" ) );
            m_matrixGroupBox->setVisible( true );

            break;
        }
        case CAMERIC_IE_MODE_SKETCH:
        {
            m_tintGroupBox->setVisible( false );
            m_colorSelectionGroupBox->setVisible( false );
            m_sharpeningGroupBox->setVisible( false );
            
            m_matrixComboBox[0]->setCurrentIndex( 3 );  // -1 
            m_matrixComboBox[1]->setCurrentIndex( 3 );  // -1
            m_matrixComboBox[2]->setCurrentIndex( 3 );  // -1
            
            m_matrixComboBox[3]->setCurrentIndex( 3 );  // -1
            m_matrixComboBox[4]->setCurrentIndex( 8 );  //  8
            m_matrixComboBox[5]->setCurrentIndex( 3 );  // -1
            
            m_matrixComboBox[6]->setCurrentIndex( 3 );  // -1
            m_matrixComboBox[7]->setCurrentIndex( 3 );  // -1
            m_matrixComboBox[8]->setCurrentIndex( 3 );  // -1

            m_matrixGroupBox->setTitle( tr( "Laplace Filter" ) );
            m_matrixGroupBox->setVisible( true );
            break;
        }
        case CAMERIC_IE_MODE_SHARPEN:
        {

            m_tintGroupBox->setVisible( false );
            m_colorSelectionGroupBox->setVisible( false );
            m_sharpeningGroupBox->setVisible( true );

            m_matrixComboBox[0]->setCurrentIndex( 3 );  // -1 
            m_matrixComboBox[1]->setCurrentIndex( 3 );  // -1
            m_matrixComboBox[2]->setCurrentIndex( 3 );  // -1
            
            m_matrixComboBox[3]->setCurrentIndex( 3 );  // -1
            m_matrixComboBox[4]->setCurrentIndex( 8 );  //  8
            m_matrixComboBox[5]->setCurrentIndex( 3 );  // -1
            
            m_matrixComboBox[6]->setCurrentIndex( 3 );  // -1
            m_matrixComboBox[7]->setCurrentIndex( 3 );  // -1
            m_matrixComboBox[8]->setCurrentIndex( 3 );  // -1

            m_matrixGroupBox->setTitle( tr( "Convolution Matrix" ) );
            m_matrixGroupBox->setVisible( true );
            break;
        }
        default:
        {
            break;
        }
    }

    if ( true == m_enableCheckBox->isChecked() )
    {
        //enable
        toggleEnable( true );
    }
}


void ImgEffectsWidget::changeRange()
{
    if ( true == m_enableCheckBox->isChecked() )
    {
        //disable
        toggleEnable( false );
        //enable
        toggleEnable( true );
    }

    //setRange of spin boxes
}


void ImgEffectsWidget::setTintCb( int value )
{
    if ( NULL == m_camDevice )
    {
        return;
    }

    m_tintCbSlider->blockSignals( true );
    m_tintCbSpinBox->blockSignals( true );

    if ( true == m_enableCheckBox->isChecked() )
    {
    	//m_camDevice->imgEffectsSetTintCb( (uint8_t)value );
    }

    m_tintCbSlider->setValue( value );
    m_tintCbSpinBox->setValue( value );

    m_tintCbSlider->blockSignals( false );
    m_tintCbSpinBox->blockSignals( false );
}


void ImgEffectsWidget::setTintCr( int value )
{
    if ( NULL == m_camDevice )
    {
        return;
    }

    m_tintCrSlider->blockSignals( true );
    m_tintCrSpinBox->blockSignals( true );

    if ( true == m_enableCheckBox->isChecked() )
    {
    	//m_camDevice->imgEffectsSetTintCr( (uint8_t)value );
    }

    m_tintCrSlider->setValue( value );
    m_tintCrSpinBox->setValue( value );

    m_tintCrSlider->blockSignals( false );
    m_tintCrSpinBox->blockSignals( false );
}


void ImgEffectsWidget::setColor()
{
    if ( NULL == m_camDevice )
    {
        return;
    }

    bool red = m_redCheckBox->isChecked();
    bool green = m_greenCheckBox->isChecked();
    bool blue = m_blueCheckBox->isChecked();

    CamerIcIeColorSelection_t color = CAMERIC_IE_COLOR_SELECTION_INVALID;
    if ( red )
    {
        if ( green )
        {
            color = blue ? CAMERIC_IE_COLOR_SELECTION_RGB :
                CAMERIC_IE_COLOR_SELECTION_RG;
        }
        else
        {
            color = blue ? CAMERIC_IE_COLOR_SELECTION_RB :
                CAMERIC_IE_COLOR_SELECTION_R;
        }
    }
    else if ( green )
    {
        color = blue ? CAMERIC_IE_COLOR_SELECTION_GB :
            CAMERIC_IE_COLOR_SELECTION_G;
    }
    else if ( blue )
    {
        color = CAMERIC_IE_COLOR_SELECTION_B;
    }
    else
    {
        return;
    }

    uint8_t threshold = (uint8_t)m_colorThresholdSpinBox->value();

    if ( true == m_enableCheckBox->isChecked() )
    {
    	//m_camDevice->imgEffectsSetColorSelection( color, threshold );
    }
}


void ImgEffectsWidget::setColorThreshold( int value )
{
    if ( NULL == m_camDevice )
    {
        return;
    }

    m_colorThresholdSlider->blockSignals( true );
    m_colorThresholdSpinBox->blockSignals( true );

    m_colorThresholdSlider->setValue( value );
    m_colorThresholdSpinBox->setValue( value );

    setColor();

    m_colorThresholdSlider->blockSignals( false );
    m_colorThresholdSpinBox->blockSignals( false );
}


void ImgEffectsWidget::setMatrix()
{
    //FIXME
}


void ImgEffectsWidget::setSharpen( double value )
{
    if ( NULL == m_camDevice )
    {
        return;
    }
#if 0
    m_sharpenFactorSlider->blockSignals( true );
    m_sharpenFactorSpinBox->blockSignals( true );

    int idx = (int)( (float)value / (float)m_sharpenFactorSpinBox->singleStep() );
    uint8_t factor    = (uint8_t)idx;
    uint8_t threshold = (uint8_t)m_sharpenThresholdSpinBox->value();

    if ( true == m_enableCheckBox->isChecked() )
    {
    	//m_camDevice->imgEffectsSetSharpen( factor, threshold );
    }

    m_sharpenFactorSlider->setValue( idx );
    m_sharpenFactorSpinBox->setValue( value );

    m_sharpenFactorSlider->blockSignals( false );
    m_sharpenFactorSpinBox->blockSignals( false );
#endif
}


void ImgEffectsWidget::changeSharpening( int value )
{
    double newFactor = m_sharpenFactorSpinBox->minimum() + (double)value * m_sharpenFactorSpinBox->singleStep();
    setSharpen( newFactor );
}


void ImgEffectsWidget::setSharpenThreshold( int value )
{
    if ( NULL == m_camDevice )
    {
        return;
    }
#if 0
    m_sharpenThresholdSlider->blockSignals( true );
    m_sharpenThresholdSpinBox->blockSignals( true );

    int idx = (int)( (float)m_sharpenFactorSpinBox->value() / (float)m_sharpenFactorSpinBox->singleStep() );
    uint8_t factor      = (uint8_t)idx;
    uint8_t threshold   = (uint8_t)value;

    if ( true == m_enableCheckBox->isChecked() )
    {
    	m_camDevice->imgEffectsSetSharpen( factor, threshold );
    }

    m_sharpenThresholdSlider->setValue( value );
    m_sharpenThresholdSpinBox->setValue( value );

    m_sharpenThresholdSlider->blockSignals( false );
    m_sharpenThresholdSpinBox->blockSignals( false );
#endif
}


void ImgEffectsWidget::updateStatus()
{
#if 0
    if ( CamEngineItf::State::Invalid == m_camDevice->state() )
    {
        setEnabled( false );
        return;
    }
#endif
    setEnabled( true );

    bool running = false;
    //CamEngineCprocConfig_t config;
    //memset( &config, 0, sizeof( CamEngineCprocConfig_t ) );

    CamerIcIeMode_t   mode  = CAMERIC_IE_MODE_GRAYSCALE;
    CamerIcIeRange_t  range = CAMERIC_IE_RANGE_BT601;

    //m_camDevice->cProcStatus( running, config );

    m_enableCheckBox->blockSignals( true );
    m_effectComboBox->blockSignals( true );
    m_rangeComboBox->blockSignals( true );

    m_enableCheckBox->setChecked( running );

    int effectIdx = m_effectComboBox->findData( mode );
    if ( -1 != effectIdx )
    {
    	m_effectComboBox->setCurrentIndex( effectIdx );
    }
    int rangeIdx = m_rangeComboBox->findData( range );
    if ( -1 != rangeIdx )
    {
    	m_rangeComboBox->setCurrentIndex( rangeIdx );
    }

    m_enableCheckBox->blockSignals( false );
    m_effectComboBox->blockSignals( false );
    m_rangeComboBox->blockSignals( false );

    /*
    setContrast( config.contrast );
    setBrightness( config.brightness );
    setSaturation( config.saturation );
    setHue( config.hue );
    */
 }

