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

#include "calib_features.hpp"
#include "cam_common.hpp"
#include "json-app.hpp"
#include <algorithm>

using namespace camdev;

/********************************* AE ********************************************/
CalibAe::CalibAe(XMLDocument &document) : Element(document) { name = NAME_AE; }

void CalibAe::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibAe::composeSubElements(XMLElement &element) {
  subElementProc(element, AFPS, [&](XMLElement &subElement) {
    subElement.SetAttribute(AFPS, ecm.isAfps);
  });

  subElementProc(element, FLICKER_PERIOD, [&](XMLElement &subElement) {
    subElement.SetText(ecm.flickerPeriod);
  });

  subElementSet(element, DAMPING_OVER, config.dampingOver);
  subElementSet(element, DAMPING_UNDER, config.dampingUnder);
  subElementSet(element, SET_POINT, config.setPoint);
  subElementSet(element, TOLERANCE, config.tolerance);
}

void CalibAe::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibAe::parseSubElements(XMLElement &element) {
  subElementProc(element, AFPS, [&](XMLElement &subElement) {
    subElement.QueryBoolAttribute(AFPS, &ecm.isAfps);
  });

  subElementProc(element, FLICKER_PERIOD, [&](XMLElement &subElement) {
    subElement.QueryIntText((int32_t *)&ecm.flickerPeriod);
  });

  subElementGet(element, DAMPING_OVER, config.dampingOver);
  subElementGet(element, DAMPING_UNDER, config.dampingUnder);
  subElementGet(element, SET_POINT, config.setPoint);
  subElementGet(element, TOLERANCE, config.tolerance);
}



/********************************* AF ********************************************/

CalibAf::CalibAf(XMLDocument &document) : Element(document) { name = NAME_AF; }

void CalibAf::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibAf::composeSubElements(XMLElement &element) {
  subElementProc(element, ALGORITHM, [&](XMLElement &subElement) {
    subElement.SetText(config.searchAlgorithm);
    subElement.SetAttribute(ONESHOT, config.isOneshot);
  });
}

void CalibAf::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibAf::parseSubElements(XMLElement &element) {
  subElementProc(element, ALGORITHM, [&](XMLElement &subElement) {
    subElement.QueryIntText((int32_t *)(&config.searchAlgorithm));
    subElement.QueryBoolAttribute(ONESHOT, &config.isOneshot);
  });
}


/********************************* AVS ********************************************/
CalibAvs::CalibAvs(XMLDocument &document) : Element(document) {
  name = NAME_AVS;
  config.reset();
}

void CalibAvs::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
  element.SetAttribute(USE_PARAMS, config.isUseParams);
}

void CalibAvs::composeSubElements(XMLElement &element) {
  subElementSet(element, DAMP_NUMBER_OF_INTERPOLATION_POINTS,
                config.numItpPoints);

  subElementSet(element, DAMP_ACCELERATION, config.acceleration);
  subElementSet(element, DAMP_BASE_GAIN, config.baseGain);
  subElementSet(element, DAMP_FALL_OFF, config.fallOff);
  subElementSet(element, DAMP_THETA, config.theta);
}

void CalibAvs::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
  element.QueryBoolAttribute(USE_PARAMS, &config.isUseParams);
}

void CalibAvs::parseSubElements(XMLElement &element) {
  subElementGet(element, DAMP_NUMBER_OF_INTERPOLATION_POINTS,
                config.numItpPoints);
  subElementGet(element, DAMP_ACCELERATION, config.acceleration);
  subElementGet(element, DAMP_BASE_GAIN, config.baseGain);
  subElementGet(element, DAMP_FALL_OFF, config.fallOff);
  subElementGet(element, DAMP_THETA, config.theta);
}

/********************************* AWB ********************************************/

CalibAwb::CalibAwb(XMLDocument &document) : Element(document) { name = NAME_AWB; }

void CalibAwb::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibAwb::composeSubElements(XMLElement &element) {
  subElementProc(element, DAMPING, [&](XMLElement &subElement) {
    subElement.SetText(config.isDamping);
  });

  subElementProc(element, INDEX, [&](XMLElement &subElement) {
    subElement.SetText(config.index);
  });

  subElementProc(element, MODE, [&](XMLElement &subElement) {
    subElement.SetText(config.mode);
  });
}

void CalibAwb::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibAwb::parseSubElements(XMLElement &element) {
  subElementProc(element, DAMPING, [&](XMLElement &subElement) {
    subElement.QueryBoolText(&config.isDamping);
  });

  subElementProc(element, INDEX, [&](XMLElement &subElement) {
    subElement.QueryUnsignedText(&config.index);
  });

  subElementProc(element, MODE, [&](XMLElement &subElement) {
    subElement.QueryIntText((int32_t *)(&config.mode));
  });
}

/********************************* BLS ********************************************/

CalibBls::CalibBls(XMLDocument &document) : Element(document) { name = NAME_BLS; }

void CalibBls::composeSubElements(XMLElement &element) {
  subElementSet(element, KEY_RED, config.red);
  subElementSet(element, KEY_GREEN_B, config.greenB);
  subElementSet(element, KEY_GREEN_R, config.greenR);
  subElementSet(element, KEY_BLUE, config.blue);
}

void CalibBls::parseSubElements(XMLElement &element) {
  subElementGet(element, KEY_RED, config.red);
  subElementGet(element, KEY_GREEN_B, config.greenB);
  subElementGet(element, KEY_GREEN_R, config.greenR);
  subElementGet(element, KEY_BLUE, config.blue);
}

/********************************* CAC ********************************************/

CalibCac::CalibCac(XMLDocument &document) : Element(document) {
  name = NAME_CAC;

  REFSET(config, 0);
}

void CalibCac::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibCac::composeSubElements(XMLElement &element) {
  subElementProc(element, CONFIG, [&](XMLElement &subElement) {
    std::string stringData =
        base64_encode((unsigned char const *)&config, sizeof(config));

    subElement.SetText(stringData.c_str());
  });
}

