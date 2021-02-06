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

#include "level1/registerwidget.h"


#include <QtGui>
#include <QtXml/QDomDocument>
#include <QFile>
#include <QItemSelectionModel>
#include <QList>
#include <unistd.h>


RegisterWidget::RegisterWidget( VirtualCamera* camDevice, QWidget *parent, bool main_notsub )
  : QWidget(parent),
    m_camDevice(camDevice),
    m_main_notsub(main_notsub)
{
    m_addrOffset = m_main_notsub ? 0 : ( HAL_BASEADDR_MARVIN_2 - HAL_BASEADDR_MARVIN );
    createMainSelectionTabsGroupBox();
    createMainWindowDisplayGroupBox();
    createSubWindowRegisterGroupBox();
    createSubWindowButtonGroupBox();
    createLogDisplayGroupBox();
    createLogReplayHandleButtonGroupBox();

    registerSelectedNo = 25;
    pCamericSubItem = new QVector<QDomElement>;

    logTimeinformation();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget( MainSelectionTabsGroupBox );
    mainLayout->addWidget( MainWindowDisplayGroupBox );

    setLayout( mainLayout );
}

void RegisterWidget::addRegister
(
    QAbstractItemModel *model,
    const QString &accessMode,
    const QString &lineEditRegisterAddress,
    const QString &registerName,
    const QString &registerDefaultValue,
    const QString &registerCurrentValue
)
{

    int rowCount = model->rowCount();
    model->insertRow(rowCount);
    model->setData(model->index(rowCount, 0), accessMode);
    model->setData(model->index(rowCount, 1), lineEditRegisterAddress);
    model->setData(model->index(rowCount, 2), registerName);
    model->setData(model->index(rowCount, 3), registerDefaultValue);
    model->setData(model->index(rowCount, 4), registerCurrentValue);
}

void RegisterWidget::updateRegister(const QString &registerCurrentValue, uint32_t i)
{
    int rowCount = model->rowCount();
    model->item(i,4)->setText(registerCurrentValue);
}

QAbstractItemModel * RegisterWidget::createRegisterModel(QObject *parent)
{
    model = new QStandardItemModel(0, 5, parent);

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("R\\W"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Address"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Name"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Default"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Value"));

    return model;
}

void RegisterWidget::selectChanged(const QItemSelection & index1,const QItemSelection & index2)
{
    bool ok = true;
    uint32_t regValue;
    QModelIndexList indexlist = index1.indexes();

    row = indexlist[0].row();
    QString stringPermit;
    QVariant qvariantPermit = model->data(model->index(row, 0));
    QVariant qvariantAddress = model->data(model->index(row, 1));
    QVariant qvariantRegValue = model->data(model->index(row, 4));
    stringPermit = qvariantPermit.toString();
    stringAddress = qvariantAddress.toString();
    stringRegHexValue = qvariantRegValue.toString();

    regValue = stringRegHexValue.toUInt(&ok, 16);
    /*stringRegHexValue = QString::number(regValue, 16);*/
    stringRegHexValue.sprintf( "%08x", regValue );
    stringRegDecValue = QString::number(regValue, 10);
    stringRegBinValue = QString::number(regValue, 2);

    lineEditRegisterAddress->setText(stringAddress);
    hexValue->setText(stringRegHexValue);
    decValue->setText(stringRegDecValue);
    binValue->setText(stringRegBinValue);
    linePreStringValue = stringRegHexValue;
    IsValueEdited = true;
    if (stringPermit.indexOf("W")<0)
    {
        hexValue->setEnabled(false);
        decValue->setEnabled(false);
        binValue->setEnabled(false);
        setButton->setEnabled(false);
    }
    else
    {
        hexValue->setEnabled(true);
        decValue->setEnabled(true);
        binValue->setEnabled(true);
        setButton->setEnabled(true);

    }
}
#if 0
void RegisterWidget::selectChanged(const QModelIndex & index)
{
    bool ok = true;
    uint32_t regValue;
    row = index.row();
    QString stringPermit;
    QVariant qvariantPermit = model->data(model->index(row, 0));
    QVariant qvariantAddress = model->data(model->index(row, 1));
    QVariant qvariantRegValue = model->data(model->index(row, 4));
/*TODO*/
    stringPermit = qvariantPermit.toString();
    stringAddress = qvariantAddress.toString();
    stringRegHexValue = qvariantRegValue.toString();

    regValue = stringRegHexValue.toUInt(&ok, 16);
    /*stringRegHexValue = QString::number(regValue, 16);*/
    stringRegHexValue.sprintf( "%08x", regValue );
    stringRegDecValue = QString::number(regValue, 10);
    stringRegBinValue = QString::number(regValue, 2);

    lineEditRegisterAddress->setText(stringAddress);
    hexValue->setText(stringRegHexValue);
    decValue->setText(stringRegDecValue);
    binValue->setText(stringRegBinValue);
    linePreStringValue = stringRegHexValue;
    IsValueEdited = true;
    if (stringPermit.indexOf("W")<0)
    {
        hexValue->setEnabled(false);
        decValue->setEnabled(false);
        binValue->setEnabled(false);
        setButton->setEnabled(false);
    }
    else
    {
        hexValue->setEnabled(true);
        decValue->setEnabled(true);
        binValue->setEnabled(true);
        setButton->setEnabled(true);

    }
}
#endif

void RegisterWidget::createMainWindowDisplayGroupBox()
{
    MainWindowDisplayGroupBox = new QGroupBox(("CamerIc Module"));

    SubWindowRegisterGroupBox = new QGroupBox;
    LogDisplayGroupBox = new QGroupBox;

    QGridLayout* layout = new QGridLayout;

    layout->addWidget( SubWindowRegisterGroupBox, 0, 0 );
    // layout->addWidget(LogDisplayGroupBox, 1,0 );

    MainWindowDisplayGroupBox->setLayout(layout);
}

void RegisterWidget::logBrowse()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                tr("Open XML File"), QDir::currentPath(),tr("XML Files (*.xml)"));

    displayLogContent(fileName);
}

void RegisterWidget::displayLogContent(QString fileName)
{
    QDomDocument doc;
    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"File has loaded...";
    }
    if (!doc.setContent(&file))
    {
        qDebug()<<"No file to read...";
        file.close();
        return;
    }

    QString string = doc.toString();
    logInformationDisplayBox->setPlainText(string);
    openLogXmlFileName = fileName;
    file.close();
}

void RegisterWidget::createSubLogReplayHandleButtonGroupBox()
{
    logBrowseButton = new QPushButton(tr("Browse..."));
    logReplayConfirmButton = new QPushButton(tr("Replay"));
    QGridLayout* layout = new QGridLayout;

    layout->addWidget(logBrowseButton, 0,0 );
    layout->addWidget(logReplayConfirmButton, 1,0 );

    SubLogReplayHandleButtonGroupBox->setLayout(layout);

    connect( logBrowseButton, SIGNAL(clicked()), this, SLOT(logBrowse()) );
    connect( logReplayConfirmButton, SIGNAL(clicked()), this, SLOT(logReplay()) );
}

void RegisterWidget::logReplayButtonClicked()
{
    QGridLayout* layout = new QGridLayout;
    SubLogReplayHandleButtonGroupBox = new QGroupBox;
    QGroupBox *groupBox = new QGroupBox(tr("Log XML file selection"));

    logInformationDisplayBox = new QTextEdit;

    layout->addWidget(logInformationDisplayBox , 0, 0);
    layout->addWidget(SubLogReplayHandleButtonGroupBox, 0, 1);


    groupBox->setMinimumSize(1000,600);
    groupBox->setLayout(layout);

    groupBox->show();

    logInformationDisplayBox->clear();
    createSubLogReplayHandleButtonGroupBox();

}

