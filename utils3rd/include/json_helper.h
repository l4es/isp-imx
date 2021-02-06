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

#ifndef DEVELOPER_MEDIACONTROL_INCLUDE_JSON_HELPER_H_
#define DEVELOPER_MEDIACONTROL_INCLUDE_JSON_HELPER_H_

#include <string>
#include <typeinfo>
#include <functional>
#include <type_traits>

#include "json/json.h"

template<typename T>  // It's better to auto check type by compiler.
inline void readValueFromNode(const Json::Value& node, T& ret) {
    // int uint8_t int8_t uint16_t int16_t int32_t bool
    ret = static_cast<T>(node.asUInt64());
}

template< >
inline void readValueFromNode(const Json::Value& node, int& ret) {
    ret = node.asInt();
}

template< >
inline void readValueFromNode(const Json::Value& node, uint32_t& ret) {
    ret = node.asUInt();
}

template< >
inline void readValueFromNode(const Json::Value& node, int64_t& ret) {
    ret = node.asInt64();
}

template< >
inline void readValueFromNode(const Json::Value& node, bool& ret) {
    ret = node.asBool();
}

template< >
inline void readValueFromNode(const Json::Value& node, float& ret) {
    ret = node.asFloat();
}

template< >
inline void readValueFromNode(const Json::Value& node, double& ret) {
    ret = node.asDouble();
}

template< >
inline void readValueFromNode(const Json::Value& node, std::string& ret) {
    ret = node.asCString();
}

template<typename T>
void readValueByName(const Json::Value& node, const char* szName, T& ret) {
    Json::Value item = node[szName];
    if (!item.isNull())
        readValueFromNode(item, ret);
}

template<typename T>   // we should not write multi-functions for every array type of camera engine,
void readArrayFromNode(const Json::Value& node, const char* szName, T* array) {
    Json::Value curve = node[szName];
    if (curve.isArray()) {
        int i = 0;
        for (auto& item : curve) {
            readValueFromNode(item, array[i++]);
        }
    }
}

template<typename T>  // can be extend to float/double type.
void printIntArary(T* array, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}

template<typename T>
void writeArrayToNode(const T *array, Json::Value& node, const char *section, int size) {
    for (int i = 0; i < size; i ++) {
        node[section][i] = array[i];
    }
}

#define JH_GET_TYPE(x) std::remove_reference<decltype((x))>::type
#define JH_READ_VAL(x, y, z) readValueByName<JH_GET_TYPE((x))>((y), (z), (x));
#define JH_READ_ARRAY(x, y, z) readArrayFromNode<JH_GET_TYPE((*z))>((x), (y), (z));

#define addArray(x, y, z) writeArrayToNode<JH_GET_TYPE((x)[0])>(x, y, z, sizeof(x)/sizeof((x)[0]));

#endif  // DEVELOPER_MEDIACONTROL_INCLUDE_JSON_HELPER_H_