void CalibCac::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibCac::parseSubElements(XMLElement &element) {
  subElementProc(element, CONFIG, [&](XMLElement &subElement) {
    std::string decodedString = base64_decode(subElement.GetText());

    std::copy(decodedString.begin(), decodedString.end(), (char *)&config);
  });
}

/********************************* CNR ********************************************/
CalibCnr::CalibCnr(XMLDocument &document) : Element(document) { name = NAME_CNR; }

void CalibCnr::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibCnr::composeSubElements(XMLElement &element) {
  subElementSet(element, THRESHOLD_CHANNEL ".1", config.tc1);
  subElementSet(element, THRESHOLD_CHANNEL ".2", config.tc2);
}

void CalibCnr::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibCnr::parseSubElements(XMLElement &element) {
  subElementGet(element, THRESHOLD_CHANNEL ".1", config.tc1);
  subElementGet(element, THRESHOLD_CHANNEL ".2", config.tc2);
}

/********************************* CPROC ********************************************/
CalibCproc::CalibCproc(XMLDocument &document) : Element(document) {
  name = NAME_CPROC;
}

void CalibCproc::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibCproc::composeSubElements(XMLElement &element) {
  subElementProc(element, CONFIG, [&](XMLElement &subElement) {
    subElement.SetText(
        base64_encode((unsigned char const *)&config, sizeof(config)).c_str());
  });
}

void CalibCproc::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibCproc::parseSubElements(XMLElement &element) {
  subElementProc(element, CONFIG, [&](XMLElement &subElement) {
    std::string decodedString = base64_decode(subElement.GetText());

    std::copy(decodedString.begin(), decodedString.end(), (char *)&config);
  });
}

/********************************* DEMOSAIC ********************************************/
CalibDemosaic::CalibDemosaic(XMLDocument &document) : Element(document) {
  name = NAME_DMS;
}

void CalibDemosaic::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}
void CalibDemosaic::composeSubElements(XMLElement &element) {
  subElementSet(element, MODE, config.mode);
  subElementSet(element, THRESHOLD, config.threshold);
}

void CalibDemosaic::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibDemosaic::parseSubElements(XMLElement &element) {
  int32_t mode = 0;

  subElementGet(element, MODE, mode);
  config.mode = static_cast<Config::Mode>(mode);

  subElementGet(element, THRESHOLD, config.threshold);
}

/********************************* DPCC ********************************************/
CalibDpcc::CalibDpcc(XMLDocument &document) : Element(document) { name = NAME_DPCC; }

void CalibDpcc::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibDpcc::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}


/********************************* DPF ********************************************/
CalibDpf::CalibDpf(XMLDocument &document) : Element(document) { name = NAME_DPF; }

void CalibDpf::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibDpf::composeSubElements(XMLElement &element) {
  subElementSet(element, GRADIENT, config.gradient);
  subElementSet(element, OFFSET, config.offset);
  subElementSet(element, MINIMUM_BOUND, config.minimumBound);
  subElementSet(element, DIVISION_FACTOR, config.divisionFactor);

  subElementSet(element, SIGMA_GREEN, config.sigmaGreen);
  subElementSet(element, SIGMA_RED_BLUE, config.sigmaRedBlue);
}

void CalibDpf::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibDpf::parseSubElements(XMLElement &element) {
  element.QueryBoolAttribute(ADAPTIVE, &config.isAdaptive);

  subElementGet(element, GRADIENT, config.gradient);
  subElementGet(element, OFFSET, config.offset);
  subElementGet(element, MINIMUM_BOUND, config.minimumBound);
  subElementGet(element, DIVISION_FACTOR, config.divisionFactor);
  subElementGet(element, SIGMA_GREEN, config.sigmaGreen);
  subElementGet(element, SIGMA_RED_BLUE, config.sigmaRedBlue);
}

/********************************* EC ********************************************/
CalibEc::CalibEc(XMLDocument &document) : Element(document) {}

void CalibEc::composeSubElements(XMLElement &element) {
  subElementSet(element, GAIN, config.gain);
  subElementSet(element, HDR_RATIO, config.hdrRatio);
  subElementSet(element, INTERGRATION_TIME, config.integrationTime);
}

void CalibEc::parseSubElements(XMLElement &element) {
  subElementGet(element, GAIN, config.gain);
  subElementGet(element, HDR_RATIO, config.hdrRatio);
  subElementGet(element, INTERGRATION_TIME, config.integrationTime);
}


/********************************* EE ********************************************/
CalibEe::CalibEe(XMLDocument &document) : Element(document) { name = NAME_EE; }

void CalibEe::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibEe::composeSubElements(XMLElement &xmLElement) {
  xmLElement.SetAttribute(AUTO, config.isAuto);

  subElementSet(xmLElement, EDGE_GAIN, config.config.edgeGain);
  subElementSet(xmLElement, STRENGTH, config.config.strength);
  subElementSet(xmLElement, UV_GAIN, config.config.uvGain);
  subElementSet(xmLElement, Y_GAIN_DOWN, config.config.yDownGain);
  subElementSet(xmLElement, Y_GAIN_UP, config.config.yUpGain);

  subElementSetJson(xmLElement, TABLE, table.jTable);
}

void CalibEe::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibEe::parseSubElements(XMLElement &xmLElement) {
  xmLElement.QueryBoolAttribute(AUTO, &config.isAuto);

  subElementGet(xmLElement, EDGE_GAIN, config.config.edgeGain);
  subElementGet(xmLElement, STRENGTH, config.config.strength);
  subElementGet(xmLElement, UV_GAIN, config.config.uvGain);
  subElementGet(xmLElement, Y_GAIN_DOWN, config.config.yDownGain);
  subElementGet(xmLElement, Y_GAIN_UP, config.config.yUpGain);

  subElementGet(xmLElement, TABLE, table.jTable);
}