void RegisterWidget::logTimeinformation()
{
    QDateTime datetime = QDateTime::currentDateTime();
    saveLogXmlFileName = datetime.toString(tr("yyyyMMdd-hhmmss"));
}

void RegisterWidget::logReplay()
{
    RESULT result = RET_SUCCESS;
    uint32_t regValue;
    bool ok;
    uint32_t                replayAddress;
    uint32_t                replayValue;

    QDomDocument doc;
    QFile file(openLogXmlFileName);
    if(file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"File has loaded...";
    }
    if (!doc.setContent(&file))
    {
        qDebug()<<"No file to read...";
        file.close();
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode CamerIc1_element = docElem.firstChild();
    QDomNode CamerIc1Child_element = CamerIc1_element.firstChild();
    while(!CamerIc1_element.isNull())
    {
        QDomElement e = CamerIc1_element.toElement();
        if(!e.isNull())
        {
            while(!CamerIc1Child_element.isNull())
            {
                QDomElement c = CamerIc1Child_element.toElement();
                if(!c.isNull())
                {
                    QString string = c.attribute("address","");
                    linePreStringValue = c.attribute("PreValue","");
/*                    linePreStringValue = linePreStringValue -"0x";*/
                    replayAddress = string.toUInt(&ok,16);
                    replayValue = c.text().toUInt(&ok,16);

                    //Todo: change to IOCTL
                    //result = CamerIcSetRegister(nullptr, replayAddress + m_addrOffset, replayValue );
                    result = RET_SUCCESS;
                    if (result != RET_SUCCESS)
                    {
                        printf("Can not read the register value!\n");
                    }
                    refreshRegisterEditLogBox(c.tagName(), string, c.text());
                    usleep(2500);
                }
                CamerIc1Child_element = CamerIc1Child_element.nextSibling();
            }
        }
        CamerIc1_element = CamerIc1_element.nextSibling();
        CamerIc1Child_element = CamerIc1_element.firstChild();
    }
    file.close();
}

void RegisterWidget::logResetButtonClicked()
{
    registerValueEditLogBox->clear();
}

void RegisterWidget::createLogReplayHandleButtonGroupBox()
{
    logReplayButton = new QPushButton(tr("Load"));
    logResetButton = new QPushButton(tr("Reset")) ;
    QGridLayout* layout = new QGridLayout;

    layout->addWidget(logReplayButton, 0,0 );
    layout->addWidget(logResetButton, 0,1 );

    LogReplayHandleButtonGroupBox->setLayout(layout);

    connect(logReplayButton, SIGNAL(clicked()), this, SLOT(logReplayButtonClicked()));
    connect(logResetButton, SIGNAL(clicked()), this, SLOT(logResetButtonClicked()));
}

void RegisterWidget::createLogDisplayGroupBox()
{
    registerValueEditLogBox = new QTextEdit;
    LogReplayHandleButtonGroupBox = new QGroupBox;

    registerValueEditLogBox->setReadOnly(true);

    QGridLayout* layout = new QGridLayout;

    layout->addWidget(registerValueEditLogBox, 0, 0);
    layout->addWidget(LogReplayHandleButtonGroupBox, 1,0 );

    LogDisplayGroupBox->setLayout(layout);
}

void RegisterWidget::createSubWindowRegisterGroupBox()
{
    QGridLayout* layout = new QGridLayout;
    QAbstractItemModel *model = createRegisterModel(SubWindowRegisterGroupBox);

    SubWindowButtonGroupBox = new QGroupBox;

    registerView = new QTreeView;
    registerView->setRootIsDecorated(false);
    registerView->setAlternatingRowColors(true);
    registerView->setModel(model);
    QItemSelectionModel *mySelectmodel = new QItemSelectionModel(model);
    registerView->setSelectionModel(mySelectmodel);


    connect(registerView->selectionModel(), SIGNAL(selectionChanged( const QItemSelection &,const QItemSelection &)), this, SLOT(selectChanged(const QItemSelection &,const QItemSelection &)));
    //connect(registerView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(selectChanged(const QModelIndex &)));


    layout->addWidget(registerView, 0, 0);
    layout->addWidget(SubWindowButtonGroupBox,1,0);

    SubWindowRegisterGroupBox->setLayout(layout);
}

void RegisterWidget::createSubWindowButtonGroupBox()
{
    QGridLayout* layout = new QGridLayout;

    setButton           = new QPushButton(tr("Set"));
    QPushButton* refreshButton         = new QPushButton(tr("Refresh"));
    QPushButton* readRegFileButton         = new QPushButton(tr("Read Register File"));
    QPushButton* writeRegFileButton        = new QPushButton(tr("Write Register File"));

    lineEditRegisterAddress = new QLineEdit;
    hexValue = new QLineEdit(this);
    decValue = new QLineEdit;
    binValue = new QLineEdit;

    lineEditRegisterAddress->setReadOnly(true);

    QLabel *addressLabel = new QLabel(tr("ad"));
    QLabel *hexValueLabel = new QLabel(tr("0x"));
    QLabel *decValueLabel = new QLabel(tr("0d"));
    QLabel *binValueLabel = new QLabel(tr("0b"));

    layout->addWidget( addressLabel,            0, 0 );
    layout->addWidget( lineEditRegisterAddress, 0, 1 );
    layout->addWidget( hexValueLabel,           0, 2 );
    layout->addWidget( hexValue,                0, 3 );
    layout->addWidget( decValueLabel,           1, 0 );
    layout->addWidget( decValue,                1, 1 );
    layout->addWidget( binValueLabel,           1, 2 );
    layout->addWidget( binValue,                1, 3 );

    layout->addWidget( setButton,               2, 1 );
    layout->addWidget( refreshButton,           2, 3 );

#ifdef DEBUG
    QPushButton* dumpButton = new QPushButton(tr("Dump"));
    QPushButton* dumpAllButton = new QPushButton(tr("Dump All"));
    connect(dumpButton, SIGNAL(clicked()), this, SLOT(dumpButtonClicked()));
    connect(dumpAllButton, SIGNAL(clicked()), this, SLOT(dumpAllButtonClicked()));
    layout->addWidget( dumpAllButton,           3, 1 );
    layout->addWidget( dumpButton,              3, 3 );
#endif // DEBUG

    //layout->addWidget( readRegFileButton,   3, 1 );
    //layout->addWidget( writeRegFileButton,  3, 3 );

    SubWindowButtonGroupBox->setLayout(layout);

    connect(hexValue,SIGNAL(textChanged(const QString &)),this ,SLOT(slotHexTextChanged()));
    connect(decValue,SIGNAL(textChanged(const QString &)),this ,SLOT(slotDecTextChanged()));
    connect(binValue,SIGNAL(textChanged(const QString &)),this ,SLOT(slotBinTextChanged()));

    connect(readRegFileButton, SIGNAL(clicked()), this, SLOT(readRegFileOpen()));
    connect(writeRegFileButton, SIGNAL(clicked()), this, SLOT(writeRegFileOpen()));
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(refreshRegFileButtonClicked()));
    connect(setButton, SIGNAL(clicked()), this, SLOT(setButtonClicked()));
}

void RegisterWidget::readRegFileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open XML File"), QDir::currentPath(),tr("XML Files (*.xml)"));

    readRegFileButtonClicked(fileName);
}

void RegisterWidget::writeRegFileOpen()
{
    bool bFileExists;
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save XML File"), QDir::currentPath(),tr("XML Files (*.xml)"));
    QDir qdir ;
    bFileExists = qdir.exists(fileName);
    if(bFileExists == true)
    {
        replaceRegFileButtonClicked(fileName);
    }
    else
    {
        createRegFileButtonClicked(fileName);
    }
}

