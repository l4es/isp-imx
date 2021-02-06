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
 * @file    calibdb.cpp
 *
 *****************************************************************************/
#include "calibdb.hpp"
#include "calibtags.hpp"
#include "xmltags.hpp"
#include "macros.hpp"
#include "calib_calibration.hpp"
#include "calib_features.hpp"
#include <common/cam_types.h>
#include <common/return_codes.h>
#include <ebase/builtins.h>
#include <ebase/trace.h>
#include <iostream>

CREATE_TRACER(CALIB_INF, "CALIB_INF: ", INFO, 1);
CREATE_TRACER(CALIB_WAN, "CALIB_WAN: ", WARNING, 1);
CREATE_TRACER(CALIB_ERR, "CALIB_ERR: ", ERROR, 1);

inline void calib_string_copy(char *dst, const char *src, int len) {
    strncpy(dst, src, len - 1);
    dst[len - 1] = '\0';
}

static int32_t
ParseFloatArray(const char *c_string, /**< trimmed c string */
                float *values,        /**< pointer to memory */
                const int32_t num     /**< number of expected float values */
) {
  float *value = values;
  char *str = (char *)c_string;

  int32_t last = strlen(str);

  /* check for beginning/closing parenthesis */
  if ((str[0] != '[') || (str[last - 1] != ']')) {
    return (-1);
  }

  /* calc. end address of string */
  char *str_last = str + (last - 1);

  /* skipped left parenthesis */
  str++;

  /* skip spaces */
  while (*str == 0x20) {
    str++;
  }

  int32_t cnt = 0;
  int32_t scanned;
  float f;

  /* parse the c-string */
  while ((str != str_last) && (cnt < num)) {
    scanned = sscanf(str, "%f", &f);
    if (scanned != 1) {
      goto err1;
    } else {
      value[cnt] = f;
      cnt++;
    }

    /* remove detected float */
    while ((*str != 0x20) && (*str != ',') && (*str != ']')) {
      str++;
    }

    /* skip spaces and comma */
    while ((*str == 0x20) || (*str == ',')) {
      str++;
    }
  }

  return cnt;

err1:
  MEMSET(values, 0, (sizeof(float) * num));

  return 0;
}

static int32_t
ParseUshortArray(const char *c_string, /**< trimmed c string */
                 uint16_t *values,     /**< pointer to memory */
                 const int32_t num     /**< number of expected float values */
) {
  uint16_t *value = values;
  char *str = (char *)c_string;

  int32_t last = strlen(str);

  /* check for beginning/closing parenthesis */
  if ((str[0] != '[') || (str[last - 1] != ']')) {
    return (-1);
  }

  /* calc. end address of string */
  char *str_last = str + (last - 1);

  /* skipped left parenthesis */
  str++;

  /* skip spaces */
  while (*str == 0x20) {
    str++;
  }

  int32_t cnt = 0;
  int32_t scanned;
  uint16_t f;

  /* parse the c-string */
  while ((str != str_last) && (cnt < num)) {
    scanned = sscanf(str, "%hu", &f);
    if (scanned != 1) {
      goto err1;
    } else {
      value[cnt] = f;
      cnt++;
    }

    /* remove detected float */
    while ((*str != 0x20) && (*str != ',') && (*str != ']')) {
      str++;
    }

    /* skip spaces and comma */
    while ((*str == 0x20) || (*str == ',')) {
      str++;
    }
  }

  return cnt;

err1:
  MEMSET(values, 0, (sizeof(uint16_t) * num));

  return 0;
}

static int32_t
ParseShortArray(const char *c_string, /**< trimmed c string */
                int16_t *values,      /**< pointer to memory */
                const int32_t num     /**< number of expected float values */
) {
  int16_t *value = values;
  char *str = (char *)c_string;

  int32_t last = strlen(str);

  /* check for beginning/closing parenthesis */
  if ((str[0] != '[') || (str[last - 1] != ']')) {
    return (-1);
  }

  /* calc. end address of string */
  char *str_last = str + (last - 1);

  /* skipped left parenthesis */
  str++;

  /* skip spaces */
  while (*str == 0x20) {
    str++;
  }

  int32_t cnt = 0;
  int32_t scanned;
  int16_t f;

  /* parse the c-string */
  while ((str != str_last) && (cnt < num)) {
    scanned = sscanf(str, "%hd", &f);
    if (scanned != 1) {
      goto err1;
    } else {
      value[cnt] = f;
      cnt++;
    }

    /* remove detected float */
    while ((*str != 0x20) && (*str != ',') && (*str != ']')) {
      str++;
    }

    /* skip spaces and comma */
    while ((*str == 0x20) || (*str == ',')) {
      str++;
    }
  }

  return cnt;

err1:
  MEMSET(values, 0, (sizeof(uint16_t) * num));

  return 0;
}

static int32_t
ParseByteArray(const char *c_string, /**< trimmed c string */
               uint8_t *values,      /**< pointer to memory */
               const int32_t num     /**< number of expected float values */
) {
  uint8_t *value = values;
  char *str = (char *)c_string;

  int32_t last = strlen(str);

  /* check for beginning/closing parenthesis */
  if ((str[0] != '[') || (str[last - 1] != ']')) {
    return (-1);
  }

  /* calc. end address of string */
  char *str_last = str + (last - 1);

  /* skipped left parenthesis */
  str++;

  /* skip spaces */
  while (*str == 0x20) {
    str++;
  }

  int32_t cnt = 0;
  int32_t scanned;
  uint16_t f;

  /* parse the c-string */
  while ((str != str_last) && (cnt < num)) {
    scanned = sscanf(str, "%hu", &f);
    if (scanned != 1) {
      goto err1;
    } else {
      value[cnt] = (uint8_t)f;
      cnt++;
    }

    /* remove detected float */
    while ((*str != 0x20) && (*str != ',') && (*str != ']')) {
      str++;
    }

    /* skip spaces and comma */
    while ((*str == 0x20) || (*str == ',')) {
      str++;
    }
  }

  return cnt;

err1:
  MEMSET(values, 0, (sizeof(uint8_t) * num));

  return 0;
}

static int32_t
ParseCcProfileArray(const char *c_string,        /**< trimmed c string */
                    CamCcProfileName_t values[], /**< pointer to memory */
                    const int32_t num /**< number of expected float values */
) {
  char *str = (char *)c_string;

  int32_t last = strlen(str);

  /* calc. end address of string */
  char *str_last = str + (last - 1);

  /* skip beginning spaces */
  while (*str == 0x20) {
    str++;
  }

  /* skip ending spaces */
  while (*str_last == 0x20) {
    str_last--;
  }

  int32_t cnt = 0;
  int32_t scanned;
  CamCcProfileName_t f;
  MEMSET(f, 0, sizeof(f));

  /* parse the c-string */
  while ((str != str_last) && (cnt < num)) {
    scanned = sscanf(str, "%s", f);
    if (scanned != 1) {
      goto err1;
    } else {
      strcpy(values[cnt], f);
      cnt++;
    }

    /* remove detected string */
    while ((*str != 0x20) && (*str != ',') && (*str != ']') &&
           (str != str_last)) {
      str++;
    }

    if (str != str_last) {
      /* skip spaces and comma */
      while ((*str == 0x20) || (*str == ',')) {
        str++;
      }
    }

    MEMSET(f, 0, sizeof(f));
  }

  return cnt;

err1:
  MEMSET(values, 0, (sizeof(uint16_t) * num));

  return 0;
}

static int32_t
ParseLscProfileArray(const char *c_string,         /**< trimmed c string */
                     CamLscProfileName_t values[], /**< pointer to memory */
                     const int32_t num /**< number of expected float values */
) {
  char *str = (char *)c_string;

  int32_t last = strlen(str);

  /* calc. end address of string */
  char *str_last = str + (last - 1);

  /* skip beginning spaces */
  while (*str == 0x20) {
    str++;
  }

  /* skip ending spaces */
  while (*str_last == 0x20) {
    str_last--;
  }

  int32_t cnt = 0;
  int32_t scanned;
  CamLscProfileName_t f;
  MEMSET(f, 0, sizeof(f));

  /* parse the c-string */
  while ((str != str_last) && (cnt < num)) {
    scanned = sscanf(str, "%s", f);
    if (scanned != 1) {
      goto err1;
    } else {
      strcpy(values[cnt], f);
      cnt++;
    }

    /* remove detected string */
    while ((*str != 0x20) && (*str != ',') && (*str != ']') &&
           (str != str_last)) {
      str++;
    }

    if (str != str_last) {
      /* skip spaces and comma */
      while ((*str == 0x20) || (*str == ',')) {
        str++;
      }
    }

    MEMSET(f, 0, sizeof(f));
  }

  return cnt;

err1:
  MEMSET(values, 0, (sizeof(uint16_t) * num));

  return 0;
}