void CalibEe::Config::reset() {
  isAuto = true;

  config.edgeGain = 1800;
  config.strength = 100;
  config.uvGain = 512;
  config.yDownGain = 10000;
  config.yUpGain = 10000;
}

void CalibEe::Table::reset() {
#if 0 // need to do
  std::string data = "{ \"columns\": [\"HDR\", \"Gain\", \"Integration Time\", "
                     "\"Edge Gain\", \"Strength\", \"UV Gain\", \"Y Gain "
                     "Down\", \"Y Gain Up\"], "
                     "\"rows\": [] }";

  jTable = JsonApp::fromString(data);
#endif
}

/********************************* GC ********************************************/
CalibGc::CalibGc(XMLDocument &document) : Element(document) {
  name = NAME_GC;

  config.curve = standardCurve;
}

void CalibGc::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibGc::composeSubElements(XMLElement &element) {
  subElementProc(element, CURVE, [&](XMLElement &subElement) {
    subElement.SetText(base64_encode((unsigned char const *)&config.curve,
                                        sizeof(CamEngineGammaOutCurve_t))
                              .c_str());
  });
}

void CalibGc::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibGc::parseSubElements(XMLElement &element) {
  subElementProc(element, CURVE, [&](XMLElement &subElement) {
    std::string decodedString = base64_decode(subElement.GetText());

    std::copy(decodedString.begin(), decodedString.end(),
              (char *)&config.curve);
  });
}


/********************************* HDR ********************************************/
CalibHdr::CalibHdr(XMLDocument &document) : Element(document) { name = NAME_HDR; }

void CalibHdr::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibHdr::composeSubElements(XMLElement &element) {
  subElementSet(element, EXPOSURE_RATIO, config.exposureRatio);
  subElementSet(element, EXTENSION_BIT, config.extensionBit);
}

void CalibHdr::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibHdr::parseSubElements(XMLElement &element) {
  subElementGet(element, EXPOSURE_RATIO, config.exposureRatio);
  subElementGet(element, EXTENSION_BIT, config.extensionBit);
}

/********************************* IE ********************************************/
CalibIe::CalibIe(XMLDocument &document) : Element(document) { name = NAME_IE; }

void CalibIe::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibIe::composeSubElements(XMLElement &element) {
  subElementSet(element, MODE, config.config.mode);
  subElementSet(element, RANGE, config.config.range);

  subElementProc(element, MODE_CONFIG, [&](XMLElement &subElement) {
    std::string stringData =
        base64_encode((unsigned char const *)&config.config.ModeConfig,
                      sizeof(config.config.ModeConfig));

    subElement.SetText(stringData.c_str());
  });
}

void CalibIe::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibIe::parseSubElements(XMLElement &element) {
  subElementGet(element, MODE, (int32_t &)config.config.mode);
  subElementGet(element, RANGE, (int32_t &)config.config.range);

  subElementProc(element, MODE_CONFIG, [&](XMLElement &subElement) {
    std::string decodedString = base64_decode(subElement.GetText());

    std::copy(decodedString.begin(), decodedString.end(),
              (char *)&config.config.ModeConfig);
  });
}

/********************************* Image and Images ********************************************/
void CalibImage::composeSubElements(XMLElement &element) {
  subElementProc(element, "file",  [&](XMLElement &subElement) {
    subElement.SetText(config.fileName.c_str());
  });
}

void CalibImage::parseSubElements(XMLElement &element) {
  subElementProc(element, "file", [&](XMLElement &subElement) {
    if (subElement.GetText()) {
      config.fileName = subElement.GetText();
    }
  });
}

CalibImages::CalibImages(XMLDocument &document) : Element(document) {
  name = NAME_IMAGES;

  for (int32_t i = 0; i < ISP_INPUT_MAX; i++) {
    images.emplace_back(document);
  }
}

void CalibImages::composeSubElements(XMLElement &element) {
  element.DeleteChildren();

  std::for_each(images.begin(), images.end(), [&](CalibImage &image) {
    XMLElement *pSubElement = nullptr;

    element.InsertEndChild(pSubElement = document.NewElement(IMAGE));

    image.composeSubElements(*pSubElement);
  });
}

void CalibImages::parseSubElements(XMLElement &element) {
  auto pSubElement = element.FirstChildElement(IMAGE);

  int32_t i = 0;

  while (pSubElement) {
    images[i++].parseSubElements(*pSubElement);

    pSubElement = pSubElement->NextSiblingElement();
  }
}

/********************************* Input ********************************************/
void CalibInput::composeSubElements(XMLElement &element) {
  subElementProc(element, TYPE, [&](XMLElement &subElement) {
    subElement.SetText(config.type);
  });
}

void CalibInput::parseSubElements(XMLElement &element) {
  subElementProc(element, TYPE, [&](XMLElement &subElement) {
    subElement.QueryIntText(reinterpret_cast<int32_t *>(&config.type));
  });
}

CalibInputs::CalibInputs(XMLDocument &document) : Element(document) {
  name = NAME_INPUTS;

  for (int32_t i = 0; i < ISP_INPUT_MAX; i++) {
    inputs.emplace_back(document);
  }
}

void CalibInputs::composeSubElements(XMLElement &element) {
  element.DeleteChildren();

  std::for_each(inputs.begin(), inputs.end(), [&](CalibInput &input) {
    XMLElement *pSubElement = nullptr;

    element.InsertEndChild(pSubElement = document.NewElement(INPUT));

    input.composeSubElements(*pSubElement);
  });

  subElementProc(element, INDEX, [&](XMLElement &subElement) {
    subElement.SetText(config.index);
  });
}

void CalibInputs::parseSubElements(XMLElement &element) {
  auto pSubElement = element.FirstChildElement(INPUT);

  int32_t i = 0;

  while (pSubElement && i < static_cast<int>(inputs.size())) {
    inputs[i++].parseSubElements(*pSubElement);

    pSubElement = pSubElement->NextSiblingElement();
  }

  subElementProc(element, INDEX, [&](XMLElement &subElement) {
    subElement.QueryIntText(reinterpret_cast<int32_t *>(&config.index));
  });
}