void RegisterWidget::slotHexTextChanged()
{
    bool ok;
    uint32_t regValue;
    if(true == IsValueEdited)       /*  The slot Signal is SIGNAL(textChanged)*/
    {
        IsValueEdited = false;
        lineHexEditText = hexValue->text();
        regValue = lineHexEditText.toUInt(&ok, 16);
        if(false == ok)
        {
            hexTextErrorMessageDialog = new QErrorMessage();
            hexTextErrorMessageDialog->showMessage(tr("HexText input error"));
            regValue = linePreStringValue.toUInt(&ok, 16);
            stringRegHexValue = linePreStringValue;
            hexValue->setText(stringRegHexValue);
        }
        stringRegHexValue.sprintf( "%08x", regValue );
        stringRegDecValue = QString::number(regValue, 10);
        stringRegBinValue = QString::number(regValue, 2);
        hexValue->setText(stringRegHexValue);
        decValue->setText(stringRegDecValue);
        binValue->setText(stringRegBinValue);
        lineHexEditText = stringRegHexValue;
    }
    else
    {
        return;
    }
    IsValueEdited = true;
}

void RegisterWidget::slotDecTextChanged()
{
    bool ok;
    uint32_t regValue;
    uint32_t regValueTemp;
    if(true == IsValueEdited)       /*  The slot Signal is SIGNAL(textChanged)*/
    {
        IsValueEdited = false;
        lineDecEditText = decValue->text();
        regValue = lineDecEditText.toUInt(&ok, 10);
        if(false == ok)
        {
            regValue = linePreStringValue.toUInt(&ok, 16);
            stringRegDecValue = QString::number(regValue, 10);
            decValue->setText(stringRegDecValue);
            hexTextErrorMessageDialog = new QErrorMessage();
            hexTextErrorMessageDialog->showMessage(tr("DecText input error"));

        }
        stringRegHexValue = QString::number(regValue, 16);
        /****TODO
        regValueTemp = stringRegHexValue.toUInt(&ok, 16);
        stringRegHexValue.sprintf( "%08x", regValueTemp );
        *****/
        stringRegBinValue = QString::number(regValue, 2);
        hexValue->setText(stringRegHexValue);
        binValue->setText(stringRegBinValue);
        lineHexEditText = stringRegHexValue;
    }
    else
    {
        return;
    }
    IsValueEdited = true;
}

void RegisterWidget::slotBinTextChanged()
{
    bool ok;
    uint32_t regValue;
    if(true == IsValueEdited)       /* The slot Signal is SIGNAL(textChanged)*/
    {
        IsValueEdited = false;
        lineBinEditText = binValue->text();
        regValue = lineBinEditText.toUInt(&ok, 2);
        if(false == ok)
        {
            regValue = linePreStringValue.toUInt(&ok, 16);
            stringRegBinValue = QString::number(regValue, 2);
            binValue->setText(stringRegBinValue);
            binTextErrorMessageDialog = new QErrorMessage();
            binTextErrorMessageDialog->showMessage(tr("BinText input error"));
        }
        stringRegHexValue = QString::number(regValue, 16);
        stringRegDecValue = QString::number(regValue, 10);
        hexValue->setText("0x"+stringRegHexValue);
        decValue->setText(stringRegDecValue);
        lineHexEditText = stringRegHexValue;
    }
    else
    {
        return;
    }
    IsValueEdited = true;
}

void RegisterWidget::setButtonClicked()
{
    RESULT                  result = RET_SUCCESS;
    uint32_t regValue;
    bool ok;
    /*qDebug()<<qPrintable(lineHexEditText)<<endl;*/
    if((true == IsValueEdited)&&(linePreStringValue != lineHexEditText))
    {
        regValue = lineHexEditText.toUInt(&ok,16);
        result = RET_SUCCESS;
        //result = CamerIcSetRegister(nullptr, RegistersMap[row].Address + m_addrOffset, regValue);
        if (result != RET_SUCCESS)
        {
            printf("Can not read the register value!\n");
        }
        saveLogInforToXmlFile();
        refreshRegisterEditLogBox(stringModuleName, stringAddress, lineHexEditText);
        refreshRegFileButtonClicked();
    }
}

void RegisterWidget::saveLogInforToXmlFile()
{
    uint16_t i = 0u;
    uint16_t j = 0u;
    QString string ;
    QDomText text;
    QDomElement* element;
    bool bFileExists = false;
    QString fileName;
    QDir qdir ;

    bFileExists = qdir.exists(saveLogXmlFileName+".xml");
    QDomDocument doc;
    QFile file(saveLogXmlFileName+".xml");

    if(file.open(QIODevice::ReadWrite))
    {
        qDebug()<<"File has Saved...";
    }

    if (true == bFileExists)
    {
        if (!doc.setContent(&file))
        {
            qDebug()<<"No file to read...";
            file.close();
            return;
        }

        pCamericSubItem->clear();
        QDomElement docElem = doc.documentElement();
        QDomElement subRoot = doc.createElement(xmlStringModuleName);
        docElem.appendChild(subRoot);

        element = new QDomElement();
        *element = doc.createElement(model->item(i,2)->text());
        pCamericSubItem->append(*element);
        (*element).setAttribute(tr("address"),stringAddress);
        (*element).setAttribute(tr("PreValue"),"0x"+linePreStringValue);
        (*element).setAttribute(tr("size"),tr("[1 1]"));
        (*element).setAttribute(tr("type"),tr("uint32_t"));
        text = doc.createTextNode("0x"+lineHexEditText);
        pCamericSubItem->value(0).appendChild(text);
        subRoot.appendChild(pCamericSubItem->value(0));
        file.reset();
        QTextStream out(&file);
        doc.save(out,4);
        file.close();
    }
    else
    {
        pCamericSubItem->clear();

        QDomProcessingInstruction instruction;
        instruction = doc.createProcessingInstruction("xml","version=\"1.0\"");
        doc.appendChild(instruction);

        QDomElement root=doc.createElement(tr("CAMERIC_1"));
        doc.appendChild(root);

        QDomElement subRoot = doc.createElement(xmlStringModuleName);
        root.appendChild(subRoot);

        element = new QDomElement();
        *element = doc.createElement(model->item(i,2)->text());
        pCamericSubItem->append(*element);
        (*element).setAttribute(tr("address"),stringAddress);
        (*element).setAttribute(tr("PreValue"),"0x"+linePreStringValue);
        (*element).setAttribute(tr("size"),tr("[1 1]"));
        (*element).setAttribute(tr("type"),tr("uint32_t"));
        text = doc.createTextNode("0x"+lineHexEditText);
        pCamericSubItem->value(0).appendChild(text);
        subRoot.appendChild(pCamericSubItem->value(0));

        file.reset();
        QTextStream out(&file);
        doc.save(out,4);
        file.close();
    }
}

void RegisterWidget::refreshRegisterEditLogBox(QString textStringModuleName,QString textStringAddress , QString textStringValue )
{
    registerValueEditLogBox->insertPlainText("Set Module ("+textStringModuleName+" ): Address :"+ textStringAddress+" value : 0x"+linePreStringValue +"-->0x" + textStringValue +"\n");
    linePreStringValue = lineHexEditText;
}