CalibDb::~CalibDb() {
  if (hCalibDb != NULL) {
    int32_t ret = CamCalibDbRelease(&hCalibDb);
    DCT_ASSERT(ret == RET_SUCCESS);
  }
}

int32_t CalibDb::install(std::string &filename) {
  XMLDocument doc(true, COLLAPSE_WHITESPACE);

  XMLError err = doc.LoadFile(filename.c_str());
  if (err != XML_SUCCESS) {
    TRACE(CALIB_ERR, "XML error: %d \n", err);

    REPORT(RET_FAILURE);
  }

  int32_t ret = RET_SUCCESS;

  DCT_ASSERT(!CamCalibDbCreate(&hCalibDb));

  XMLElement *pRoot = doc.RootElement();

  ret = parseEntryHeader(pRoot->FirstChildElement(CALIB_HEADER_TAG), NULL);
  REPORT(ret);

  ret = parseEntrySensor(pRoot->FirstChildElement(CALIB_SENSOR_TAG), NULL);
  REPORT(ret);

  ret = parseEntrySystem(pRoot->FirstChildElement(CALIB_SYSTEM_TAG), NULL);
  REPORT(ret);

  doc.Clear();

  return RET_SUCCESS;
}

int32_t CalibDb::uninstall() {
  if (hCalibDb) {
    DCT_ASSERT(!CamCalibDbRelease(&hCalibDb));
  }

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryCell(const XMLElement *pElement, int32_t noElements,
                                parseCellContent func, void *pParam) {
  int32_t cnt = 0;

  const XMLNode *pChild = pElement->FirstChild();

  while (pChild && (cnt < noElements)) {
    if (!strcmp(pChild->ToElement()->Name(), CALIB_CELL_TAG)) {
      int32_t ret = (this->*func)(pChild->ToElement(), pParam);
      REPORT(ret);
    } else {
      TRACE(CALIB_ERR, "unknown cell tag: %s \n", pChild->ToElement()->Name());

      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
    cnt++;
  }

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryHeader(const XMLElement *pElement, void *pParam) {
  (void)pParam;

  CamCalibDbMetaData_t meta_data;
  MEMSET(&meta_data, 0, sizeof(meta_data));

  const XMLNode *pChild = pElement->FirstChild();

  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());
    const char *pValue = tag.value();

    if (!strcmp(pChild->ToElement()->Name(), CALIB_HEADER_CREATION_DATE_TAG) &&
        (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      calib_string_copy(meta_data.cdate, pValue, sizeof(meta_data.cdate));
    } else if (!strcmp(pChild->ToElement()->Name(), CALIB_HEADER_CREATOR_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      calib_string_copy(meta_data.cname, pValue, sizeof(meta_data.cname));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_HEADER_GENERATOR_VERSION_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      calib_string_copy(meta_data.cversion, pValue, sizeof(meta_data.cversion));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_HEADER_SENSOR_NAME_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      calib_string_copy(meta_data.sname, pValue, sizeof(meta_data.sname));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_HEADER_SAMPLE_NAME_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      calib_string_copy(meta_data.sid, pValue, sizeof(meta_data.sid));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_HEADER_RESOLUTION_TAG)) {
      int32_t ret = parseEntryCell(pChild->ToElement(), tag.size(),
                                   &CalibDb::parseEntryResolution);
      REPORT(ret);
    } else {
      TRACE(CALIB_ERR, "parse error in header section, unknown tag: %s \n",
            pChild->ToElement()->Name());

      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  int32_t ret = CamCalibDbSetMetaData(hCalibDb, &meta_data);
  REPORT(ret);

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryResolution(const XMLElement *pElement,
                                      void *pParam) {
  (void)pParam;

  CamResolution_t resolution;
  MEMSET(&resolution, 0, sizeof(resolution));

  const XMLNode *pChild = pElement->FirstChild();

  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());
    const char *pValue = tag.value();

    if (!strcmp(pChild->ToElement()->Name(),
                CALIB_HEADER_RESOLUTION_NAME_TAG) &&
        (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      calib_string_copy(resolution.name, pValue, sizeof(resolution.name));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_HEADER_RESOLUTION_WIDTH_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseUshortArray(pValue, &resolution.width, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_HEADER_RESOLUTION_HEIGHT_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseUshortArray(pValue, &resolution.height, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_HEADER_RESOLUTION_FRATE_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseUshortArray(pValue, &resolution.framerate, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_HEADER_RESOLUTION_FRATE_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CELL)) && (tag.size() > 0)) {
      resolution.framerate = 0; // TODO: parse AFPS framerates
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_HEADER_RESOLUTION_ID_TAG)) {
      resolution.id = tag.valueToUInt();
    } else {
      TRACE(CALIB_ERR, "unknown tag: %s \n", pChild->ToElement()->Name());

      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  int32_t ret = CamCalibDbAddResolution(hCalibDb, &resolution);
  REPORT(ret);

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntrySensor(const XMLElement *pElement, void *pParam) {
  (void)pParam;

  int32_t ret = RET_SUCCESS;

  const XMLNode *pChild = pElement->FirstChild();

  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());

    if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_AWB_TAG)) {
      ret = parseEntryAwb(pChild->ToElement());
      REPORT(ret);
    } else if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_LSC_TAG)) {
      ret = parseEntryCell(pChild->ToElement(), tag.size(),
                           &CalibDb::parseEntryLsc);
      REPORT(ret);
    } else if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_CC_TAG)) {
      ret = parseEntryCell(pChild->ToElement(), tag.size(),
                           &CalibDb::parseEntryCc);
      REPORT(ret);
    } else if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_AF_TAG)) {
      TRACE(CALIB_INF, "tag: %s \n", pChild->ToElement()->Name());
    } else if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_AEC_TAG)) {
      TRACE(CALIB_INF, "tag: %s \n", pChild->ToElement()->Name());
      ret = parseEntryAec(pChild->ToElement());
      REPORT(ret);
    } else if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_BLS_TAG)) {
      ret = parseEntryCell(pChild->ToElement(), tag.size(),
                           &CalibDb::parseEntryBls);
      REPORT(ret);
    } else if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_DEGAMMA_TAG)) {
      TRACE(CALIB_INF, "tag: %s \n", pChild->ToElement()->Name());
    } else if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_WDR_TAG)) {
      TRACE(CALIB_INF, "tag: %s \n", pChild->ToElement()->Name());
    } else if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_CAC_TAG)) {
      ret = parseEntryCell(pChild->ToElement(), tag.size(),
                           &CalibDb::parseEntryCac);
      REPORT(ret);
    } else if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_DPF_TAG)) {
      ret = parseEntryCell(pChild->ToElement(), tag.size(),
                           &CalibDb::parseEntryDpf);
      REPORT(ret);
    } else if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_DPCC_TAG)) {
      ret = parseEntryCell(pChild->ToElement(), tag.size(),
                           &CalibDb::parseEntryDpcc);
      REPORT(ret);
    } else if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_HDR_TAG)) {
      ret = parseEntryHdr(pChild->ToElement());
      REPORT(ret);
    } else {
      TRACE(CALIB_ERR, "parse error in header section, unknown tag: %s \n",
            pChild->ToElement()->Name());

      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryAec(const XMLElement *pElement, void *pParam) {
  (void)pParam;

  int32_t ret = RET_SUCCESS;

  CamCalibAecGlobal_t aec_data;

  const XMLNode *pChild = pElement->FirstChild();

  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());

    if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_AEC_SETPOINT_TAG) &&
        (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseFloatArray(tag.value(), &aec_data.SetPoint, 1);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AEC_CLM_TOLERANCE_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseFloatArray(tag.value(), &aec_data.ClmTolerance, 1);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AEC_DAMP_OVER_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseFloatArray(tag.value(), &aec_data.DampOver, 1);
      DCT_ASSERT((no == tag.size()));
    }else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AEC_DAMP_OVER_VIDEO_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      TRACE(CALIB_WAN, "Don't support: %s \n", pChild->ToElement()->Name());
      // int32_t no = ParseFloatArray(tag.value(), &aec_data.DampOverVideo, 1);
      // DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AEC_DAMP_UNDER_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseFloatArray(tag.value(), &aec_data.DampUnder, 1);
      DCT_ASSERT((no == tag.size()));
    }else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AEC_DAMP_UNDER_VIDEO_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      TRACE(CALIB_WAN, "Don't support: %s \n", pChild->ToElement()->Name());
      // int32_t no = ParseFloatArray(tag.value(), &aec_data.DampUnderVideo, 1);
      // DCT_ASSERT((no == tag.size()));
    }else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AEC_AFPS_MAX_GAIN_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseFloatArray(tag.value(), &aec_data.AfpsMaxGain, 1);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_AEC_ECM_TAG)) {
      //FIXME: crash at second time start,  release mode
#if 0
      ret = parseEntryCell(pChild->ToElement(), tag.size(),
                           &CalibDb::parseEntryAecEcm);
      REPORT(ret);
#endif
    } else {
      TRACE(CALIB_ERR, "parse error in AEC section, unknown tag: %s \n",
            pChild->ToElement()->Name());
      // REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  ret = CamCalibDbAddAecGlobal(hCalibDb, &aec_data);
  REPORT(ret);

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryAecEcm(const XMLElement *pElement, void *pParam) {
  (void)pParam;

  int32_t ret = RET_SUCCESS;

  CamEcmProfile_t EcmProfile;
  MEMSET(&EcmProfile, 0, sizeof(EcmProfile));

  ListInit(&EcmProfile.ecm_scheme);

  const XMLNode *pChild = pElement->FirstChild();

  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());

    if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_AEC_ECM_NAME_TAG) &&
        (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.valueToUpper();
      calib_string_copy(EcmProfile.name, pValue, sizeof(EcmProfile.name));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AEC_ECM_SCHEMES_TAG)) {
      ret =
          parseEntryCell(pChild->ToElement(), tag.size(),
                         &CalibDb::parseEntryAecEcmPriorityScheme, &EcmProfile);
      REPORT(ret);
    } else {
      TRACE(CALIB_ERR, "parse error in ECM section, unknown tag: %s \n",
            pChild->ToElement()->Name());

      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  ret = CamCalibDbAddEcmProfile(hCalibDb, &EcmProfile);
  REPORT(ret);

  // free linked ecm_schemes
  List *l = ListRemoveHead(&EcmProfile.ecm_scheme);
  while (l) {
    List * ListNode = l;
    l = ListRemoveHead(ListNode);
    free(ListNode);
  }

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryAecEcmPriorityScheme(const XMLElement *pElement,
                                                void *pParam) {
  DCT_ASSERT(pElement);
  DCT_ASSERT(pParam);

  CamEcmProfile_t *pEcmProfile = (CamEcmProfile_t *)pParam;

  CamEcmScheme_t *pEcmScheme = (CamEcmScheme_t *)malloc(sizeof(CamEcmScheme_t));
  DCT_ASSERT(pEcmScheme);

  MEMSET(pEcmScheme, 0, sizeof(*pEcmScheme));

  const XMLNode *pChild = pElement->FirstChild();

  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());

    if (!strcmp(pChild->ToElement()->Name(),
                CALIB_SENSOR_AEC_ECM_SCHEME_NAME_TAG) &&
        (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.valueToUpper();
      calib_string_copy(pEcmScheme->name, pValue, sizeof(pEcmScheme->name));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AEC_ECM_SCHEME_OFFSETT0FAC_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseFloatArray(tag.value(), &pEcmScheme->OffsetT0Fac, 1);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AEC_ECM_SCHEME_SLOPEA0_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseFloatArray(tag.value(), &pEcmScheme->SlopeA0, 1);
      DCT_ASSERT((no == tag.size()));
    } else {
      TRACE(CALIB_ERR, "parse error in ECM section, unknown tag: %s \n",
            pChild->ToElement()->Name());

      free(pEcmScheme);
      pEcmScheme = NULL;

      // return ( false );
    }

    pChild = pChild->NextSibling();
  }

  if (pEcmScheme) {
    ListPrepareItem(pEcmScheme);
    ListAddTail(&pEcmProfile->ecm_scheme, pEcmScheme);
  }

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryAwb(const XMLElement *pElement, void *pParam) {
  (void)pParam;

  int32_t ret = RET_SUCCESS;

  const XMLNode *pChild = pElement->FirstChild();

  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());

    if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_AWB_GLOBALS_TAG)) {
      ret = parseEntryCell(pChild->ToElement(), tag.size(),
                           &CalibDb::parseEntryAwbGlobals);
      REPORT(ret);
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_ILLUMINATION_TAG)) {
      ret = parseEntryCell(pChild->ToElement(), tag.size(),
                           &CalibDb::parseEntryAwbIllumination);
      REPORT(ret);
    } else {
      TRACE(CALIB_ERR, "parse error in AWB section, unknown tag: %s \n",
            pChild->ToElement()->Name());
      // return ( false );
    }

    pChild = pChild->NextSibling();
  }

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryAwbGlobals(const XMLElement *pElement,
                                      void *pParam) {
  (void)pParam;

  CamCalibAwbGlobal_t awb_data;

  /* CamAwbClipParm_t */
  float *pRg1 = NULL;
  int32_t nRg1 = 0;
  float *pMaxDist1 = NULL;
  int32_t nMaxDist1 = 0;
  float *pRg2 = NULL;
  int32_t nRg2 = 0;
  float *pMaxDist2 = NULL;
  int32_t nMaxDist2 = 0;

  /* CamAwbGlobalFadeParm_t */
  float *pGlobalFade1 = NULL;
  int32_t nGlobalFade1 = 0;
  float *pGlobalGainDistance1 = NULL;
  int32_t nGlobalGainDistance1 = 0;
  float *pGlobalFade2 = NULL;
  int32_t nGlobalFade2 = 0;
  float *pGlobalGainDistance2 = NULL;
  int32_t nGlobalGainDistance2 = 0;

  /* CamAwbFade2Parm_t */
  float *pFade = NULL;
  int32_t nFade = 0;
  float *pCbMinRegionMax = NULL;
  int32_t nCbMinRegionMax = 0;
  float *pCrMinRegionMax = NULL;
  int32_t nCrMinRegionMax = 0;
  float *pMaxCSumRegionMax = NULL;
  int32_t nMaxCSumRegionMax = 0;
  float *pCbMinRegionMin = NULL;
  int32_t nCbMinRegionMin = 0;
  float *pCrMinRegionMin = NULL;
  int32_t nCrMinRegionMin = 0;
  float *pMaxCSumRegionMin = NULL;
  int32_t nMaxCSumRegionMin = 0;

  const XMLNode *pChild = pElement->FirstChild();

  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());
    const char *pValue = tag.value();

    if (!strcmp(pChild->ToElement()->Name(),
                CALIB_SENSOR_AWB_GLOBALS_NAME_TAG) &&
        (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      calib_string_copy(awb_data.name, pValue, sizeof(awb_data.name));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_RESOLUTION_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      calib_string_copy(awb_data.resolution, pValue, sizeof(awb_data.resolution));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_SENSOR_FILENAME_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      // do nothing
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_SVDMEANVALUE_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0))

    {
      int32_t i = (sizeof(awb_data.SVDMeanValue) /
                   sizeof(awb_data.SVDMeanValue.fCoeff[0]));
      int32_t no = ParseFloatArray(pValue, awb_data.SVDMeanValue.fCoeff, i);

      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_PCAMATRIX_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i =
          (sizeof(awb_data.PCAMatrix) / sizeof(awb_data.PCAMatrix.fCoeff[0]));
      int32_t no = ParseFloatArray(tag.value(), awb_data.PCAMatrix.fCoeff, i);

      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_CENTERLINE_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i =
          (sizeof(awb_data.CenterLine) / sizeof(awb_data.CenterLine.f_N0_Rg));
      int32_t no =
          ParseFloatArray(tag.value(), &awb_data.CenterLine.f_N0_Rg, i);

      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_KFACTOR_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i =
          (sizeof(awb_data.KFactor) / sizeof(awb_data.KFactor.fCoeff[0]));
      int32_t no = ParseFloatArray(tag.value(), awb_data.KFactor.fCoeff, i);

      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_RG1_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0) &&
               (NULL == pRg1)) {
      nRg1 = tag.size();
      pRg1 = (float *)malloc(sizeof(float) * nRg1);
      DCT_ASSERT(pRg1);

      int32_t no = ParseFloatArray(tag.value(), pRg1, nRg1);
      DCT_ASSERT((no == nRg1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_MAXDIST1_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0) &&
               (NULL == pMaxDist1)) {
      nMaxDist1 = tag.size();
      pMaxDist1 = (float *)malloc(sizeof(float) * nMaxDist1);
      DCT_ASSERT(pMaxDist1);

      int32_t no = ParseFloatArray(tag.value(), pMaxDist1, nMaxDist1);
      DCT_ASSERT((no == nRg1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_RG2_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0) &&
               (NULL == pRg2)) {
      nRg2 = tag.size();
      pRg2 = (float *)malloc(sizeof(float) * nRg2);
      DCT_ASSERT(pRg2);

      int32_t no = ParseFloatArray(tag.value(), pRg2, nRg2);
      DCT_ASSERT((no == nRg2));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_MAXDIST2_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0) &&
               (NULL == pMaxDist2)) {
      nMaxDist2 = tag.size();
      pMaxDist2 = (float *)malloc(sizeof(float) * nMaxDist2);
      DCT_ASSERT(pMaxDist2);

      int32_t no = ParseFloatArray(tag.value(), pMaxDist2, nMaxDist2);
      DCT_ASSERT((no == nMaxDist2));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_GLOBALFADE1_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0) &&
               (NULL == pGlobalFade1)) {
      nGlobalFade1 = tag.size();
      pGlobalFade1 = (float *)malloc(sizeof(float) * nGlobalFade1);
      DCT_ASSERT(pGlobalFade1);

      int32_t no = ParseFloatArray(tag.value(), pGlobalFade1, nGlobalFade1);
      DCT_ASSERT((no == nGlobalFade1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_GLOBALGAINDIST1_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0) &&
               (NULL == pGlobalGainDistance1)) {
      nGlobalGainDistance1 = tag.size();
      pGlobalGainDistance1 =
          (float *)malloc(sizeof(float) * nGlobalGainDistance1);
      DCT_ASSERT(pGlobalGainDistance1);

      int32_t no = ParseFloatArray(tag.value(), pGlobalGainDistance1,
                                   nGlobalGainDistance1);
      DCT_ASSERT((no == nGlobalGainDistance1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_GLOBALFADE2_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0) &&
               (NULL == pGlobalFade2)) {
      nGlobalFade2 = tag.size();
      pGlobalFade2 = (float *)malloc(sizeof(float) * nGlobalFade2);
      DCT_ASSERT(pGlobalFade2);

      int32_t no = ParseFloatArray(tag.value(), pGlobalFade2, nGlobalFade2);
      DCT_ASSERT((no == nGlobalFade2));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_GLOBALGAINDIST2_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0) &&
               (NULL == pGlobalGainDistance2)) {
      nGlobalGainDistance2 = tag.size();
      pGlobalGainDistance2 =
          (float *)malloc(sizeof(float) * nGlobalGainDistance2);
      DCT_ASSERT(pGlobalGainDistance2);

      int32_t no = ParseFloatArray(tag.value(), pGlobalGainDistance2,
                                   nGlobalGainDistance2);
      DCT_ASSERT((no == nGlobalGainDistance2));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_FADE2_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0) &&
               (NULL == pFade)) {
      nFade = tag.size();
      pFade = (float *)malloc(sizeof(float) * nFade);
      DCT_ASSERT(pFade);

      int32_t no = ParseFloatArray(tag.value(), pFade, nFade);
      DCT_ASSERT((no == nFade));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_CB_MIN_REGIONMAX_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0) &&
               (NULL == pCbMinRegionMax)) {
      nCbMinRegionMax = tag.size();
      pCbMinRegionMax = (float *)malloc(sizeof(float) * nCbMinRegionMax);
      DCT_ASSERT(pCbMinRegionMax);

      int32_t no =
          ParseFloatArray(tag.value(), pCbMinRegionMax, nCbMinRegionMax);
      DCT_ASSERT((no == nCbMinRegionMax));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_CR_MIN_REGIONMAX_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0) &&
               (NULL == pCrMinRegionMax)) {
      nCrMinRegionMax = tag.size();
      pCrMinRegionMax = (float *)malloc(sizeof(float) * nCrMinRegionMax);
      DCT_ASSERT(pCrMinRegionMax);

      int32_t no =
          ParseFloatArray(tag.value(), pCrMinRegionMax, nCrMinRegionMax);
      DCT_ASSERT((no == nCrMinRegionMax));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_MAX_CSUM_REGIONMAX_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0) &&
               (NULL == pMaxCSumRegionMax)) {
      nMaxCSumRegionMax = tag.size();
      pMaxCSumRegionMax = (float *)malloc(sizeof(float) * nMaxCSumRegionMax);
      DCT_ASSERT(pMaxCSumRegionMax);

      int32_t no =
          ParseFloatArray(tag.value(), pMaxCSumRegionMax, nMaxCSumRegionMax);
      DCT_ASSERT((no == nMaxCSumRegionMax));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_CB_MIN_REGIONMIN_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0) &&
               (NULL == pCbMinRegionMin)) {
      nCbMinRegionMin = tag.size();
      pCbMinRegionMin = (float *)malloc(sizeof(float) * nCbMinRegionMin);
      DCT_ASSERT(pCbMinRegionMin);

      int32_t no =
          ParseFloatArray(tag.value(), pCbMinRegionMin, nCbMinRegionMin);
      DCT_ASSERT((no == nCbMinRegionMin));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_CR_MIN_REGIONMIN_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0) &&
               (NULL == pCrMinRegionMin)) {
      nCrMinRegionMin = tag.size();
      pCrMinRegionMin = (float *)malloc(sizeof(float) * nCrMinRegionMin);
      DCT_ASSERT(pCrMinRegionMin);

      int32_t no =
          ParseFloatArray(tag.value(), pCrMinRegionMin, nCrMinRegionMin);
      DCT_ASSERT((no == nCrMinRegionMin));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_MAX_CSUM_REGIONMIN_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0) &&
               (NULL == pMaxCSumRegionMin)) {
      nMaxCSumRegionMin = tag.size();
      pMaxCSumRegionMin = (float *)malloc(sizeof(float) * nMaxCSumRegionMin);
      DCT_ASSERT(pMaxCSumRegionMin);

      int32_t no =
          ParseFloatArray(tag.value(), pMaxCSumRegionMin, nMaxCSumRegionMin);
      DCT_ASSERT((no == nMaxCSumRegionMin));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_RGPROJ_INDOOR_MIN_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseFloatArray(tag.value(), &awb_data.fRgProjIndoorMin, 1);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_RGPROJ_OUTDOOR_MIN_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseFloatArray(tag.value(), &awb_data.fRgProjOutdoorMin, 1);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_RGPROJ_MAX_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseFloatArray(tag.value(), &awb_data.fRgProjMax, 1);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_RGPROJ_MAX_SKY_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseFloatArray(tag.value(), &awb_data.fRgProjMaxSky, 1);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_CLIP_OUTDOOR) &&
               (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.valueToUpper();
      calib_string_copy(awb_data.outdoor_clipping_profile, pValue,
              sizeof(awb_data.outdoor_clipping_profile));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseFloatArray(tag.value(), &awb_data.fRegionSize, 1);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE_INC) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseFloatArray(tag.value(), &awb_data.fRegionSizeInc, 1);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE_DEC) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseFloatArray(tag.value(), &awb_data.fRegionSizeDec, 1);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_IIR)) {
      const XMLNode *pSubChild = pChild->ToElement()->FirstChild();
      while (pSubChild) {
        XmlTag tag = XmlTag(pSubChild->ToElement());
        if (!strcmp(pSubChild->ToElement()->Name(),
                    CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_COEF_ADD) &&
            (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
          int32_t no =
              ParseFloatArray(tag.value(), &awb_data.IIR.fIIRDampCoefAdd, 1);
          DCT_ASSERT((no == tag.size()));
        } else if (!strcmp(pSubChild->ToElement()->Name(),
                           CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_COEF_SUB) &&
                   (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
          int32_t no =
              ParseFloatArray(tag.value(), &awb_data.IIR.fIIRDampCoefSub, 1);
          DCT_ASSERT((no == tag.size()));
        } else if (!strcmp(
                       pSubChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_FILTER_THRESHOLD) &&
                   (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
          int32_t no = ParseFloatArray(
              tag.value(), &awb_data.IIR.fIIRDampFilterThreshold, 1);
          DCT_ASSERT((no == tag.size()));
        } else if (!strcmp(pSubChild->ToElement()->Name(),
                           CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_MIN) &&
                   (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
          int32_t no =
              ParseFloatArray(tag.value(), &awb_data.IIR.fIIRDampingCoefMin, 1);
          DCT_ASSERT((no == tag.size()));
        } else if (!strcmp(pSubChild->ToElement()->Name(),
                           CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_MAX) &&
                   (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
          int32_t no =
              ParseFloatArray(tag.value(), &awb_data.IIR.fIIRDampingCoefMax, 1);
          DCT_ASSERT((no == tag.size()));
        } else if (!strcmp(pSubChild->ToElement()->Name(),
                           CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_INIT) &&
                   (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
          int32_t no = ParseFloatArray(tag.value(),
                                       &awb_data.IIR.fIIRDampingCoefInit, 1);
          DCT_ASSERT((no == tag.size()));
        } else if (
            !strcmp(pSubChild->ToElement()->Name(),
                    CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_FILTER_SIZE_MAX) &&
            (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
          int32_t no =
              ParseUshortArray(tag.value(), &awb_data.IIR.IIRFilterSize, 1);
          DCT_ASSERT((no == tag.size()));
        } else if (
            !strcmp(pSubChild->ToElement()->Name(),
                    CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_FILTER_SIZE_MIN) &&
            (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
        } else if (!strcmp(pSubChild->ToElement()->Name(),
                           CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_MIDDLE) &&
                   (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
          int32_t no = ParseFloatArray(tag.value(),
                                       &awb_data.IIR.fIIRFilterInitValue, 1);
          DCT_ASSERT((no == tag.size()));
        } else {
          TRACE(CALIB_ERR, "parse error in AWB GLOBALS - IIR section: %s \n",
                pSubChild->ToElement()->Name());
          DCT_ASSERT("parse error in AWB GLOBALS - IIR section");
        }

        pSubChild = pSubChild->NextSibling();
      }
    } else {
      TRACE(CALIB_ERR, "parse error in AWB section, unknown tag: %s \n",
            pChild->ToElement()->Name());
      DCT_ASSERT("parse error in AWB section");
    }

    pChild = pChild->NextSibling();
  }

  DCT_ASSERT((nRg1 == nMaxDist1));
  DCT_ASSERT((nRg2 == nMaxDist2));

  DCT_ASSERT((nGlobalFade1 == nGlobalGainDistance1));
  DCT_ASSERT((nGlobalFade2 == nGlobalGainDistance2));

  DCT_ASSERT((nFade == nCbMinRegionMax));
  DCT_ASSERT((nFade == nCrMinRegionMax));
  DCT_ASSERT((nFade == nMaxCSumRegionMax));
  DCT_ASSERT((nFade == nCbMinRegionMin));
  DCT_ASSERT((nFade == nCrMinRegionMin));
  DCT_ASSERT((nFade == nMaxCSumRegionMin));

  /* CamAwbClipParm_t */
  awb_data.AwbClipParam.ArraySize1 = nRg1;
  awb_data.AwbClipParam.pRg1 = pRg1;
  awb_data.AwbClipParam.pMaxDist1 = pMaxDist1;
  awb_data.AwbClipParam.ArraySize2 = nRg2;
  awb_data.AwbClipParam.pRg2 = pRg2;
  awb_data.AwbClipParam.pMaxDist2 = pMaxDist2;

  /* CamAwbGlobalFadeParm_t */
  awb_data.AwbGlobalFadeParm.ArraySize1 = nGlobalFade1;
  awb_data.AwbGlobalFadeParm.pGlobalFade1 = pGlobalFade1;
  awb_data.AwbGlobalFadeParm.pGlobalGainDistance1 = pGlobalGainDistance1;
  awb_data.AwbGlobalFadeParm.ArraySize2 = nGlobalFade2;
  awb_data.AwbGlobalFadeParm.pGlobalFade2 = pGlobalFade2;
  awb_data.AwbGlobalFadeParm.pGlobalGainDistance2 = pGlobalGainDistance2;

  /* CamAwbFade2Parm_t */
  awb_data.AwbFade2Parm.ArraySize = nFade;
  awb_data.AwbFade2Parm.pFade = pFade;
  awb_data.AwbFade2Parm.pCbMinRegionMax = pCbMinRegionMax;
  awb_data.AwbFade2Parm.pCrMinRegionMax = pCrMinRegionMax;
  awb_data.AwbFade2Parm.pMaxCSumRegionMax = pMaxCSumRegionMax;
  awb_data.AwbFade2Parm.pCbMinRegionMin = pCbMinRegionMin;
  awb_data.AwbFade2Parm.pCrMinRegionMin = pCrMinRegionMin;
  awb_data.AwbFade2Parm.pMaxCSumRegionMin = pMaxCSumRegionMin;

  int32_t ret = CamCalibDbAddAwbGlobal(hCalibDb, &awb_data);
  DCT_ASSERT(ret == RET_SUCCESS);

  /* cleanup */
  free(pRg1);
  free(pMaxDist1);
  free(pRg2);
  free(pMaxDist2);

  free(pGlobalFade1);
  free(pGlobalGainDistance1);
  free(pGlobalFade2);
  free(pGlobalGainDistance2);

  free(pFade);
  free(pCbMinRegionMax);
  free(pCrMinRegionMax);
  free(pMaxCSumRegionMax);
  free(pCbMinRegionMin);
  free(pCrMinRegionMin);
  free(pMaxCSumRegionMin);

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryAwbIllumination(const XMLElement *pElement,
                                           void *pParam) {
  (void)pParam;

  int32_t ret = RET_SUCCESS;

  CamIlluProfile_t illu;
  MEMSET(&illu, 0, sizeof(illu));

  const XMLNode *pChild = pElement->FirstChild();

  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());

    if (!strcmp(pChild->ToElement()->Name(),
                CALIB_SENSOR_AWB_ILLUMINATION_NAME_TAG) &&
        (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.valueToUpper();
      calib_string_copy(illu.name, pValue, sizeof(illu.name));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.valueToUpper();
      if (!strcmp(pValue, CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_INDOOR)) {
        illu.DoorType = CAM_DOOR_TYPE_INDOOR;
      } else if (!strcmp(pValue,
                         CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_OUTDOOR)) {
        illu.DoorType = CAM_DOOR_TYPE_OUTDOOR;
      } else {
        TRACE(CALIB_ERR, "invalid illumination doortype: %s \n", pValue);
      }
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.valueToUpper();
      if (!strcmp(pValue, CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_MANUAL)) {
        illu.AwbType = CAM_AWB_TYPE_MANUAL;
      } else if (!strcmp(pValue, CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_AUTO)) {
        illu.AwbType = CAM_AWB_TYPE_AUTO;
      } else {
        TRACE(CALIB_ERR, "invalid AWB type: %s \n", pValue);
      }
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_WB_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i =
          (sizeof(illu.ComponentGain) / sizeof(illu.ComponentGain.fCoeff[0]));
      int32_t no = ParseFloatArray(tag.value(), illu.ComponentGain.fCoeff, i);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_CC_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i =
          (sizeof(illu.CrossTalkCoeff) / sizeof(illu.CrossTalkCoeff.fCoeff[0]));
      int32_t no = ParseFloatArray(tag.value(), illu.CrossTalkCoeff.fCoeff, i);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_CTO_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i = (sizeof(illu.CrossTalkOffset) /
                   sizeof(illu.CrossTalkOffset.fCoeff[0]));
      int32_t no = ParseFloatArray(tag.value(), illu.CrossTalkOffset.fCoeff, i);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_ILLUMINATION_GMM_TAG)) {
      const XMLNode *pSubChild = pChild->ToElement()->FirstChild();
      while (pSubChild) {
        XmlTag tag = XmlTag(pSubChild->ToElement());
        if (!strcmp(pSubChild->ToElement()->Name(),
                    CALIB_SENSOR_AWB_ILLUMINATION_GMM_GAUSSIAN_MVALUE_TAG) &&
            (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
          int32_t i = (sizeof(illu.GaussMeanValue) /
                       sizeof(illu.GaussMeanValue.fCoeff[0]));
          int32_t no =
              ParseFloatArray(tag.value(), illu.GaussMeanValue.fCoeff, i);
          DCT_ASSERT((no == tag.size()));
        } else if (!strcmp(
                       pSubChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_ILLUMINATION_GMM_INV_COV_MATRIX_TAG) &&
                   (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
          int32_t i = (sizeof(illu.CovarianceMatrix) /
                       sizeof(illu.CovarianceMatrix.fCoeff[0]));
          int32_t no =
              ParseFloatArray(tag.value(), illu.CovarianceMatrix.fCoeff, i);
          DCT_ASSERT((no == tag.size()));
        } else if (
            !strcmp(pSubChild->ToElement()->Name(),
                    CALIB_SENSOR_AWB_ILLUMINATION_GMM_GAUSSIAN_SFACTOR_TAG) &&
            (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
          int32_t i =
              (sizeof(illu.GaussFactor) / sizeof(illu.GaussFactor.fCoeff[0]));
          int32_t no = ParseFloatArray(tag.value(), illu.GaussFactor.fCoeff, i);
          DCT_ASSERT((no == tag.size()));
        } else if (!strcmp(pSubChild->ToElement()->Name(),
                           CALIB_SENSOR_AWB_ILLUMINATION_GMM_TAU_TAG) &&
                   (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
          int32_t i =
              (sizeof(illu.Threshold) / sizeof(illu.Threshold.fCoeff[0]));
          int32_t no = ParseFloatArray(tag.value(), illu.Threshold.fCoeff, i);
          DCT_ASSERT((no == tag.size()));
        } else {
          TRACE(CALIB_ERR,
                "parse error in AWB gaussian mixture model section, unknown "
                "tag: %s \n",
                pSubChild->ToElement()->Name());
          REPORT(RET_FAILURE);
        }

        pSubChild = pSubChild->NextSibling();
      }
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_TAG)) {
      float *afGain = NULL;
      int32_t n_gains = 0;
      float *afSat = NULL;
      int32_t n_sats = 0;

      const XMLNode *pSubChild = pChild->ToElement()->FirstChild();

      while (pSubChild) {
        XmlTag tag = XmlTag(pSubChild->ToElement());

        if (!strcmp(pSubChild->ToElement()->Name(),
                    CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_GAIN_TAG) &&
            (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
          if (!afGain) {
            n_gains = tag.size();
            afGain = (float *)malloc((n_gains * sizeof(float)));
            DCT_ASSERT(afGain);

            MEMSET(afGain, 0, (n_gains * sizeof(float)));
          }

          int32_t no = ParseFloatArray(tag.value(), afGain, n_gains);
          DCT_ASSERT((no == n_gains));
        } else if (!strcmp(pSubChild->ToElement()->Name(),
                           CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_SAT_TAG) &&
                   (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
          if (!afSat) {
            n_sats = tag.size();
            afSat = (float *)malloc((n_sats * sizeof(float)));
            DCT_ASSERT(afSat);

            MEMSET(afSat, 0, (n_sats * sizeof(float)));
          }

          int32_t no = ParseFloatArray(tag.value(), afSat, n_sats);
          DCT_ASSERT((no == n_sats));
        } else {
          TRACE(CALIB_ERR,
                "parse error in AWB saturation curve section, unknown "
                "tag: %s \n",
                pSubChild->ToElement()->Name());
          REPORT(RET_FAILURE);
        }

        pSubChild = pSubChild->NextSibling();
      }

      DCT_ASSERT((n_gains == n_sats));
      illu.SaturationCurve.ArraySize = n_gains;
      illu.SaturationCurve.pSensorGain = afGain;
      illu.SaturationCurve.pSaturation = afSat;
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_TAG)) {
      float *afGain = NULL;
      int32_t n_gains = 0;
      float *afVig = NULL;
      int32_t n_vigs = 0;

      const XMLNode *pSubChild = pChild->ToElement()->FirstChild();

      while (pSubChild) {
        XmlTag tag = XmlTag(pSubChild->ToElement());
        if (!strcmp(pSubChild->ToElement()->Name(),
                    CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_GAIN_TAG) &&
            (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
          if (!afGain) {
            n_gains = tag.size();
            afGain = (float *)malloc((n_gains * sizeof(float)));
            DCT_ASSERT(afGain);

            MEMSET(afGain, 0, (n_gains * sizeof(float)));
          }

          int32_t no = ParseFloatArray(tag.value(), afGain, n_gains);
          DCT_ASSERT((no == n_gains));
        } else if (!strcmp(pSubChild->ToElement()->Name(),
                           CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_VIG_TAG) &&
                   (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
          if (!afVig) {
            n_vigs = tag.size();
            afVig = (float *)malloc((n_vigs * sizeof(float)));
            DCT_ASSERT(afVig);

            MEMSET(afVig, 0, (n_vigs * sizeof(float)));
          }

          int32_t no = ParseFloatArray(tag.value(), afVig, n_vigs);
          DCT_ASSERT((no == n_vigs));
        } else {
          TRACE(CALIB_ERR,
                "parse error in AWB saturation curve section, unknown "
                "tag: %s \n",
                pSubChild->ToElement()->Name());
          REPORT(RET_FAILURE);
        }

        pSubChild = pSubChild->NextSibling();
      }

      DCT_ASSERT((n_gains == n_vigs));
      illu.VignettingCurve.ArraySize = n_gains;
      illu.VignettingCurve.pSensorGain = afGain;
      illu.VignettingCurve.pVignetting = afVig;
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_ILLUMINATION_ALSC_TAG)) {
      ret = parseEntryCell(pChild->ToElement(), tag.size(),
                           &CalibDb::parseEntryAwbIlluminationAlsc, &illu);
      REPORT(ret);
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_ILLUMINATION_ACC_TAG)) {
      const XMLNode *pSubChild = pChild->ToElement()->FirstChild();
      while (pSubChild) {
        XmlTag tag = XmlTag(pSubChild->ToElement());
        if (!strcmp(pSubChild->ToElement()->Name(),
                    CALIB_SENSOR_AWB_ILLUMINATION_ACC_CC_PROFILE_LIST_TAG) &&
            (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
          const char *pValue = tag.valueToUpper();
          int32_t no =
              ParseCcProfileArray(pValue, illu.cc_profiles, CAM_NO_CC_PROFILES);
          DCT_ASSERT((no <= CAM_NO_CC_PROFILES));
          illu.cc_no = no;
        } else {
          TRACE(CALIB_ERR, "parse error in AWB aCC: %s \n",
                pSubChild->ToElement()->Name());
          REPORT(RET_FAILURE);
        }

        pSubChild = pSubChild->NextSibling();
      }
    } else {
      TRACE(CALIB_ERR,
            "parse error in AWB illumination section, unknown tag: %s \n",
            pChild->ToElement()->Name());
      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  ret = CamCalibDbAddIllumination(hCalibDb, &illu);
  DCT_ASSERT(ret == RET_SUCCESS);

  /* cleanup */
  free(illu.SaturationCurve.pSensorGain);
  free(illu.SaturationCurve.pSaturation);
  free(illu.VignettingCurve.pSensorGain);
  free(illu.VignettingCurve.pVignetting);

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryAwbIlluminationAlsc(const XMLElement *pElement,
                                               void *pParam) {

  DCT_ASSERT(pParam);

  CamIlluProfile_t *pIllu = (CamIlluProfile_t *)pParam;

  const char *pLscProfiles = NULL;
  int32_t resIdx = -1;

  const XMLNode *pChild = pElement->FirstChild();
  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());
    if (!strcmp(pChild->ToElement()->Name(),
                CALIB_SENSOR_AWB_ILLUMINATION_ALSC_RES_LSC_PROFILE_LIST_TAG)) {
      pLscProfiles = tag.valueToUpper();
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_AWB_ILLUMINATION_ALSC_RES_TAG)) {
      const char *pValue = tag.value();
      int32_t ret = CamCalibDbGetResolutionIdxByName(hCalibDb, pValue, &resIdx);
      DCT_ASSERT(ret == RET_SUCCESS);
    } else {
      TRACE(CALIB_ERR, "unknown aLSC tag: %s \n", pChild->ToElement()->Name());
      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  DCT_ASSERT(resIdx != -1);

  int32_t no = ParseLscProfileArray(pLscProfiles, pIllu->lsc_profiles[resIdx],
                                    CAM_NO_LSC_PROFILES);
  DCT_ASSERT((no <= CAM_NO_LSC_PROFILES));
  pIllu->lsc_no[resIdx] = no;

  pIllu->lsc_res_no++;

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryAwbIlluminationAcc(const XMLElement *pElement,
                                              void *pParam) {
  (void)pParam;

  const XMLNode *pChild = pElement->FirstChild();
  while (pChild) {
    if (!strcmp(pChild->ToElement()->Name(),
                CALIB_SENSOR_AWB_ILLUMINATION_ACC_CC_PROFILE_LIST_TAG)) {
    } else {
      TRACE(CALIB_ERR, "unknown aCC tag: %s \n", pChild->ToElement()->Name());
      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryLsc(const XMLElement *pElement, void *pParam) {

  CamLscProfile_t lsc_profile;
  MEMSET(&lsc_profile, 0, sizeof(lsc_profile));

  const XMLNode *pChild = pElement->FirstChild();
  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());
    if (!strcmp(pChild->ToElement()->Name(),
                CALIB_SENSOR_LSC_PROFILE_NAME_TAG) &&
        (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.valueToUpper();
      calib_string_copy(lsc_profile.name, pValue, sizeof(lsc_profile.name));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_LSC_PROFILE_RESOLUTION_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.value();
      calib_string_copy(lsc_profile.resolution, pValue, sizeof(lsc_profile.resolution));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_LSC_PROFILE_ILLUMINATION_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.valueToUpper();
      calib_string_copy(lsc_profile.illumination, pValue,
              sizeof(lsc_profile.illumination));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_LSC_PROFILE_LSC_SECTORS_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseUshortArray(tag.value(), &lsc_profile.LscSectors, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_LSC_PROFILE_LSC_NO_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseUshortArray(tag.value(), &lsc_profile.LscNo, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_LSC_PROFILE_LSC_XO_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseUshortArray(tag.value(), &lsc_profile.LscXo, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_LSC_PROFILE_LSC_YO_TAG)) {
      int32_t no = ParseUshortArray(tag.value(), &lsc_profile.LscYo, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_LSC_PROFILE_LSC_SECTOR_SIZE_X_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i = (sizeof(lsc_profile.LscXSizeTbl) /
                   sizeof(lsc_profile.LscXSizeTbl[0]));
      int32_t no = ParseUshortArray(tag.value(), lsc_profile.LscXSizeTbl, i);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_LSC_PROFILE_LSC_SECTOR_SIZE_Y_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i = (sizeof(lsc_profile.LscYSizeTbl) /
                   sizeof(lsc_profile.LscYSizeTbl[0]));
      int32_t no = ParseUshortArray(tag.value(), lsc_profile.LscYSizeTbl, i);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_LSC_PROFILE_LSC_VIGNETTING_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no =
          ParseFloatArray(tag.value(), (float *)(&lsc_profile.vignetting), 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_RED_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i =
          (sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_RED]) /
           sizeof(
               lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[0]));
      int32_t no = ParseUshortArray(
          tag.value(),
          (lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff), i);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_GREENR_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i =
          (sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR]) /
           sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR]
                      .uCoeff[0]));
      int32_t no = ParseUshortArray(
          tag.value(),
          lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff, i);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_GREENB_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i =
          (sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB]) /
           sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB]
                      .uCoeff[0]));
      int32_t no = ParseUshortArray(
          tag.value(),
          (uint16_t *)(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB]
                           .uCoeff),
          i);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_BLUE_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i =
          (sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE]) /
           sizeof(
               lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[0]));
      int32_t no = ParseUshortArray(
          tag.value(),
          (uint16_t *)(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE]
                           .uCoeff),
          i);
      DCT_ASSERT((no == tag.size()));
    } else {
      TRACE(CALIB_ERR, "parse error in LSC section, unknown tag: %s \n",
            pChild->ToElement()->Name());

      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  int32_t ret = CamCalibDbAddLscProfile(hCalibDb, &lsc_profile);
  DCT_ASSERT(ret == RET_SUCCESS);

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryCc(const XMLElement *pElement, void *pParam) {
  (void)pParam;

  CamCcProfile_t cc_profile;
  MEMSET(&cc_profile, 0, sizeof(cc_profile));

  const XMLNode *pChild = pElement->FirstChild();
  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());
    if (!strcmp(pChild->ToElement()->Name(),
                CALIB_SENSOR_CC_PROFILE_NAME_TAG) &&
        (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.valueToUpper();
      calib_string_copy(cc_profile.name, pValue, sizeof(cc_profile.name));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_CC_PROFILE_SATURATION_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseFloatArray(tag.value(), &cc_profile.saturation, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_CC_PROFILE_CC_MATRIX_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i = (sizeof(cc_profile.CrossTalkCoeff) /
                   sizeof(cc_profile.CrossTalkCoeff.fCoeff[0]));
      int32_t no =
          ParseFloatArray(tag.value(), cc_profile.CrossTalkCoeff.fCoeff, i);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_CC_PROFILE_CC_OFFSETS_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i = (sizeof(cc_profile.CrossTalkOffset) /
                   sizeof(cc_profile.CrossTalkOffset.fCoeff[0]));
      int32_t no =
          ParseFloatArray(tag.value(), cc_profile.CrossTalkOffset.fCoeff, i);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_CC_PROFILE_WB_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i = (sizeof(cc_profile.ComponentGain) /
                   sizeof(cc_profile.ComponentGain.fCoeff[0]));
      int32_t no =
          ParseFloatArray(tag.value(), cc_profile.ComponentGain.fCoeff, i);
      DCT_ASSERT((no == tag.size()));
    } else {
      TRACE(CALIB_ERR, "parse error in CC section, unknown tag: %s \n",
            pChild->ToElement()->Name());
      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  int32_t ret = CamCalibDbAddCcProfile(hCalibDb, &cc_profile);
  REPORT(ret);

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryBls(const XMLElement *pElement, void *pParam) {
  (void)pParam;

  CamBlsProfile_t bls_profile;
  MEMSET(&bls_profile, 0, sizeof(bls_profile));

  const XMLNode *pChild = pElement->FirstChild();

  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());
    if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_BLS_NAME_TAG) &&
        (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.valueToUpper();
      calib_string_copy(bls_profile.name, pValue, sizeof(bls_profile.name));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_BLS_RESOLUTION_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.value();
      calib_string_copy(bls_profile.resolution, pValue, sizeof(bls_profile.resolution));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_BLS_DATA_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i =
          (sizeof(bls_profile.level) / sizeof(bls_profile.level.uCoeff[0]));
      int32_t no = ParseUshortArray(tag.value(), bls_profile.level.uCoeff, i);
      DCT_ASSERT((no == tag.size()));
    } else {
      TRACE(CALIB_ERR, "parse error in BLS section, unknown tag: %s \n",
            pChild->ToElement()->Name());

      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  int32_t ret = CamCalibDbAddBlsProfile(hCalibDb, &bls_profile);
  REPORT(ret);

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryCac(const XMLElement *pElement, void *pParam) {
  (void)pParam;

  CamCacProfile_t cac_profile;
  MEMSET(&cac_profile, 0, sizeof(cac_profile));

  const XMLNode *pChild = pElement->FirstChild();

  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());
    if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_CAC_NAME_TAG) &&
        (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.valueToUpper();
      calib_string_copy(cac_profile.name, pValue, sizeof(cac_profile.name));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_CAC_RESOLUTION_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.value();
      calib_string_copy(cac_profile.resolution, pValue, sizeof(cac_profile.resolution));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SESNOR_CAC_X_NORMSHIFT_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseByteArray(tag.value(), &cac_profile.x_ns, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SESNOR_CAC_X_NORMFACTOR_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseByteArray(tag.value(), &cac_profile.x_nf, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SESNOR_CAC_Y_NORMSHIFT_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseByteArray(tag.value(), &cac_profile.y_ns, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SESNOR_CAC_Y_NORMFACTOR_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseByteArray(tag.value(), &cac_profile.y_nf, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SESNOR_CAC_X_OFFSET_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseShortArray(tag.value(), &cac_profile.hCenterOffset, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SESNOR_CAC_Y_OFFSET_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no = ParseShortArray(tag.value(), &cac_profile.vCenterOffset, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SESNOR_CAC_RED_PARAMETERS_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i = (sizeof(cac_profile.Red) / sizeof(cac_profile.Red.fCoeff[0]));
      int32_t no = ParseFloatArray(tag.value(), cac_profile.Red.fCoeff, i);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SESNOR_CAC_BLUE_PARAMETERS_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t i =
          (sizeof(cac_profile.Blue) / sizeof(cac_profile.Blue.fCoeff[0]));
      int32_t no = ParseFloatArray(tag.value(), cac_profile.Blue.fCoeff, i);
      DCT_ASSERT((no == tag.size()));
    } else {
      TRACE(CALIB_ERR, "parse error in CAC section, unknown tag: %s \n",
            pChild->ToElement()->Name());

      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  int32_t ret = CamCalibDbAddCacProfile(hCalibDb, &cac_profile);
  REPORT(ret);

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryDpf(const XMLElement *pElement, void *pParam) {
  (void)pParam;

  CamDpfProfile_t dpf_profile;
  MEMSET(&dpf_profile, 0, sizeof(dpf_profile));

  const XMLNode *pChild = pElement->FirstChild();
  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());

    if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_DPF_NAME_TAG) &&
        (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.valueToUpper();
      calib_string_copy(dpf_profile.name, pValue, sizeof(dpf_profile.name));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_DPF_RESOLUTION_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.value();
      calib_string_copy(dpf_profile.resolution, pValue, sizeof(dpf_profile.resolution));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_DPF_NLL_SEGMENTATION_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      int32_t no =
          ParseUshortArray(tag.value(), &dpf_profile.nll_segmentation, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_DPF_NLL_COEFF_TAG)) {
      int32_t i = (sizeof(dpf_profile.nll_coeff) /
                   sizeof(dpf_profile.nll_coeff.uCoeff[0]));
      int32_t no =
          ParseUshortArray(tag.value(), dpf_profile.nll_coeff.uCoeff, i);
      DCT_ASSERT((no == tag.size()));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_DPF_SIGMA_GREEN_TAG)) {
      int32_t no = ParseUshortArray(tag.value(), &dpf_profile.SigmaGreen, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_DPF_SIGMA_RED_BLUE_TAG)) {
      int32_t no = ParseUshortArray(tag.value(), &dpf_profile.SigmaRedBlue, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_DPF_GRADIENT_TAG)) {
      int32_t no = ParseFloatArray(tag.value(), &dpf_profile.fGradient, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_DPF_OFFSET_TAG)) {
      int32_t no = ParseFloatArray(tag.value(), &dpf_profile.fOffset, 1);
      DCT_ASSERT((no == 1));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_DPF_NLGAINS_TAG)) {
      int32_t i =
          (sizeof(dpf_profile.NfGains) / sizeof(dpf_profile.NfGains.fCoeff[0]));
      int32_t no = ParseFloatArray(tag.value(), dpf_profile.NfGains.fCoeff, i);
      DCT_ASSERT((no == tag.size()));
    } else {
      TRACE(CALIB_ERR, "parse error in DPF section, unknown tag: %s \n",
            pChild->ToElement()->Name());

      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  int32_t ret = CamCalibDbAddDpfProfile(hCalibDb, &dpf_profile);
  REPORT(ret);

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryDpcc(const XMLElement *pElement, void *pParam) {
  (void)pParam;

  int32_t ret = RET_SUCCESS;

  CamDpccProfile_t dpcc_profile;
  MEMSET(&dpcc_profile, 0, sizeof(dpcc_profile));

  const XMLNode *pChild = pElement->FirstChild();
  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());

    if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_DPCC_NAME_TAG) &&
        (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.valueToUpper();
      calib_string_copy(dpcc_profile.name, pValue, sizeof(dpcc_profile.name));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_DPCC_RESOLUTION_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      const char *pValue = tag.value();
      calib_string_copy(dpcc_profile.resolution, pValue, sizeof(dpcc_profile.resolution));
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_DPCC_REGISTER_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CELL)) && (tag.size() > 0)) {
      ret = parseEntryCell(pChild->ToElement(), tag.size(),
                           &CalibDb::parseEntryDpccRegisters, &dpcc_profile);
      REPORT(ret);
    } else {
      TRACE(CALIB_ERR, "parse error in DPCC section, unknown tag: %s \n",
            pChild->ToElement()->Name());

      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  ret = CamCalibDbAddDpccProfile(hCalibDb, &dpcc_profile);
  REPORT(ret);

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryDpccRegisters(const XMLElement *pElement,
                                         void *pParam) {
  (void)pParam;

  CamDpccProfile_t *pDpcc_profile = (CamDpccProfile_t *)pParam;

  std::string reg_name;
  uint32_t reg_value = 0U;

  const XMLNode *pChild = pElement->FirstChild();
  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());

    if (!strcmp(pChild->ToElement()->Name(),
                CALIB_SENSOR_DPCC_REGISTER_NAME_TAG) &&
        (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      reg_name = tag.valueToUpper();
    } else if (!strcmp(pChild->ToElement()->Name(),
                       CALIB_SENSOR_DPCC_REGISTER_VALUE_TAG) &&
               (tag.isType(XmlTag::TAG_TYPE_CHAR)) && (tag.size() > 0)) {
      reg_value = tag.valueToUInt();
    } else {
      TRACE(CALIB_ERR, "parse error in DPCC section, unknown tag: %s \n",
            pChild->ToElement()->Name());

      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_MODE) {
    pDpcc_profile->isp_dpcc_mode = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_OUTPUT_MODE) {
    pDpcc_profile->isp_dpcc_output_mode = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_SET_USE) {
    pDpcc_profile->isp_dpcc_set_use = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_METHODS_SET_1) {
    pDpcc_profile->isp_dpcc_methods_set_1 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_METHODS_SET_2) {
    pDpcc_profile->isp_dpcc_methods_set_2 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_METHODS_SET_3) {
    pDpcc_profile->isp_dpcc_methods_set_3 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_THRESH_1) {
    pDpcc_profile->isp_dpcc_line_thresh_1 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_MAD_FAC_1) {
    pDpcc_profile->isp_dpcc_line_mad_fac_1 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_PG_FAC_1) {
    pDpcc_profile->isp_dpcc_pg_fac_1 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_THRESH_1) {
    pDpcc_profile->isp_dpcc_rnd_thresh_1 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RG_FAC_1) {
    pDpcc_profile->isp_dpcc_rg_fac_1 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_THRESH_2) {
    pDpcc_profile->isp_dpcc_line_thresh_2 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_MAD_FAC_2) {
    pDpcc_profile->isp_dpcc_line_mad_fac_2 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_PG_FAC_2) {
    pDpcc_profile->isp_dpcc_pg_fac_2 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_THRESH_2) {
    pDpcc_profile->isp_dpcc_rnd_thresh_2 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RG_FAC_2) {
    pDpcc_profile->isp_dpcc_rg_fac_2 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_THRESH_3) {
    pDpcc_profile->isp_dpcc_line_thresh_3 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_MAD_FAC_3) {
    pDpcc_profile->isp_dpcc_line_mad_fac_3 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_PG_FAC_3) {
    pDpcc_profile->isp_dpcc_pg_fac_3 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_THRESH_3) {
    pDpcc_profile->isp_dpcc_rnd_thresh_3 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RG_FAC_3) {
    pDpcc_profile->isp_dpcc_rg_fac_3 = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RO_LIMITS) {
    pDpcc_profile->isp_dpcc_ro_limits = reg_value;
  } else if (reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_OFFS) {
    pDpcc_profile->isp_dpcc_rnd_offs = reg_value;
  } else {
    TRACE(CALIB_ERR, "unknown DPCC register: %s \n", reg_name.c_str());
  }

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntryHdr(const XMLElement *pElement, void *pParam) {
  (void)pParam;

  const XMLNode *pChild = pElement->FirstChild();
  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());
    if (!strcmp(pChild->ToElement()->Name(), CALIB_SENSOR_HDR_CURVE_TAG) &&
        (tag.isType(XmlTag::TAG_TYPE_DOUBLE)) && (tag.size() > 0)) {
      camdev::CalibHdr &hdr = pCalibration->module<camdev::CalibHdr>();

      int32_t count = sizeof(hdr.sensor.curve) / sizeof(hdr.sensor.curve[0]);

      count = ParseFloatArray(tag.value(), hdr.sensor.curve, count);

      DCT_ASSERT((count == tag.size()));
    } else {
      TRACE(CALIB_ERR, "parse error in HDR section, unknown tag: %s \n",
            pChild->ToElement()->Name());
      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  return RET_SUCCESS;
}

int32_t CalibDb::parseEntrySystem(const XMLElement *pElement, void *pParam) {
  (void)pParam;

  CamCalibSystemData_t system_data;
  MEMSET(&system_data, 0, sizeof(CamCalibSystemData_t));

  const XMLNode *pChild = pElement->FirstChild();
  while (pChild) {
    XmlTag tag = XmlTag(pChild->ToElement());

    if (!strcmp(pChild->ToElement()->Name(), CALIB_SYSTEM_AFPS_TAG)) {
      const XMLNode *pFirstChild = pChild->ToElement()->FirstChild();
      if (pFirstChild) {
        XmlTag firstTag = XmlTag(pFirstChild->ToElement());
        if (!strcmp(pFirstChild->ToElement()->Name(),
                    CALIB_SYSTEM_AFPS_DEFAULT_TAG) &&
            (firstTag.isType(XmlTag::TAG_TYPE_CHAR)) && (firstTag.size() > 0)) {
          tag = XmlTag(pFirstChild->ToElement());

          const char *pValue = tag.value();

          system_data.AfpsDefault =
              (strstr(pValue, "on")) ? BOOL_TRUE : BOOL_FALSE;
        }
      }
    } else {
      TRACE(CALIB_ERR, "parse error in system section, unknown tag: %s \n",
            pChild->ToElement()->Name());

      REPORT(RET_FAILURE);
    }

    pChild = pChild->NextSibling();
  }

  int32_t ret = CamCalibDbSetSystemData(hCalibDb, &system_data);
  REPORT(ret);

  return RET_SUCCESS;
}