/********************************* JPE ********************************************/
CalibJpe::CalibJpe(XMLDocument &document) : Element(document) { name = NAME_JPE; }

void CalibJpe::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibJpe::composeSubElements(XMLElement &element) {
  subElementSet(element, WIDTH, config.width);
  subElementSet(element, HEIGHT, config.height);
}

void CalibJpe::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibJpe::parseSubElements(XMLElement &element) {
  subElementGet(element, WIDTH, config.width);
  subElementGet(element, HEIGHT, config.height);
}


/********************************* LSC ********************************************/
CalibLsc::CalibLsc(XMLDocument &document) : Element(document) { name = NAME_LSC; }

void CalibLsc::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibLsc::composeSubElements(XMLElement &element) {
  element.SetAttribute(ADAPTIVE, config.isAdaptive);
}
void CalibLsc::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibLsc::parseSubElements(XMLElement &element) {
  element.QueryBoolAttribute(ADAPTIVE, &config.isAdaptive);
}


/********************************* NR2D ********************************************/
CalibNr2d::CalibNr2d(XMLDocument &document) : Element(document) {
  name = NAME_DNR2;

  for (int32_t i = 0; i < Nr2DSum; i++) {
    holders.emplace_back();
  }

  holders[Nr2Dv1].config.v1.reset();
  holders[Nr2Dv1].table.reset(Nr2Dv1);

  holders[Nr2Dv2].config.v2.reset();
  holders[Nr2Dv2].table.reset(Nr2Dv2);

  holders[Nr2Dv3].config.v3.reset();
  holders[Nr2Dv3].table.reset(Nr2Dv3);
}

void CalibNr2d::composeSubElements(XMLElement &element) {
  subElementProc(element, V1, [&](XMLElement &subElement) {
    subElement.SetAttribute(ENABLE, holders[Nr2Dv1].isEnable);

    subElement.SetAttribute(AUTO, holders[Nr2Dv1].config.v1.isAuto);

    subElementSet(subElement, DENOISE_PREGAMA_STRENGTH,
                  holders[Nr2Dv1].config.v1.denoisePregamaStrength);
    subElementSet(subElement, DENOISE_STRENGTH,
                  holders[Nr2Dv1].config.v1.denoiseStrength);
    subElementSet(subElement, SIGMA, holders[Nr2Dv1].config.v1.sigma);

    subElementSetJson(subElement, TABLE, holders[Nr2Dv1].table.jTable);
  });
}

void CalibNr2d::parseSubElements(XMLElement &element) {
  subElementProc(element, V1, [&](XMLElement &subElement) {
    subElement.QueryBoolAttribute(ENABLE, &holders[Nr2Dv1].isEnable);

    subElement.QueryBoolAttribute(AUTO, &holders[Nr2Dv1].config.v1.isAuto);

    subElementGet(subElement, DENOISE_PREGAMA_STRENGTH,
                  holders[Nr2Dv1].config.v1.denoisePregamaStrength);
    subElementGet(subElement, DENOISE_STRENGTH,
                  holders[Nr2Dv1].config.v1.denoiseStrength);
    subElementGet(subElement, SIGMA, holders[Nr2Dv1].config.v1.sigma);

    subElementGet(subElement, TABLE, holders[Nr2Dv1].table.jTable);
  });
}

void CalibNr2d::Config::Nr2Dv1::reset() {
  isAuto = true;

  denoisePregamaStrength = 1;
  denoiseStrength = 80;
  sigma = 2.0;
}

void CalibNr2d::Config::Nr2Dv2::reset() {}

void CalibNr2d::Config::Nr2Dv3::reset() {}

void CalibNr2d::Table::reset(Generation generation) {
  std::string data;
#if 0 //need to do
  if (generation == Nr2Dv1) {
    data = "{ \"columns\": [ \"HDR\", \"Gain\", \"Integration Time\", "
           "\"Pre-gamma Strength\", \"Denoise Strength\", \"Sigma\"], "
           "\"rows\": [] }";
  } else if (generation == Nr2Dv2) {

  } else if (generation == Nr2Dv3) {
  }

  jTable = JsonApp::fromString(data);
#endif
}
/********************************* NR3D ********************************************/
CalibNr3d::CalibNr3d(XMLDocument &document) : Element(document) {
  name = NAME_DNR3;

  for (int32_t i = 0; i < Nr3DSum; i++) {
    holders.emplace_back();
  }

  holders[Nr3Dv1].config.v1.reset();
  holders[Nr3Dv1].table.reset(Nr3Dv1);

  holders[Nr3Dv2].config.v2.reset();
  holders[Nr3Dv2].table.reset(Nr3Dv2);

  holders[Nr3Dv3].config.v3.reset();
  holders[Nr3Dv3].table.reset(Nr3Dv3);
}

void CalibNr3d::composeSubElements(XMLElement &element) {
  subElementProc(element, V1, [&](XMLElement &subElement) {
    subElement.SetAttribute(ENABLE, holders[Nr3Dv1].isEnable);

    subElement.SetAttribute(AUTO, holders[Nr3Dv1].config.v1.isAuto);

    subElementSet(subElement, DELTA_FACTOR,
                  holders[Nr3Dv1].config.v1.deltaFactor);
    subElementSet(subElement, MOTION_FACTOR,
                  holders[Nr3Dv1].config.v1.motionFactor);
    subElementSet(subElement, STRENGTH, holders[Nr3Dv1].config.v1.strength);

    subElementSetJson(subElement, TABLE, holders[Nr3Dv1].table.jTable);
  });
}