void RegisterWidget::createMainSelectionTabsGroupBox()
{
    MainSelectionTabsGroupBox = new QGroupBox( tr("Select CamerIc Module") );
    QGridLayout *layout = new QGridLayout;

    QAction* mainControlButton           = new QAction(tr("01 Main Control"),this);
    QAction* imageEffectsButton          = new QAction(tr("02 Image Effects"),this);
    QAction* superImposeButton           = new QAction(tr("03 Superimpose"),this);
    QAction* ispMainButton               = new QAction(tr("04 ISP Main"),this);
    QAction* ispFlashButton              = new QAction(tr("05 ISP Flash"),this);
    QAction* ispShutterButton            = new QAction(tr("06 ISP Shutter"),this);
    QAction* ispCprogButton              = new QAction(tr("07 CPROC"),this);
    QAction* mainResizeButton            = new QAction(tr("08 Main Resize"),this);
    QAction* selfResizeButton            = new QAction(tr("09 Self Resize"),this);
    QAction* miButton                    = new QAction(tr("10 Memory Interface"),this);
    QAction* jpegButton                  = new QAction(tr("11 JPEG"),this);
    QAction* smiaButton                  = new QAction(tr("12 SMIA"),this);
    QAction* mipiButton                  = new QAction(tr("13 MIPI"),this);
    QAction* afmButton                   = new QAction(tr("14 ISP AFM"),this);
    QAction* badPixButton                = new QAction(tr("15 ISP BP"),this);
    QAction* lscButton                   = new QAction(tr("16 ISP LSC"),this);
    QAction* imageStabButton             = new QAction(tr("17 ISP IS"),this);
    QAction* ispHistButton               = new QAction(tr("18 ISP Hist"),this);
    QAction* ispFiltButton               = new QAction(tr("19 ISP Filt"),this);
    QAction* ipsCACButton                = new QAction(tr("20 ISP CAC"),this);
    QAction* expMeasureButton            = new QAction(tr("21 ISP EXP"),this);
    QAction* ispBlsButton                = new QAction(tr("22 ISP BLS"),this);
    QAction* ispDpfButton                = new QAction(tr("23 ISP DPF"),this);
    QAction* ispDpccButton               = new QAction(tr("24 ISP DPCC"),this);
    QAction* ispWdrButton                = new QAction(tr("25 ISP WDR"),this);
    QAction* ispAwbButton                = new QAction(tr("26 ISP AWB"),this);

    popupButton = new QPushButton(tr("Module"));
    QMenu *menu = new QMenu(this);
    menu->addAction(mainControlButton);
    menu->addAction(imageEffectsButton);
    menu->addAction(superImposeButton);
    menu->addAction(ispMainButton);
    menu->addAction(ispFlashButton);
    menu->addAction(ispShutterButton);
    menu->addAction(ispCprogButton);
    menu->addAction(mainResizeButton);
    menu->addAction(selfResizeButton);
    menu->addAction(miButton);
    menu->addAction(jpegButton);
    menu->addAction(smiaButton);
    menu->addAction(mipiButton);
    menu->addAction(afmButton);
    menu->addAction(badPixButton);
    menu->addAction(lscButton);
    menu->addAction(imageStabButton);
    menu->addAction(ispHistButton);
    menu->addAction(ispFiltButton);
    menu->addAction(ipsCACButton);
    menu->addAction(expMeasureButton);
    menu->addAction(ispBlsButton);
    menu->addAction(ispDpfButton);
    menu->addAction(ispDpccButton);
    menu->addAction(ispWdrButton);
    menu->addAction(ispAwbButton);
    popupButton->setMenu(menu);

    layout->addWidget(popupButton, 0, 0);

    MainSelectionTabsGroupBox->setLayout(layout);

    connect(mainControlButton, SIGNAL(triggered()), this, SLOT(mainControlButtonClicked()));
    connect(imageEffectsButton, SIGNAL(triggered()), this, SLOT(imageEffectsButtonClicked()));
    connect(superImposeButton, SIGNAL(triggered()), this, SLOT(superImposeButtonClicked()));
    connect(ispMainButton, SIGNAL(triggered()), this, SLOT(ispMainButtonClicked()));
    connect(ispFlashButton, SIGNAL(triggered()), this, SLOT(ispFlashButtonClicked()));
    connect(ispShutterButton, SIGNAL(triggered()), this, SLOT(ispShutterButtonClicked()));
    connect(ispCprogButton, SIGNAL(triggered()), this, SLOT(ispCprogButtonClicked()));
    connect(mainResizeButton, SIGNAL(triggered()), this, SLOT(mainResizeButtonClicked()));
    connect(selfResizeButton, SIGNAL(triggered()), this, SLOT(selfResizeButtonClicked()));

    connect(miButton, SIGNAL(triggered()), this, SLOT(miButtonClicked()));
    connect(jpegButton, SIGNAL(triggered()), this, SLOT(jpegButtonClicked()));
    connect(smiaButton, SIGNAL(triggered()), this, SLOT(smiaButtonClicked()));
    connect(mipiButton, SIGNAL(triggered()), this, SLOT(mipiButtonClicked()));
    connect(afmButton, SIGNAL(triggered()), this, SLOT(afmButtonClicked()));
    connect(badPixButton, SIGNAL(triggered()), this, SLOT(badPixButtonClicked()));
    connect(lscButton, SIGNAL(triggered()), this, SLOT(lscButtonClicked()));
    connect(imageStabButton, SIGNAL(triggered()), this, SLOT(imageStabButtonClicked()));
    connect(ispHistButton, SIGNAL(triggered()), this, SLOT(ispHistButtonClicked()));
    connect(ispFiltButton, SIGNAL(triggered()), this, SLOT(ispFiltButtonClicked()));

    connect(ipsCACButton, SIGNAL(triggered()), this, SLOT(ipsCACButtonClicked()));
    connect(expMeasureButton, SIGNAL(triggered()), this, SLOT(expMeasureButtonClicked()));
    connect(ispBlsButton, SIGNAL(triggered()), this, SLOT(ispBlsButtonClicked()));
    connect(ispDpfButton, SIGNAL(triggered()), this, SLOT(ispDpfButtonClicked()));
    connect(ispDpccButton, SIGNAL(triggered()), this, SLOT(ispDpccButtonClicked()));
    connect(ispWdrButton, SIGNAL(triggered()), this, SLOT(ispWdrButtonClicked()));
    connect(ispAwbButton, SIGNAL(triggered()), this, SLOT(ispAwbButtonClicked()));

}

void RegisterWidget::refreshRegFileButtonClicked()
{
    if( registerSelectedNo == 26)   /* No Module select*/
    {
        return;
    }
    switch(registerSelectedNo)
    {
        case 25:
            updateRegisterValues(CAMERIC_MODULE_ISP_AWB);
            break;
        case 24:
            updateRegisterValues(CAMERIC_MODULE_ISP_WDR);
            break;
        case 23:
            updateRegisterValues(CAMERIC_MODULE_ISP_DPCC);
            break;
        case 22:
            updateRegisterValues(CAMERIC_MODULE_ISP_DPF);
            break;
        case 21:
            updateRegisterValues(CAMERIC_MODULE_ISP_BLS);
            break;
        case 20:
            updateRegisterValues(CAMERIC_MODULE_ISP_EXP);
            break;
        case 19:
            updateRegisterValues(CAMERIC_MODULE_ISP_CAC);
            break;
        case 18:
            updateRegisterValues(CAMERIC_MODULE_ISP_FILTER);
            break;
        case 17:
            updateRegisterValues(CAMERIC_MODULE_ISP_HIST);
            break;
        case 16:
            updateRegisterValues(CAMERIC_MODULE_ISP_IS);
            break;
        case 15:
            updateRegisterValues(CAMERIC_MODULE_ISP_LSC);
            break;
        case 14:
            updateRegisterValues(CAMERIC_MODULE_ISP_BP);
            break;
        case 13:
            updateRegisterValues(CAMERIC_MODULE_ISP_AFM);
            break;
        case 12:
            updateRegisterValues(CAMERIC_MODULE_MIPI);
            break;
        case 11:
            updateRegisterValues(CAMERIC_MODULE_SMIA);
            break;
        case 10:
            updateRegisterValues(CAMERIC_MODULE_JPE);
            break;
        case 9:
            updateRegisterValues(CAMERIC_MODULE_MI);
            break;
        case 8:
            updateRegisterValues(CAMERIC_MODULE_SRSZ);
            break;
        case 7:
            updateRegisterValues(CAMERIC_MODULE_MRSZ);
            break;
        case 6:
            updateRegisterValues(CAMERIC_MODULE_CPROC);
            break;
        case 5:
            updateRegisterValues(CAMERIC_MODULE_ISP_SHUTTER);
            break;
        case 4:
            updateRegisterValues(CAMERIC_MODULE_ISP_FLASH);
            break;
        case 3:
            updateRegisterValues(CAMERIC_MODULE_ISP_MAIN);
            break;
        case 2:
            updateRegisterValues(CAMERIC_MODULE_SUPER_IMPOSE);
            break;
        case 1:
            updateRegisterValues(CAMERIC_MODULE_IMG_EFFECTS);
            break;
        case 0:
            updateRegisterValues(CAMERIC_MODULE_MAIN_CONTROL);
            break;
        default:
            break;
    }
}

