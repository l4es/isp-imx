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
 * @file extmemvideobuffer.cpp
 *
 * @brief
 *   Implementation of a QAbstractVideoBuffer using existing buffer.
 *
 *****************************************************************************/

#include "extmemvideobuffer.h"

ExtMemVideoBuffer::ExtMemVideoBuffer( uchar* pData, int numBytes, int bytesPerLine)
    : QAbstractVideoBuffer(NoHandle)
{
    m_pData         = pData;
    m_numBytes      = numBytes;
    m_bytesPerLine  = bytesPerLine;
    m_mapMode       = QAbstractVideoBuffer::NotMapped;
}

QAbstractVideoBuffer::MapMode ExtMemVideoBuffer::mapMode() const
{
    return m_mapMode;
}

uchar *ExtMemVideoBuffer::map(MapMode mode, int *numBytes, int *bytesPerLine)
{
    if ( (m_mapMode == NotMapped) && (m_pData != 0) && (mode != NotMapped) )
    {
        m_mapMode = mode;

        if (numBytes)
            *numBytes = m_numBytes;

        if (bytesPerLine)
            *bytesPerLine = m_bytesPerLine;

        return m_pData;
    } else {
        return 0;
    }
}

void ExtMemVideoBuffer::unmap()
{
    m_mapMode = QAbstractVideoBuffer::NotMapped;
}