void CalibNr3d::parseSubElements(XMLElement &element) {
  subElementProc(element, V1, [&](XMLElement &subElement) {
    subElement.QueryBoolAttribute(ENABLE, &holders[Nr3Dv1].isEnable);

    subElement.QueryBoolAttribute(AUTO, &holders[Nr3Dv1].config.v1.isAuto);

    subElementGet(subElement, DELTA_FACTOR,
                  holders[Nr3Dv1].config.v1.deltaFactor);
    subElementGet(subElement, MOTION_FACTOR,
                  holders[Nr3Dv1].config.v1.motionFactor);
    subElementGet(subElement, STRENGTH, holders[Nr3Dv1].config.v1.strength);

    subElementGet(subElement, TABLE, holders[Nr3Dv1].table.jTable);
  });
}

void CalibNr3d::Config::Nr3Dv1::reset() {
  isAuto = true;

  deltaFactor = 32;
  motionFactor = 1024;
  strength = 100;
}

void CalibNr3d::Config::Nr3Dv2::reset() {}

void CalibNr3d::Config::Nr3Dv3::reset() {}

void CalibNr3d::Table::reset(Generation generation) {
  std::string data;
#if 0 //need to do
  if (generation == Nr3Dv1) {
    data = "{ \"columns\": [\"HDR\", \"Gain\", \"Integration Time\", "
           "\"Strength\", \"Motion Factor\", "
           "\"Delta Factor\"], "
           "\"rows\": [] }";
  } else if (generation == Nr3Dv2) {

  } else if (generation == Nr3Dv3) {
  }

  jTable = JsonApp::fromString(data);
#endif
}


/********************************* PATH ********************************************/
static const CamEnginePathConfig_t defaultSpConfig = {
    0U, 0U, CAMERIC_MI_DATAMODE_DISABLED, CAMERIC_MI_DATASTORAGE_SEMIPLANAR};

static const CamEnginePathConfig_t defaultSp2Config = {
    0U, 0U, CAMERIC_MI_DATAMODE_DISABLED, CAMERIC_MI_DATASTORAGE_SEMIPLANAR};

static const CamEnginePathConfig_t defaultRdiConfig = {
    0U, 0U, CAMERIC_MI_DATAMODE_DISABLED, CAMERIC_MI_DATASTORAGE_INTERLEAVED};

static const CamEnginePathConfig_t defaultMetaConfig = {
    0U, 0U, CAMERIC_MI_DATAMODE_DISABLED, CAMERIC_MI_DATASTORAGE_INTERLEAVED};

static const CamEnginePathConfig_t defaultMpConfig = {
    1280U, 720U, CAMERIC_MI_DATAMODE_YUV422,
    CAMERIC_MI_DATASTORAGE_SEMIPLANAR};

using namespace camdev;

CalibPaths::CalibPaths(XMLDocument &document) : Element(document) {
  name = NAME_PATHS;

  reset();
}

void CalibPaths::composeSubElements(XMLElement &element) {
  element.DeleteChildren();

  for (int32_t i = 0; i < CAMERIC_MI_PATH_MAX; i++) {
    XMLElement *pSubElement = nullptr;

    element.InsertEndChild(pSubElement = document.NewElement(PATH));

    pSubElement->SetAttribute(INDEX, i);

    pathComposeSubElements(*pSubElement, config.config[i]);
  }
}

void CalibPaths::parseSubElements(XMLElement &element) {
  auto pSubElement = element.FirstChildElement(PATH);

  int32_t i = 0;

  while (pSubElement && i < CAMERIC_MI_PATH_MAX) {
    int index = 0;

    pSubElement->QueryIntAttribute(INDEX, &index);

    pathParseSubElements(*pSubElement, config.config[index]);

    pSubElement = pSubElement->NextSiblingElement();
  }
}

void CalibPaths::pathComposeSubElements(XMLElement &element,
                                   CamEnginePathConfig_t &config) {
  subElementProc(element, HEIGHT, [&](XMLElement &subElement) {
    subElement.SetText(config.height);
  });

  subElementProc(element, LAYOUT, [&](XMLElement &subElement) {
    subElement.SetText((int32_t)config.layout);
  });

  subElementProc(element, MODE, [&](XMLElement &subElement) {
    subElement.SetText((int32_t)config.mode);
  });

  subElementProc(element, WIDTH, [&](XMLElement &subElement) {
    subElement.SetText(config.width);
  });
}

void CalibPaths::pathParseSubElements(XMLElement &element,
                                 CamEnginePathConfig_t &config) {
  subElementProc(element, HEIGHT, [&](XMLElement &subElement) {
    subElement.QueryUnsignedText(reinterpret_cast<uint32_t *>(&config.height));
  });

  subElementProc(element, LAYOUT, [&](XMLElement &subElement) {
    subElement.QueryIntText(reinterpret_cast<int32_t *>(&config.layout));
  });

  subElementProc(element, MODE, [&](XMLElement &subElement) {
    subElement.QueryIntText(reinterpret_cast<int32_t *>(&config.mode));
  });

  subElementProc(element, WIDTH, [&](XMLElement &subElement) {
    subElement.QueryUnsignedText(reinterpret_cast<uint32_t *>(&config.width));
  });
}

void CalibPaths::reset() {
  config.config[CAMERIC_MI_PATH_MAIN] = defaultMpConfig;
  config.config[CAMERIC_MI_PATH_SELF] = defaultSpConfig;
  config.config[CAMERIC_MI_PATH_SELF2_BP] = defaultSp2Config;
  config.config[CAMERIC_MI_PATH_RDI] = defaultRdiConfig;
  config.config[CAMERIC_MI_PATH_META] = defaultMetaConfig;
}


/********************************* SENSOR ********************************************/

void CalibSensor::composeSubElements(XMLElement &element) {
  subElementProc(element, CALIB_FILE, [&](XMLElement &subElement) {
    subElement.SetText(config.calibFileName.c_str());
  });

  subElementProc(element, DRV_FILE, [&](XMLElement &subElement) {
    subElement.SetText(config.driverFileName.c_str());
  });

  subElementProc(element, EC, [&](XMLElement &subElement) {
    config.ec.composeSubElements(subElement);
  });

  subElementProc(element, TEST_PATTERN, [&](XMLElement &subElement) {
    subElement.SetAttribute(ENABLE, config.isTestPattern);
  });
}