void RegisterWidget::changeRegModuleFile(QString fileName)
{

    uint16_t i = 0u;
    uint16_t j = 0u;
    QString string ;
    QDomDocument doc("marvin_registers_v1");
    QFile file(fileName);
    if(file.open(QIODevice::ReadWrite))
    {
        qDebug()<<"File has loaded...";
    }
    if (!doc.setContent(&file))
    {
        qDebug()<<"No file to read...";
        file.close();
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode CamerIc1_element = docElem.firstChild();
    QDomNode CamerIc1Child_element = CamerIc1_element.firstChild();
    while(!CamerIc1_element.isNull())
    {
            QDomElement e = CamerIc1_element.toElement();
        if(e.tagName() == xmlStringModuleName)   /*check which register selected*/
        {
            if(!e.isNull())
            {
                j = 0u;
                while(!CamerIc1Child_element.isNull())
                {
                    QDomElement c = CamerIc1Child_element.toElement();
                    if(!c.isNull())
                    {
                        QDomNode oldnode = c.firstChild();
                        QVariant qvariant = model->data(model->index(j, 4));
                        string = qvariant.toString();
                        c.firstChild().setNodeValue(string);
                        QDomNode newnode = c.firstChild();
                        c.replaceChild(newnode,oldnode);
                    }
                    CamerIc1Child_element = CamerIc1Child_element.nextSibling();
                    ++j;
                }
                file.reset();
                QTextStream out(&file);
                doc.save(out,4);
            }
        }
        CamerIc1_element = CamerIc1_element.nextSibling();
        CamerIc1Child_element = CamerIc1_element.firstChild();
        ++i;
    }
    file.close();
}

void RegisterWidget::insertRegModuleFile(QString fileName)
{
    uint16_t i = 0u;
    uint16_t j = 0u;
    QDomElement* element;
    QDomText text;

    QDomDocument doc("marvin_registers_v1");
    QFile file(fileName);
    if(file.open(QIODevice::ReadWrite))
    {
        qDebug()<<"File has loaded...";
    }
    if (!doc.setContent(&file))
    {
        qDebug()<<"No file to read...";
        file.close();
        return;
    }

    pCamericSubItem->clear();
    QDomElement docElem = doc.documentElement();
    QDomElement subRoot = doc.createElement(xmlStringModuleName);
    docElem.appendChild(subRoot);
    element = new QDomElement();
    for(i = 0; i < NumRegisters ;++i)
    {
        *element = doc.createElement(model->item(i,2)->text());
        pCamericSubItem->append(*element);
        (*element).setAttribute(tr("size"),tr("[1 1]"));
        (*element).setAttribute(tr("type"),tr("uint32_t"));
        text = doc.createTextNode(model->item(i,3)->text());
        pCamericSubItem->value(i).appendChild(text);
        subRoot.appendChild(pCamericSubItem->value(i));
    }
    file.reset();
    QTextStream out(&file);
    doc.save(out,4);
    file.close();
}

void RegisterWidget::replaceRegFileButtonClicked(QString fileName)
{
    uint16_t i = 0u;
    bool bModuleIsExist = false;

    QDomDocument doc("marvin_registers_v1");
    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"File has loaded...";
    }
    if (!doc.setContent(&file))
    {
        qDebug()<<"No file to read...";
        file.close();
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode CamerIc1_element = docElem.firstChild();

    while(!CamerIc1_element.isNull())
    {
        QDomElement e = CamerIc1_element.toElement();
        if(e.tagName() == xmlStringModuleName)     /*check which register selected*/
        {
            bModuleIsExist = true;
        }

        CamerIc1_element = CamerIc1_element.nextSibling();
        ++i;
    }

    /*check if Module xmlStringModuleName exists*/
    if(bModuleIsExist == true)
    {
        changeRegModuleFile(fileName);
    }
    else
    {
        insertRegModuleFile(fileName);
    }

    file.close();
}


void RegisterWidget::createRegFileButtonClicked(QString fileName)
{
    uint16_t i = 0u;
    uint16_t j = 0u;
    QString string ;
    QDomText text;
    QDomElement* element;

    QFile file(fileName);

    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug()<<"File has Saved...";
    }

    pCamericSubItem->clear();

    QDomDocument doc;
    QDomProcessingInstruction instruction;
    instruction = doc.createProcessingInstruction("xml","version=\"1.0\"");
    doc.appendChild(instruction);

    QDomElement root=doc.createElement(tr("CAMERIC_1"));
    doc.appendChild(root);

    QDomElement subRoot = doc.createElement(xmlStringModuleName);
    root.appendChild(subRoot);

    for(i = 0; i < NumRegisters ;++i)
    {
        element = new QDomElement();
        *element = doc.createElement(model->item(i,2)->text());
        pCamericSubItem->append(*element);
        (*element).setAttribute(tr("size"),tr("[1 1]"));
        (*element).setAttribute(tr("type"),tr("uint32_t"));
        text = doc.createTextNode(model->item(i,3)->text());
        pCamericSubItem->value(i).appendChild(text);
        subRoot.appendChild(pCamericSubItem->value(i));
    }
    file.reset();

    QTextStream out(&file);

    doc.save(out,4);

    file.close();
}

void RegisterWidget::readRegFileButtonClicked(QString fileName)
{
    uint16_t i = 0u;
    uint16_t j = 0u;
    bool bReadSuccess = false;

    QDomDocument doc("marvin_registers_v1");
    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"File has loaded...";
    }
    if (!doc.setContent(&file))
    {
        qDebug()<<"No file to read...";
        file.close();
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode CamerIc1_element = docElem.firstChild();
    QDomNode CamerIc1Child_element = CamerIc1_element.firstChild();
    while(!CamerIc1_element.isNull())
    {
            QDomElement e = CamerIc1_element.toElement();
        if( e.tagName() == xmlStringModuleName)        /*check which register selected*/
        {
            bReadSuccess = true;
            if(!e.isNull())
            {
                j = 0u;
                while(!CamerIc1Child_element.isNull())
                {
                    QDomElement c = CamerIc1Child_element.toElement();
                    if(!c.isNull())
                    {
                        model->setData(model->index(j, 4), c.text());
                    }
                    CamerIc1Child_element = CamerIc1Child_element.nextSibling();
                    ++j;
                }
            }
        }
        CamerIc1_element = CamerIc1_element.nextSibling();
        CamerIc1Child_element = CamerIc1_element.firstChild();
        ++i;
    }
    if(false == bReadSuccess)
    {
        readRegFileErrorMessageDialog = new QErrorMessage();
        readRegFileErrorMessageDialog->showMessage(tr("The xml file you loaded doesn't have the ")+xmlStringModuleName+tr(" information"));
    }
    file.close();
}

