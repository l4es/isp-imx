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

#include "calib_calibration.hpp"
#include "assert.h"
#include "calib_features.hpp"
#include "exception.hpp"
#include <algorithm>
#include <iostream>

using namespace camdev;

Calibration::Calibration() {
  list.push_back(new CalibAe(document));
  list.push_back(new CalibAf(document));
  list.push_back(new CalibAvs(document));
  list.push_back(new CalibAwb(document));
  list.push_back(new CalibBls(document));
  list.push_back(new CalibCac(document));
  list.push_back(new CalibCnr(document));
  list.push_back(new CalibCproc(document));
  list.push_back(new CalibDemosaic(document));
  list.push_back(new CalibNr3d(document));
  list.push_back(new CalibDpcc(document));
  list.push_back(new CalibDpf(document));
  list.push_back(new CalibEe(document));
  list.push_back(new CalibGc(document));
  list.push_back(new CalibHdr(document));
  list.push_back(new CalibIe(document));
  list.push_back(new CalibImages(document));
  list.push_back(new CalibInputs(document));
  list.push_back(new CalibLsc(document));
  list.push_back(new CalibPaths(document));
  list.push_back(new CalibSensors(document));
  list.push_back(new CalibSimp(document));
  list.push_back(new CalibWb(document));
  list.push_back(new CalibWdr(document));
  list.push_back(new CalibDehaze(document));
  list.push_back(new CalibFilter(document));
  list.push_back(new CalibDewarp(document));
}

Calibration::~Calibration() {
  std::for_each(list.begin(), list.end(),
                [&](Element *pElement) { delete pElement; });
}

void Calibration::load(std::string fileName) {
  document.Clear();

  XMLError ret = document.LoadFile(fileName.c_str());
  if (ret != XML_SUCCESS) {
    throw exc::LogicError(RET_FAILURE, "Can't load XML file: " + fileName);
  }

  XMLElement *pRoot = document.FirstChildElement();
  if (!pRoot) {
    return;
  }

  XMLElement *pSubElement = pRoot->FirstChildElement(NAME_CLI);

  if (!pSubElement) {
    return;
  }

  pSubElement = pSubElement->FirstChildElement();

  while (pSubElement) {
    std::for_each(list.begin(), list.end(), [&](Element *pElement) {
      try {
        pElement->parse(*pSubElement);
      } catch (const std::exception &) {
      }
    });

    pSubElement = pSubElement->NextSiblingElement();
  }
}

void Calibration::store(std::string fileName) {
  XMLElement *pRoot = document.FirstChildElement();

  if (!pRoot) {
    document.InsertEndChild(pRoot = document.NewElement("matfile"));
  }

  XMLElement *pCliXmlElement = nullptr;

  pRoot->InsertEndChild(pCliXmlElement = document.NewElement(NAME_CLI));

  std::for_each(list.begin(), list.end(), [&](Element *pElement) {
    XMLElement *pSubElement =
        pCliXmlElement->FirstChildElement(pElement->name.c_str());
    if (!pSubElement) {
      pSubElement = document.NewElement(pElement->name.c_str());
      assert(pSubElement);

      pCliXmlElement->InsertEndChild(pSubElement);
    }

    pElement->compose(*pSubElement);
  });

  XMLError ret = document.SaveFile(fileName.c_str());
  if (ret != XML_SUCCESS) {
    std::cerr << "XML save file error: " << fileName << std::endl;
  }
}

Calibration *pCalibration;