void CalibSensor::parseSubElements(XMLElement &element) {
  subElementProc(element, CALIB_FILE, [&](XMLElement &subElement) {
    if (subElement.GetText()) {
      config.calibFileName = subElement.GetText();
    }
  });

  subElementProc(element, DRV_FILE, [&](XMLElement &subElement) {
    if (subElement.GetText()) {
      config.driverFileName = subElement.GetText();
    }
  });

  subElementProc(element, EC, [&](XMLElement &subElement) {
    config.ec.parseSubElements(subElement);
  });

  subElementProc(element, TEST_PATTERN, [&](XMLElement &subElement) {
    subElement.QueryBoolAttribute(ENABLE, &config.isTestPattern);
  });
}

CalibSensors::CalibSensors(XMLDocument &document) : Element(document) {
  name = NAME_SENSORS;

  for (int32_t i = 0; i < ISP_INPUT_MAX; i++) {
    sensors.emplace_back(document);
  }
}

void CalibSensors::composeSubElements(XMLElement &element) {
  element.DeleteChildren();

  std::for_each(sensors.begin(), sensors.end(), [&](CalibSensor &sensor) {
    XMLElement *pSubElement = nullptr;

    element.InsertEndChild(pSubElement = document.NewElement(SENSOR));

    sensor.composeSubElements(*pSubElement);
  });
}

void CalibSensors::parseSubElements(XMLElement &element) {
  auto pSubElement = element.FirstChildElement(SENSOR);

  int32_t i = 0;

  while (pSubElement && i < static_cast<int>(sensors.size())) {
    sensors[i++].parseSubElements(*pSubElement);

    pSubElement = pSubElement->NextSiblingElement();
  }
}



/********************************* SIMP ********************************************/
CalibSimp::CalibSimp(XMLDocument &document) : Element(document) {
  name = NAME_SIMP;
}

void CalibSimp::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibSimp::composeSubElements(XMLElement &element) {}

void CalibSimp::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibSimp::parseSubElements(XMLElement &element) {}




/********************************* WB ********************************************/
 CalibWb::CalibWb(XMLDocument &document) : Element(document) { name = NAME_WB; }

void  CalibWb::composeSubElements(XMLElement &element) {
  Json::Value jCcMatrix;

  for (int32_t i = 0; i < 9; i++) {
    jCcMatrix.append(config.ccMatrix.Coeff[i]);
  }

  subElementSetJson(element, CC_MATRIX, jCcMatrix);

  subElementProc(element, CC_OFFSET, [&](XMLElement &subElement) {
    subElementSet(subElement, BLUE, config.ccOffset.Blue);
    subElementSet(subElement, GREEN, config.ccOffset.Green);
    subElementSet(subElement, RED, config.ccOffset.Red);
  });

  subElementProc(element, WB_GAINS, [&](XMLElement &subElement) {
    subElementSet(subElement, BLUE, config.wbGains.Blue);
    subElementSet(subElement, GREEN_B, config.wbGains.GreenB);
    subElementSet(subElement, GREEN_R, config.wbGains.GreenR);
    subElementSet(subElement, RED, config.wbGains.Red);
  });
}

void  CalibWb::parseSubElements(XMLElement &element) {
  Json::Value jCcMatrix;

  subElementGet(element, CC_MATRIX, jCcMatrix);

  for (int32_t i = 0; i < 9; i++) {
    config.ccMatrix.Coeff[i] = static_cast<uint16_t>(jCcMatrix[i].asUInt());
  }

  subElementProc(element, CC_OFFSET, [&](XMLElement &subElement) {
    subElementGet(subElement, BLUE, config.ccOffset.Blue);
    subElementGet(subElement, GREEN, config.ccOffset.Green);
    subElementGet(subElement, RED, config.ccOffset.Red);
  });

  subElementProc(element, WB_GAINS, [&](XMLElement &subElement) {
    subElementGet(subElement, BLUE, config.wbGains.Blue);
    subElementGet(subElement, GREEN_B, config.wbGains.GreenB);
    subElementGet(subElement, GREEN_R, config.wbGains.GreenR);
    subElementGet(subElement, RED, config.wbGains.Red);
  });
}


/********************************* WDR ********************************************/

static const CamEngineWdrCurve_t curveX0 = {
    // .Ym =
    {0x0,   0x7c,  0xf8,  0x174, 0x1f0, 0x26c, 0x2e8, 0x364, 0x3e0,
     0x45d, 0x4d9, 0x555, 0x5d1, 0x64d, 0x6c9, 0x745, 0x7c1, 0x83e,
     0x8ba, 0x936, 0x9b2, 0xa2e, 0xaaa, 0xb26, 0xba2, 0xc1f, 0xc9b,
     0xd17, 0xd93, 0xe0f, 0xe8b, 0xf07, 0xf83},
    // .dY =
    {0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
     4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}};

CalibWdr::CalibWdr(XMLDocument &document) : Element(document) {
  name = NAME_WDR;

  for (int32_t i = 0; i < WdrMax; i++) {
    holders.emplace_back();
  }

  holders[Wdr1].config.wdr1.reset();
  holders[Wdr1].table.reset(Wdr1);

  holders[Wdr2].config.wdr2.reset();
  holders[Wdr2].table.reset(Wdr2);

  holders[Wdr3].config.wdr3.reset();
  holders[Wdr3].table.reset(Wdr3);
  holders[Wdr1].config.wdr1.curve = curveX0;

//  holders[Wdr3].config.wdr3.strength = WDR3_DEFAULT_STRENGTH;
//  holders[Wdr3].config.wdr3.gainMax = WDR3_DEFAULT_GAIN_MAX;
//  holders[Wdr3].config.wdr3.strengthGlobal = WDR3_DEFAULT_STRENGTH_GLOBAL;
}

