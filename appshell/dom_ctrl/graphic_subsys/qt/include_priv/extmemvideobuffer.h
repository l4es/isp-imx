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
 * @file extmemvideobuffer.h
 *
 * @brief
 *   Definition of a QAbstractVideoBuffer implementation using existing buffer.
 *
 *****************************************************************************/

#include <QAbstractVideoBuffer>

class ExtMemVideoBuffer : public QAbstractVideoBuffer
{
public:
    ExtMemVideoBuffer( uchar* pData, int numBytes, int bytesPerLine);
    MapMode mapMode() const;
    uchar *map(MapMode mode, int *numBytes, int *bytesPerLine);
    void unmap();

private:
    uchar*  m_pData;
    int     m_numBytes;
    int     m_bytesPerLine;
    QAbstractVideoBuffer::MapMode m_mapMode;
};

