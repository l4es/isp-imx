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

#ifndef DEVELOPER_DEWARP_PARSER_DEWARPCONFIGPARSER_H_
#define DEVELOPER_DEWARP_PARSER_DEWARPCONFIGPARSER_H_

#include <vector>
#include <string>
#include <map>
#include "json/json.h"
#include "dewarpdev.h"

bool parseJsonFile(const char* pFileName, std::string& inputImageFile, std::string& outputImageFile,
                std::vector<uint32_t>& usermap, dewarp_distortion_map* dmap,
                dewarp_parameters& params, uint64_t& goldenHashKey, std::string& hashFile);

bool dweParseJsonNode(Json::Value& rootNode, std::vector<uint32_t>& usermap, dewarp_distortion_map* dmap, dewarp_parameters& params);

void updateJsonHash(const char* pFileName, const char* hash);

#endif  // DEVELOPER_DEWARP_PARSER_DEWARPCONFIGPARSER_H_