void RegisterWidget::ispAwbButtonClicked()
{
    popupButton->setText("26 ISP AWB");
    //printf("ispWdrButton is clicked. \n");
    registerSelectedNo = 25;
    stringModuleName = "26 ISP AWB";
    xmlStringModuleName = "CAMERIC_MODULE_ISP_AWB";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_ISP_AWB);
}

void RegisterWidget::ispWdrButtonClicked()
{
    popupButton->setText("25 ISP WDR");
    //printf("ispWdrButton is clicked. \n");
    registerSelectedNo = 24;
    stringModuleName = "25 ISP WDR";
    xmlStringModuleName = "CAMERIC_MODULE_ISP_WDR";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_ISP_WDR);
}

void RegisterWidget::ispDpccButtonClicked()
{
    popupButton->setText("24 ISP DPCC");
    //printf("ispDpccButton is clicked. \n");
    registerSelectedNo = 23;
    stringModuleName = "24 ISP DPCC";
    xmlStringModuleName = "CAMERIC_MODULE_ISP_DPCC";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_ISP_DPCC);
}

void RegisterWidget::ispDpfButtonClicked()
{
    popupButton->setText("23 ISP DPF");
    //printf("ispDpfButton is clicked. \n");
    registerSelectedNo = 22;
    stringModuleName = "23 ISP DPF";
    xmlStringModuleName = "CAMERIC_MODULE_ISP_DPF";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_ISP_DPF);
}

void RegisterWidget::ispBlsButtonClicked()
{
    popupButton->setText("22 ISP BLS");
    //printf("ispBlsButton is clicked. \n");
    registerSelectedNo = 21;
    stringModuleName = "22 ISP BLS";
    xmlStringModuleName = "CAMERIC_MODULE_ISP_BLS";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_ISP_BLS);
}

void RegisterWidget::expMeasureButtonClicked()
{
    popupButton->setText("21 ISP EXP");
    //printf("expMeasureButton is clicked. \n");
    registerSelectedNo = 20;
    stringModuleName = "21 ISP EXP";
    xmlStringModuleName = "CAMERIC_MODULE_ISP_EXP";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_ISP_EXP);
}

void RegisterWidget::ipsCACButtonClicked()
{
    popupButton->setText("20 ISP CAC");
    //printf("ipsCACButton is clicked. \n");
    registerSelectedNo = 19;
    stringModuleName = "20 ISP CAC";
    xmlStringModuleName = "CAMERIC_MODULE_ISP_CAC";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_ISP_CAC);
}

void RegisterWidget::ispFiltButtonClicked()
{
    popupButton->setText("19 ISP Filt");
    //printf("ispFiltButton is clicked. \n");
    registerSelectedNo = 18;
    stringModuleName = "19 ISP Filt";
    xmlStringModuleName = "CAMERIC_MODULE_ISP_FILT";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_ISP_FILTER);
}

void RegisterWidget::ispHistButtonClicked()
{
    popupButton->setText("18 ISP Hist");
    //printf("ispHistButton is clicked. \n");
    registerSelectedNo = 17;
    stringModuleName = "18 ISP Hist";
    xmlStringModuleName = "CAMERIC_MODULE_ISP_HIST";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_ISP_HIST);
}

void RegisterWidget::imageStabButtonClicked()
{
    popupButton->setText("17 ISP IS");
    //printf("imageStabButton is clicked. \n");
    registerSelectedNo = 16;
    stringModuleName = "17 ISP IS";
    xmlStringModuleName = "CAMERIC_MODULE_IS";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_ISP_IS);
}

void RegisterWidget::lscButtonClicked()
{
    popupButton->setText("16 ISP LSC");
    //printf("lscButton is clicked. \n");
    registerSelectedNo = 15;
    stringModuleName = "16 ISP LSC";
    xmlStringModuleName = "CAMERIC_MODULE_LSC";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_ISP_LSC);
}

void RegisterWidget::badPixButtonClicked()
{
    popupButton->setText("15 ISP BP");
    //printf("badPixButton is clicked. \n");
    registerSelectedNo = 14;
    stringModuleName = "15 ISP BP";
    xmlStringModuleName = "CAMERIC_MODULE_BP";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_ISP_BP);
}

void RegisterWidget::afmButtonClicked()
{
    popupButton->setText("14 ISP AFM");
    //printf("afmButton is clicked. \n");
    registerSelectedNo = 13;
    stringModuleName = "14 ISP AFM";
    xmlStringModuleName = "CAMERIC_MODULE_AFM";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_ISP_AFM);
}

void RegisterWidget::mipiButtonClicked()
{
    popupButton->setText("13 MIPI");
    //printf("mipiButton is clicked. \n");
    registerSelectedNo = 12;
    stringModuleName = "13 MIPI";
    xmlStringModuleName = "CAMERIC_MODULE_MIPI";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_MIPI);
}

void RegisterWidget::smiaButtonClicked()
{
    popupButton->setText("12 SMIA");
    //printf("smiaButton is clicked. \n");
    registerSelectedNo = 11;
    stringModuleName = "12 SMIA";
    xmlStringModuleName = "CAMERIC_MODULE_SMIA";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_SMIA);
}

void RegisterWidget::jpegButtonClicked()
{
    popupButton->setText("11 JPEG");
    //printf("jpegButton is clicked. \n");
    registerSelectedNo = 10;
    stringModuleName = "11 JPEG";
    xmlStringModuleName = "CAMERIC_MODULE_JPE";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_JPE);
}

void RegisterWidget::miButtonClicked()
{
    popupButton->setText("10 Memory Interface");
    //printf("miButton is clicked. \n");
    registerSelectedNo = 9;
    stringModuleName = "10 Memory Interface";
    xmlStringModuleName = "CAMERIC_MODULE_MI";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_MI);
}

void RegisterWidget::selfResizeButtonClicked()
{
    popupButton->setText("09 Self Resize");
    //printf("selfResizeButton is clicked. \n");
    registerSelectedNo = 8;
    stringModuleName = "09 Self Resize";
    xmlStringModuleName = "CAMERIC_MODULE_SRSZ";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_SRSZ  );
}
void RegisterWidget::mainResizeButtonClicked()
{
    popupButton->setText("08 Main Resize");
    //printf("mainResizeButton is clicked. \n");
    registerSelectedNo = 7;
    stringModuleName = "08 Main Resize";
    xmlStringModuleName = "CAMERIC_MODULE_MRSZ";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_MRSZ);
}

void RegisterWidget::ispCprogButtonClicked()
{
    popupButton->setText("07 ISP CPROC");
    //printf("ispCprogButton is clicked. \n");
    registerSelectedNo = 6;
    stringModuleName = "07 CPROC";
    xmlStringModuleName = "CAMERIC_MODULE_CPROC";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_CPROC);
}

void RegisterWidget::ispShutterButtonClicked()
{
    popupButton->setText("06 ISP Shutter");
    //printf("ispShutterButton is clicked. \n");
    registerSelectedNo = 5;
    stringModuleName = "06 ISP Shutter";
    xmlStringModuleName = "CAMERIC_MODULE_ISP_SHUTTER";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_ISP_SHUTTER);
}

