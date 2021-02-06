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

#include "level1/isiregmodel.h"


IsiRegModel::IsiRegModel( const IsiRegDescription_t* regDescription, QObject *parent )
  : QAbstractTableModel( parent ),
    m_regDescription( regDescription )
{ }

int
IsiRegModel::rowCount( const QModelIndex &parent ) const
{
    int rows = 0;

    if ( NULL != m_regDescription )
    {
        const IsiRegDescription_t* ptReg = m_regDescription;

        while (ptReg->Flags != eTableEnd)
        {
            ++ptReg;
            ++rows;
        }
    }

    return rows;
}

int
IsiRegModel::columnCount( const QModelIndex &parent ) const
{
    return 4;
}

QVariant
IsiRegModel::data( const QModelIndex &index, int role ) const
{
    if ( ( NULL == m_regDescription ) || ( true != index.isValid() ) )
    {
        return QVariant();
    }

    const IsiRegDescription_t* ptReg = m_regDescription;
    std::advance( ptReg, index.row() );

    if ( role == Qt::DisplayRole )
    {
        switch ( index.column() )
        {
            case 0://Address
            {
                return QString( "0x" ) +
                        QString( "%1" ).arg( ptReg->Addr, 4, 16, QLatin1Char( '0' ) ).toUpper();
            }

            case 1://Flags
            {
                return QString( "0b" ) +
                        QString( "%1" ).arg( ptReg->Flags, 8, 2, QLatin1Char( '0' ) ).toUpper();
            }

            case 2://Name
            {
                return ptReg->pName;
            }

            case 3://Default
            {
                return QString( "0x" ) +
                        QString( "%1" ).arg( ptReg->DefaultValue, 4, 16, QLatin1Char( '0' ) ).toUpper();
            }

            default:
                return QVariant();
        }
    }
    else if ( role == Qt::UserRole )
    {
        return ptReg->Addr;
    }

    return QVariant();

}

QVariant
IsiRegModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( role != Qt::DisplayRole )
    {
        return QVariant();
    }

    if ( orientation == Qt::Horizontal )
    {
        switch (section)
        {
            case 0:
                return tr("Address");

            case 1:
                return tr("R/W");

            case 2:
                return tr("Name");

            case 3:
                return tr("Default");

            default:
                return QVariant();
        }
    }
    return QVariant();
}

