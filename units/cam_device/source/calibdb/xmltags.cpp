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
/**
 * @file        xmltags.cpp
 *
 *****************************************************************************/

#include "xmltags.hpp"
#include "calibtags.hpp"
#include <string>

XmlTag::XmlTag(const XMLElement *pElement) : _pElement(pElement) {}

int32_t XmlTag::size() {
  const char *pAttr = _pElement->Attribute(CALIB_ATTRIBUTE_SIZE);

  int32_t col;
  int32_t row;

  int32_t res = sscanf(pAttr, CALIB_ATTRIBUTE_SIZE_FORMAT, &col, &row);
  if (res != CALIB_ATTRIBUTE_SIZE_NO_ELEMENTS) {
    return 0;
  }

  return col * row;
}

const char *XmlTag::value() { return _pElement->GetText(); }

const char *XmlTag::valueToUpper() {
  return _pElement->GetText();
  // TODO:
}

uint32_t XmlTag::valueToUInt() {
  uint32_t uintValue = 0;

  size_t index;

  uintValue = std::stoul(value(), &index, 16);
  if (!index) {
    uintValue = std::stoul(value());
  }

  return uintValue;
}

XmlTag::TagType_e XmlTag::type() {
  const char *pAttr = _pElement->Attribute(CALIB_ATTRIBUTE_TYPE);

  if (!strcmp(pAttr, CALIB_ATTRIBUTE_TYPE_CHAR)) {
    return (TAG_TYPE_CHAR);
  } else if (!strcmp(pAttr, CALIB_ATTRIBUTE_TYPE_DOUBLE)) {
    return (TAG_TYPE_DOUBLE);
  } else if (!strcmp(pAttr, CALIB_ATTRIBUTE_TYPE_STRUCT)) {
    return (TAG_TYPE_STRUCT);
  } else if (!strcmp(pAttr, CALIB_ATTRIBUTE_TYPE_CELL)) {
    return (TAG_TYPE_CELL);
  } else {
    return (TAG_TYPE_INVALID);
  }

  return TAG_TYPE_INVALID;
}

bool XmlTag::isType(const XmlTag::TagType_e type) {
  const char *pAttr = _pElement->Attribute(CALIB_ATTRIBUTE_TYPE);

  if (!strcmp(pAttr, CALIB_ATTRIBUTE_TYPE_CHAR)) {
    return ((bool)(TAG_TYPE_CHAR == type));
  } else if (!strcmp(pAttr, CALIB_ATTRIBUTE_TYPE_DOUBLE)) {
    return ((bool)(TAG_TYPE_DOUBLE == type));
  } else if (!strcmp(pAttr, CALIB_ATTRIBUTE_TYPE_STRUCT)) {
    return ((bool)(TAG_TYPE_STRUCT == type));
  } else if (!strcmp(pAttr, CALIB_ATTRIBUTE_TYPE_CELL)) {
    return ((bool)(TAG_TYPE_CELL == type));
  } else {
    return ((bool)(TAG_TYPE_INVALID == type));
  }

  return false;
}

XmlCellTag::XmlCellTag(const XMLElement *pE) : XmlTag(pE) {}

int32_t XmlCellTag::Index() {
  int32_t value = 0;

  const char *pAttr = _pElement->Attribute(CALIB_ATTRIBUTE_INDEX);
  if (pAttr) {
    value = atoi(pAttr);
  }

  return value;
}