void RegisterWidget::ispFlashButtonClicked()
{
    popupButton->setText("05 ISP Flash");
    //printf("ispFlashButton is clicked. \n");
    registerSelectedNo = 4;
    stringModuleName = "05 ISP Flash";
    xmlStringModuleName = "CAMERIC_MODULE_ISP_FLASH";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_ISP_FLASH);
}

void RegisterWidget::ispMainButtonClicked()
{
    popupButton->setText("04 ISP Main");
    //printf("ispMainButton is clicked. \n");
    registerSelectedNo = 3;
    stringModuleName = "04 ISP Main";
    xmlStringModuleName = "CAMERIC_MODULE_ISP_MAIN";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_ISP_MAIN);
}

void RegisterWidget::superImposeButtonClicked()
{
    popupButton->setText("03 Super Impose");
    //printf("superImposeButton is clicked. \n");
    registerSelectedNo = 2;
    stringModuleName = "03 Superimpose";
    xmlStringModuleName = "CAMERIC_MODULE_SUPER_IMPOSE";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_SUPER_IMPOSE);
}

void RegisterWidget::imageEffectsButtonClicked()
{
    popupButton->setText("02 Image Effects");
    //printf("imageEffectsButton is clicked. \n");
    registerSelectedNo = 1;
    stringModuleName = "02 Image Effects";
    xmlStringModuleName = "CAMERIC_MODULE_IMG_EFFECTS";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_IMG_EFFECTS);
}

void RegisterWidget::mainControlButtonClicked()
{
    popupButton->setText("01 Main Control");
    //printf("mainControlButton is clicked. \n");
    registerSelectedNo = 0;
    stringModuleName = "01 Main Control";
    xmlStringModuleName = "CAMERIC_MODULE_MAIN_CONTROL";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
    retrieveRegisterValuesAndDisplay(CAMERIC_MODULE_MAIN_CONTROL);
}

void RegisterWidget::updateRegisterValues( const CamerIcModuleId_t moduleId )
{
    RESULT                  result = RET_SUCCESS;
    QString                 address;
    QString                 Permission;
    QString                 Name;
    QString                 resetValue;
    QString                 string;

    //TODO: change to IOCTL
    //result = CamerIcGetRegisterDescription( moduleId, &NumRegisters, &RegistersMap );
    result = RET_SUCCESS;

    if ( (result != RET_SUCCESS) || ( !NumRegisters ) || (!RegistersMap) )
    {
        printf("Module %d can not be accessed!\n", moduleId);
    }
    else
    {
        for (uint32_t i=0; i< NumRegisters; i++)
        {

#if 0
            uint32_t registerValue;
            uint32_t registerAddress;
            uint32_t registerResetValue;
            
            Permissions_t registerPermission;
            RegName_t registerName;

            result = CamerIcGetRegister( nullptr, RegistersMap[i].Address + m_addrOffset, &registerValue );
            
            strcpy(registerName , RegistersMap[i].Name);
            registerAddress = RegistersMap[i].Address;
            registerResetValue = RegistersMap[i].ResetValue;
            Permission = getRegPermissionValue(RegistersMap,i);
#endif

            /*check result*/
            if (result != RET_SUCCESS)
            {
                printf("Can not read the register value!\n");
            }
            else
            {
                //string.sprintf( "0x%08x", registerValue );
                //updateRegister(string,i);
            }
        }
    }

}

void RegisterWidget::retrieveRegisterValuesAndDisplay(const CamerIcModuleId_t moduleId)
{
    RESULT                  result = RET_SUCCESS;
    QString                 address;
    QString                 Permission;
    QString                 Name;
    QString                 resetValue;
    QString                 string;

    model->clear();
    model->setRowCount(0);
    model->setColumnCount(5);

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("R\\W"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Address"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Name"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Default"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Value"));

    //result = CamerIcGetRegisterDescription( moduleId, &NumRegisters, &RegistersMap );
    result = RET_SUCCESS;

    if ( (result != RET_SUCCESS) || ( !NumRegisters ) || (!RegistersMap) )
    {
        printf("Module %d can not be accessed!\n", moduleId);
    }
    else
    {
        for (uint32_t i=0; i< NumRegisters; i++)
        {
#if 0
            uint32_t registerValue;
            uint32_t registerAddress;
            uint32_t registerResetValue;
            Permissions_t registerPermission;
            RegName_t registerName;

            result = CamerIcGetRegister( nullptr, RegistersMap[i].Address + m_addrOffset, &registerValue );
            strcpy(registerName , RegistersMap[i].Name);
            registerAddress = RegistersMap[i].Address;
            registerResetValue = RegistersMap[i].ResetValue;
            Permission = getRegPermissionValue(RegistersMap,i);
#endif

            /*check result*/
            if (result != RET_SUCCESS)
            {
                printf("Can not read the register value!\n");
            }
            else
            {
#if 0
                string.sprintf( "0x%08x", registerValue );
                address.sprintf("0x%08x", registerAddress);
                resetValue.sprintf("0x%08x", registerResetValue);
                addRegister(model, Permission, address, registerName, resetValue, string);
#endif
            }
            paintFontAccordingPermission(RegistersMap,i);
            setFontAccordingPermission(RegistersMap,i);
        }
    }
}

void RegisterWidget::setFontAccordingPermission(RegDescription_t *RegistersMap, uint32_t i)
{
    /*TODO    model->item(i,0)->setFont(QFont("Times",18,QFont::Bold));*/
}

void RegisterWidget::paintFontAccordingPermission(RegDescription_t *RegistersMap, uint32_t i)
{
    registerView->resizeColumnToContents(0);
    registerView->resizeColumnToContents(2);
//Change to IOCTL expansions
#if 0
    if(RegistersMap[i].Permission == 1)                        /*PERM_READ_ONLY*/
    {
        model->item(i,0)->setForeground(QBrush(QColor(255,0,255)));
        model->item(i,1)->setForeground(QBrush(QColor(255,0,255)));
        model->item(i,2)->setForeground(QBrush(QColor(255,0,255)));
        model->item(i,3)->setForeground(QBrush(QColor(255,0,255)));
        model->item(i,4)->setForeground(QBrush(QColor(255,0,255)));
    }
    else if(RegistersMap[i].Permission == 2)                    /*PERM_READ_WRITE*/
    {
        model->item(i,0)->setForeground(QBrush(QColor(0,0,255)));
        model->item(i,1)->setForeground(QBrush(QColor(0,0,255)));
        model->item(i,2)->setForeground(QBrush(QColor(0,0,255)));
        model->item(i,3)->setForeground(QBrush(QColor(0,0,255)));
        model->item(i,4)->setForeground(QBrush(QColor(0,0,255)));
    }
    else if(RegistersMap[i].Permission == 3)                    /*PERM_WRITE_ONLY*/
    {
        model->item(i,0)->setForeground(QBrush(QColor(0,155,0)));
        model->item(i,1)->setForeground(QBrush(QColor(0,155,0)));
        model->item(i,2)->setForeground(QBrush(QColor(0,155,0)));
        model->item(i,3)->setForeground(QBrush(QColor(0,155,0)));
        model->item(i,4)->setForeground(QBrush(QColor(0,155,0)));
    }
    else
    {
        /*TODO*/
    }
#endif

    model->item(i,0)->setEditable(false);
    model->item(i,1)->setEditable(false);
    model->item(i,2)->setEditable(false);
    model->item(i,3)->setEditable(false);
    model->item(i,4)->setEditable(false);
}

QString RegisterWidget::getRegPermissionValue(RegDescription_t *RegistersMap, uint32_t i)
{
    QString Permission;
    /*Permission check*/
#if 0
    if(RegistersMap[i].Permission == 0)
    {
        Permission.sprintf("%s", "Inv");
    }
    else if(RegistersMap[i].Permission == 2)
    {
        Permission.sprintf("%s", "R\\W");
    }
    else if(RegistersMap[i].Permission == 3)
    {
        Permission.sprintf("%s", "W");
    }
    else
    {
        Permission.sprintf("%s", "R");
    }
#endif
    return Permission;
}