void CalibWdr::composeSubElements(XMLElement &element) {
  subElementProc(element, V1, [&](XMLElement &subElement) {
    subElement.SetAttribute(ENABLE, holders[Wdr1].isEnable);

    CamEngineWdrCurve_t &curve = holders[Wdr1].config.wdr1.curve;

    Json::Value jDy;
    Json::Value jYm;

    for (int32_t i = 0; i < WDR_CURVE_ARRAY_SIZE; i++) {
      jDy.append(curve.dY[i]);
    }

    subElementSetJson(subElement, DY, jDy);

    for (int32_t i = 0; i < WDR_CURVE_ARRAY_SIZE; i++) {
      jYm.append(curve.Ym[i]);
    }

    subElementSetJson(subElement, YM, jYm);
  });

  subElementProc(element, V2, [&](XMLElement &subElement) {
    subElement.SetAttribute(ENABLE, holders[Wdr2].isEnable);

    Config::Wdr2 &wdr2 = holders[Wdr2].config.wdr2;

    subElementSet(subElement, STRENGTH, wdr2.strength);
  });

  subElementProc(element, V3, [&](XMLElement &subElement) {
    subElement.SetAttribute(ENABLE, holders[Wdr3].isEnable);

    subElement.SetAttribute(AUTO, holders[Wdr3].config.wdr3.isAuto);

    Config::Wdr3 &wdr3 = holders[Wdr3].config.wdr3;

    subElementSet(subElement, MAX_GAIN, wdr3.gainMax);
    subElementSet(subElement, STRENGTH, wdr3.strength);
    subElementSet(subElement, STRENGTH_GLOBAL, wdr3.strengthGlobal);

    subElementSetJson(subElement, TABLE, holders[Wdr3].table.jTable);
  });
}

void CalibWdr::parseSubElements(XMLElement &element) {
  subElementProc(element, V1, [&](XMLElement &subElement) {
    subElement.QueryBoolAttribute(ENABLE, &holders[Wdr1].isEnable);

    Json::Value jCurve;

    subElementProc(subElement, CURVE, [&](XMLElement &subElement) {
      CamEngineWdrCurve_t &curve = holders[Wdr1].config.wdr1.curve;

      Json::Value jDy;

      subElementGet(subElement, DY, jDy);

      for (int32_t i = 0; i < WDR_CURVE_ARRAY_SIZE; i++) {
        curve.dY[i] = static_cast<uint8_t>(jDy[i].asUInt());
      }

      Json::Value jYm;

      subElementGet(subElement, YM, jYm);

      for (int32_t i = 0; i < WDR_CURVE_ARRAY_SIZE; i++) {
        curve.Ym[i] = static_cast<uint16_t>(jYm[i].asUInt());
      }
    });
  });

  subElementProc(element, V2, [&](XMLElement &subElement) {
    subElement.QueryBoolAttribute(ENABLE, &holders[Wdr2].isEnable);

    subElementGet(subElement, STRENGTH, holders[Wdr2].config.wdr2.strength);
  });

  subElementProc(element, V3, [&](XMLElement &subElement) {
    subElement.QueryBoolAttribute(ENABLE, &holders[Wdr3].isEnable);

    subElement.QueryBoolAttribute(AUTO, &holders[Wdr3].config.wdr3.isAuto);

    Config::Wdr3 &wdr3 = holders[Wdr3].config.wdr3;

    subElementGet(subElement, MAX_GAIN, wdr3.gainMax);
    subElementGet(subElement, STRENGTH, wdr3.strength);
    subElementGet(subElement, STRENGTH_GLOBAL, wdr3.strengthGlobal);

    subElementGet(subElement, TABLE, holders[Wdr3].table.jTable);
  });
}

void CalibWdr::Config::Wdr1::reset() {
  const CamEngineWdrCurve_t curveX0 = {
      // .Ym =
      {0x0,   0x7c,  0xf8,  0x174, 0x1f0, 0x26c, 0x2e8, 0x364, 0x3e0,
       0x45d, 0x4d9, 0x555, 0x5d1, 0x64d, 0x6c9, 0x745, 0x7c1, 0x83e,
       0x8ba, 0x936, 0x9b2, 0xa2e, 0xaaa, 0xb26, 0xba2, 0xc1f, 0xc9b,
       0xd17, 0xd93, 0xe0f, 0xe8b, 0xf07, 0xf83},
      // .dY =
      {0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
       4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}};

  curve = curveX0;
}

void CalibWdr::Config::Wdr2::reset() { strength = 0; }

void CalibWdr::Config::Wdr3::reset() {
  isAuto = true;

  gainMax = 16;
  strength = 100;
  strengthGlobal = 0;
}

void CalibWdr::Table::reset(Generation generation) {
  std::string data;

  if (generation == Wdr1) {

  } else if (generation == Wdr2) {

  } else if (generation == Wdr3) {
    data = "{ \"columns\": [\"HDR\", \"Gain\", \"Integration Time\", "
           "\"Strength\", \"Max Gain\", \"Global Curve\"], "
           "\"rows\": [] }";
  }

  //jTable = JsonApp::fromString(data);//need json app
}

/********************************* DEHAZE ********************************************/

CalibDehaze::CalibDehaze(XMLDocument &document) : Element(document) { name = NAME_DEHAZE; }


/********************************* FILTER ********************************************/

CalibFilter::CalibFilter(XMLDocument &document) : Element(document) {
  name = NAME_FILTER;

  config.reset();
  table.reset();
}

void CalibFilter::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibFilter::composeSubElements(XMLElement &element) {
  element.SetAttribute(AUTO, config.isAuto);

  subElementSet(element, DENOISE, config.denoise);
  subElementSet(element, SHARPEN, config.sharpen);

  subElementSetJson(element, TABLE, table.jTable);
}

