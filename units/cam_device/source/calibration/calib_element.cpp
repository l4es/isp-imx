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

#include "calib_element.hpp"
#include "exception.hpp"

using namespace camdev;

Element::Element(XMLDocument &document, std::string name)
    : document(document), name(name) {}

Element::~Element() {}

void Element::compose(XMLElement &element) {
  composeAttributes(element);

  composeSubElements(element);
}

void Element::composeAttributes(XMLElement &) {}

void Element::composeSubElements(XMLElement &) {}

void Element::parse(XMLElement &element) {
  if (name != element.Name()) {
    throw std::exception();
  }

  parseAttributes(element);

  parseSubElements(element);
}

void Element::parseAttributes(XMLElement &) {}

void Element::parseSubElements(XMLElement &) {}

XMLElement &Element::subElementGet(XMLElement &element, const char *pKey,
                                    int16_t &value) {
  XMLElement *pSubElement = nullptr;

  if ((pSubElement = element.FirstChildElement(pKey))) {
    int32_t value32 = 0;

    pSubElement->QueryIntText(&value32);

    value = static_cast<int16_t>(value32);
  }

  return *pSubElement;
}

XMLElement &Element::subElementGet(XMLElement &element, const char *pKey,
                                    int32_t &value) {
  XMLElement *pSubElement = nullptr;

  if ((pSubElement = element.FirstChildElement(pKey))) {
    pSubElement->QueryIntText(&value);
  }

  return *pSubElement;
}

XMLElement &Element::subElementGet(XMLElement &element, const char *pKey,
                                    uint8_t &value) {
  XMLElement *pSubElement = nullptr;

  if ((pSubElement = element.FirstChildElement(pKey))) {
    uint32_t value32 = 0;

    pSubElement->QueryUnsignedText(&value32);

    value = static_cast<uint8_t>(value32);
  }

  return *pSubElement;
}

XMLElement &Element::subElementGet(XMLElement &element, const char *pKey,
                                    uint16_t &value) {
  XMLElement *pSubElement = nullptr;

  if ((pSubElement = element.FirstChildElement(pKey))) {
    uint32_t value32 = 0;

    pSubElement->QueryUnsignedText(&value32);

    value = static_cast<uint16_t>(value32);
  }

  return *pSubElement;
}

XMLElement &Element::subElementGet(XMLElement &element, const char *pKey,
                                    uint32_t &value) {
  XMLElement *pSubElement = nullptr;

  if ((pSubElement = element.FirstChildElement(pKey))) {
    pSubElement->QueryUnsignedText(&value);
  }

  return *pSubElement;
}

XMLElement &Element::subElementGet(XMLElement &element, const char *pKey,
                                    double &value) {
  XMLElement *pSubElement = nullptr;

  if ((pSubElement = element.FirstChildElement(pKey))) {
    pSubElement->QueryDoubleText(&value);
  }

  return *pSubElement;
}

XMLElement &Element::subElementGet(XMLElement &element, const char *pKey,
                                    float &value) {
  XMLElement *pSubElement = nullptr;

  if ((pSubElement = element.FirstChildElement(pKey))) {
    pSubElement->QueryFloatText(&value);
  }

  return *pSubElement;
}

XMLElement &Element::subElementGet(XMLElement &element, const char *pKey,
                                    Json::Value &value) {
  XMLElement *pSubElement = nullptr;

  if ((pSubElement = element.FirstChildElement(pKey))) {
    Json::CharReaderBuilder charReaderBuilder;
    Json::CharReader *pCharReader = charReaderBuilder.newCharReader();

    std::string data = pSubElement->GetText();
    std::string errors;

    bool isSuccess = pCharReader->parse(
        data.c_str(), data.c_str() + data.size(), &value, &errors);
    delete pCharReader;

    if (!isSuccess) {
      throw exc::LogicError(-1, "Parse JSON failed");
    }
  }

  return *pSubElement;
}

XMLElement &Element::subElementSetJson(XMLElement &element, const char *pKey,
                                        Json::Value &value) {
  XMLElement *pSubElement = nullptr;

  if (!(pSubElement = element.FirstChildElement(pKey))) {
    element.InsertEndChild(pSubElement = document.NewElement(pKey));
  }

  Json::StreamWriterBuilder streamWriterBuilder;
  streamWriterBuilder["indentation"] = "";

  pSubElement->SetText(Json::writeString(streamWriterBuilder, value).c_str());

  return *pSubElement;
}