void RegisterWidget::dumpButtonClicked()
{
    if( registerSelectedNo >= 24)   /* No Module selected */
    {
        return;
    }

#if 0
    QString fileAppendix = QString( m_camDevice->isBitstream3D() ? ( m_main_notsub ? "-main" : "-sub" ) : "" ) + QString( "_register_dump.txt" );
    QString fileName = QDir( QDir::currentPath() ).filePath( xmlStringModuleName + fileAppendix );
            fileName = QFileDialog::getSaveFileName( this, tr("Select Dump File"), fileName, tr("TXT Files (*.txt)"), 0, QFileDialog::DontResolveSymlinks );

    switch(registerSelectedNo)
    {
        case 25:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_ISP_AWB, stringModuleName, fileName);
            break;
        case 24:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_ISP_WDR, stringModuleName, fileName);
            break;
        case 23:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_ISP_DPCC, stringModuleName, fileName);
            break;
        case 22:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_ISP_DPF, stringModuleName, fileName);
            break;
        case 21:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_ISP_BLS, stringModuleName, fileName);
            break;
        case 20:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_ISP_EXP, stringModuleName, fileName);
            break;
        case 19:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_ISP_CAC, stringModuleName, fileName);
            break;
        case 18:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_ISP_FILTER, stringModuleName, fileName);
            break;
        case 17:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_ISP_HIST, stringModuleName, fileName);
            break;
        case 16:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_ISP_IS, stringModuleName, fileName);
            break;
        case 15:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_ISP_LSC, stringModuleName, fileName);
            break;
        case 14:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_ISP_BP, stringModuleName, fileName);
            break;
        case 13:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_ISP_AFM, stringModuleName, fileName);
            break;
        case 12:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_MIPI, stringModuleName, fileName);
            break;
        case 11:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_SMIA, stringModuleName, fileName);
            break;
        case 10:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_JPE, stringModuleName, fileName);
            break;
        case 9:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_MI, stringModuleName, fileName);
            break;
        case 8:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_SRSZ, stringModuleName, fileName);
            break;
        case 7:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_MRSZ, stringModuleName, fileName);
            break;
        case 6:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_CPROC, stringModuleName, fileName);
            break;
        case 5:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_ISP_SHUTTER, stringModuleName, fileName);
            break;
        case 4:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_ISP_FLASH, stringModuleName, fileName);
            break;
        case 3:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_ISP_MAIN, stringModuleName, fileName);
            break;
        case 2:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_SUPER_IMPOSE, stringModuleName, fileName);
            break;
        case 1:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_IMG_EFFECTS, stringModuleName, fileName);
            break;
        case 0:
            retrieveRegisterValuesAndDump(CAMERIC_MODULE_MAIN_CONTROL, stringModuleName, fileName);
            break;
        default:
            break;
    }
#endif
}

void RegisterWidget::dumpAllButtonClicked()
{
#if 0
    QString fileAppendix = QString( m_camDevice->isBitstream3D() ? ( m_main_notsub ? "-main" : "-sub" ) : "" ) + QString( "_register_dump.txt" );
    QString fileName = QDir( QDir::currentPath() ).filePath( QString("CAMERIC") + fileAppendix );
            fileName = QFileDialog::getSaveFileName( this, tr("Select Dump File"), fileName, tr("TXT Files (*.txt)"), 0, QFileDialog::DontResolveSymlinks );

    retrieveAllRegisterValuesAndDump(fileName);
#endif
}

void RegisterWidget::retrieveRegisterValuesAndDump( const CamerIcModuleId_t moduleId, QString &moduleName, QString &fileName )
{
    FILE* pFile;
    pFile = fopen( fileName.toLatin1().constData(), "w" );
    if (!pFile)
    {
        printf( "Can't open file '%s'\n", fileName.toLatin1().constData() );
        return;
    }

    fprintf( pFile,"*************************************************************\n" );
    fprintf( pFile,"* CAMERIC REGISTERS (BASED @ 0x%08X)                    *\n", HAL_BASEADDR_MARVIN + m_addrOffset );
    fprintf( pFile,"*************************************************************\n" );
    fprintf( pFile," Module: %s\n", moduleName.toLatin1().constData() );
    fprintf( pFile,"*************************************************************\n" );

    dumpModuleRegisterValuesHelper( pFile, moduleId );

    fclose( pFile );
}

void RegisterWidget::retrieveAllRegisterValuesAndDump( QString &fileName )
{
    FILE* pFile;
    pFile = fopen( fileName.toLatin1().constData(), "w" );
    if (!pFile)
    {
        printf("Can't open file '%s'\n", fileName.toLatin1().constData());
        return;
    }

    fprintf( pFile,"*************************************************************\n" );
    fprintf( pFile,"* CAMERIC REGISTERS (BASED @ 0x%08X)                    *\n", HAL_BASEADDR_MARVIN + m_addrOffset );

    for(uint32_t module = 1; module < CAMERIC_MODULE_MAX; module++)
    {
        fprintf( pFile,"*************************************************************\n" );
        fprintf( pFile," Module: %d\n", module );
        fprintf( pFile,"*************************************************************\n" );

        dumpModuleRegisterValuesHelper( pFile, (CamerIcModuleId_t) module );
    }

    fclose(pFile);
}


void RegisterWidget::dumpModuleRegisterValuesHelper( FILE* pFile, CamerIcModuleId_t moduleId )
{
    uint32_t NumRegisters;
    RegDescription_t *RegistersMap;

    //RESULT result = CamerIcGetRegisterDescription(moduleId, &NumRegisters, &RegistersMap );
    RESULT result = RET_SUCCESS;
    if ( (result != RET_SUCCESS) || ( !NumRegisters ) || (!RegistersMap) )
    {
        fprintf(pFile," Module can not be accessed!\n");
    }
    else
    {
#if 0
        for (uint32_t i=0; i< NumRegisters; i++)
        {
            uint32_t registerValue;
            uint32_t registerAddress;
            uint32_t registerResetValue;
            Permissions_t registerPermission;
            RegName_t registerName;

            result = CamerIcGetRegister(nullptr, RegistersMap[i].Address + m_addrOffset, &registerValue );
            strcpy(registerName , RegistersMap[i].Name);
            registerAddress     = RegistersMap[i].Address;
            registerResetValue  = RegistersMap[i].ResetValue;
            registerPermission  = RegistersMap[i].Permission;

            fprintf(pFile, " %-30s @ 0x%04X", registerName, registerAddress);
            switch(registerPermission)
            {
                case PERM_READ_WRITE:
                case PERM_READ_ONLY :
                    if (result == RET_SUCCESS)
                    {
                        fprintf(pFile, " = 0x%08X", registerValue);
                        if (registerValue == registerResetValue)
                        {
                            fprintf(pFile, " (= default value)\n");
                        }
                        else
                        {
                            fprintf(pFile, " (default was 0x%08X)\n", registerResetValue);
                        }
                    }
                    else
                    {
                        fprintf(pFile, " <read failure %d>\n", result);
                    }
                    break;
                case PERM_WRITE_ONLY:
                    if (result == RET_SUCCESS)
                    {
                    fprintf(pFile, " <is only writable>\n");
                    }
                    else
                    {
                        fprintf(pFile, " <access failure %d>\n", result);
                    }
                    break;
                default:
                    fprintf(pFile, " <unknown access permission>\n");
            }
        }
#endif
    }
}