void CalibFilter::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibFilter::parseSubElements(XMLElement &element) {
  element.QueryBoolAttribute(AUTO, &config.isAuto);

  subElementGet(element, DENOISE, config.denoise);
  subElementGet(element, SHARPEN, config.sharpen);

  subElementGet(element, TABLE, table.jTable);
}

void CalibFilter::Config::reset() {
  isAuto = true;

  denoise = 1;
  sharpen = 3;
}

void CalibFilter::Table::reset() {
/*
  std::string data = "{ \"columns\": [\"HDR\", \"Gain\", \"Integration Time\", "
                     "\"Denoising\", \"Sharpening\"], "
                     "\"rows\": [] }";

  jTable = JsonApp::fromString(data);
*/ //need to do
}


/********************************* DEWARP ********************************************/

CalibDewarp::CalibDewarp(XMLDocument &document) : Element(document) {
  name = NAME_DEWARP;
}

CalibDewarp::~CalibDewarp() {
#if defined CTRL_DEWARP
  config.freeUserMap();
#endif
}

void CalibDewarp::composeAttributes(XMLElement &element) {
  element.SetAttribute(ENABLE, isEnable);
}

void CalibDewarp::composeSubElements(XMLElement &element) {
#if defined CTRL_DEWARP
  subElementProc(element, KEY_PARAMS, [&](XMLElement &subElement) {
    dewarp_parameters &params = config.params;

    subElement.SetAttribute(KEY_SPLIT, params.is_split != 0);

    subElementSet(subElement, KEY_BOUNDARY_U, params.boundary_pixel.U);
    subElementSet(subElement, KEY_BOUNDARY_V, params.boundary_pixel.V);
    subElementSet(subElement, KEY_BOUNDARY_Y, params.boundary_pixel.Y);

    subElementSet(subElement, KEY_IMAGE_HEIGHT, params.image_size.height);
    subElementSet(subElement, KEY_IMAGE_WIDTH, params.image_size.width);

    subElementSet(subElement, KEY_PIX_FMT_IN, params.pix_fmt_in);
    subElementSet(subElement, KEY_PIX_FMT_OUT, params.pix_fmt_out);

    subElementSet(subElement, KEY_TYPE, params.dewarp_type);
  });

  subElementProc(element, KEY_MAP, [&](XMLElement &subElement) {
    dewarp_distortion_map &map = config.distortionMap[0];

    Json::Value jCameraMatrix;
    Json::Value jPerspectiveMatrix;
    Json::Value jDistortionCoeff;

    for (int32_t i = 0; i < 9; i++) {
      jCameraMatrix.append(map.camera_matrix[i]);
      jPerspectiveMatrix.append(map.perspective_matrix[i]);

      if (i < 8) {
        jDistortionCoeff.append(map.distortion_coeff[i]);
      }
    }

    subElementSetJson(subElement, KEY_CAMERA_MATRIX, jCameraMatrix);
    subElementSetJson(subElement, KEY_PERSPECTIVE_MATRIX, jPerspectiveMatrix);
    subElementSetJson(subElement, KEY_DISTORTION_COEFF, jDistortionCoeff);
  });
#else
  element.ToElement();
#endif
}

void CalibDewarp::parseAttributes(XMLElement &element) {
  element.QueryBoolAttribute(ENABLE, &isEnable);
}

void CalibDewarp::parseSubElements(XMLElement &element) {
#if defined CTRL_DEWARP
  subElementProc(element, KEY_PARAMS, [&](XMLElement &subElement) {
    dewarp_parameters &params = config.params;

    bool isSplit = false;

    subElement.QueryBoolAttribute(KEY_SPLIT, &isSplit);

    params.is_split = isSplit ? 1 : 0;

    subElementGet(subElement, KEY_BOUNDARY_U, params.boundary_pixel.U);
    subElementGet(subElement, KEY_BOUNDARY_V, params.boundary_pixel.V);
    subElementGet(subElement, KEY_BOUNDARY_Y, params.boundary_pixel.Y);

    subElementGet(subElement, KEY_IMAGE_HEIGHT, params.image_size.height);
    subElementGet(subElement, KEY_IMAGE_WIDTH, params.image_size.width);

    subElementGet(subElement, KEY_PIX_FMT_IN, params.pix_fmt_in);
    subElementGet(subElement, KEY_PIX_FMT_OUT, params.pix_fmt_out);

    subElementGet(subElement, KEY_TYPE, params.dewarp_type);
  });

  subElementProc(element, KEY_MAP, [&](XMLElement &subElement) {
    Json::Value jCameraMatrix;

    subElementGet(subElement, KEY_CAMERA_MATRIX, jCameraMatrix);

    Json::Value jPerspectiveMatrix;

    subElementGet(subElement, KEY_PERSPECTIVE_MATRIX, jPerspectiveMatrix);

    Json::Value jDistortionCoeff;

    subElementGet(subElement, KEY_DISTORTION_COEFF, jDistortionCoeff);

    dewarp_distortion_map &map = config.distortionMap[0];

    for (int32_t i = 0; i < 9; i++) {
      map.camera_matrix[i] = jCameraMatrix[i].asDouble();
      map.perspective_matrix[i] = jPerspectiveMatrix[i].asDouble();

      if (i < 8) {
        map.distortion_coeff[i] = jDistortionCoeff[i].asDouble();
      }
    }
  });
#else
  element.ToElement();
#endif
}

#if defined CTRL_DEWARP
CalibDewarp::Config &CalibDewarp::Config::operator=(const Config &other) {
  if (this != &other) {
    this->freeUserMap();

    memcpy(this, &other, sizeof(*this));
  }

  return *this;
}

void CalibDewarp::Config::freeUserMap() {
  if (distortionMap->pUserMap) {
    free(distortionMap->pUserMap);
  }

  for (int32_t i = 0; i < DEWARP_CORE_MAX; i++) {
    dewarp_distortion_map *pDistortion = distortionMap + i;

    if (pDistortion->pUserMap) {
      pDistortion->pUserMap = nullptr;
    }

    pDistortion->userMapSize = 0;
  }
}
#endif
