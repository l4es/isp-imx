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
#include "meta_builder.hpp"
#include "json_helper.h"

void addRect(const struct isp_rect& rc,  Json::Value& node, const char *name) {
    node[name][0] = rc.x;
    node[name][1] = rc.y;
    node[name][2] = rc.w;
    node[name][3] = rc.h;
}

bool getKeyByValue(const viv_metadata_tag& tag, int val, std::string &ret) {
    ret = "null";
    for (auto& item : tag) {
        if (item.second == val) {
            ret = item.first;
            return true;
        }
    }
    return false;
}

bool buildMetadata(struct isp_metadata *meta, Json::Value& root) {
    std::string strKey;
    // exp
    Json::Value node;
    node.clear();
    node["enable"] = meta->exp.enable;
    getKeyByValue(VIV_METADATA_EXP_MODE, meta->exp.mode, strKey);
    node["mode"] = strKey;
    addRect(meta->exp.rc, node, "rect");
    addArray(meta->exp.mean, node, "mean");
    root[SECTION_NAME_EXP] = node;

    // exp2, optional because of too large.
    if (meta->exp2.enable) {
        node.clear();
        node["enable"] = meta->awb.enable;
        addRect(meta->exp2.rc, node, "rect");
        addArray(meta->exp2.mean, node, "mean");
        node["weight"][0] = meta->exp2.r;
        node["weight"][1] = meta->exp2.gr;
        node["weight"][2] = meta->exp2.gb;
        node["weight"][3] = meta->exp2.b;
        root[SECTION_NAME_EXP2] = node;
    }

    // awb
    node.clear();
    node["enable"] = meta->awb.enable;
    addRect(meta->awb.rc, node, "rect");
    getKeyByValue(VIV_METADATA_AWB_MODE, meta->awb.mode, strKey);
    node["mode"] = strKey;
    node["gain"][0] = meta->awb.gain_r;
    node["gain"][1] = meta->awb.gain_gr;
    node["gain"][2] = meta->awb.gain_gb;
    node["gain"][3] = meta->awb.gain_b;
    node["mean"][0] = meta->awb.r;
    node["mean"][1] = meta->awb.g;
    node["mean"][2] = meta->awb.b;
    node["mean"][3] = meta->awb.no_white_count;
    root[SECTION_NAME_AWB] = node;

    // afm
    node.clear();
    node["enable"] = meta->afm.enable;
    addRect(meta->afm.rc[0], node, "window 0");
    addRect(meta->afm.rc[1], node, "window 1");
    addRect(meta->afm.rc[2], node, "window 2");
    node["sum"][0] = meta->afm.sum_a;
    node["sum"][1] = meta->afm.sum_b;
    node["sum"][2] = meta->afm.sum_c;
    node["lum"][0] = meta->afm.lum_a;
    node["lum"][1] = meta->afm.lum_b;
    node["lum"][2] = meta->afm.lum_c;
    root[SECTION_NAME_AFM] = node;

    // vsm
    node.clear();
    node["enable"] = meta->vsm.enable;
    addRect(meta->vsm.rc, node, "rect");
    node["h_seg"] = meta->vsm.h_seg;
    node["v_seg"] = meta->vsm.v_seg;
    node["x"] = meta->vsm.x;
    node["y"] = meta->vsm.y;
    root[SECTION_NAME_VSM] = node;

    // hist
    node.clear();
    node["enable"] = meta->hist.enable;
    node["type"] = meta->hist.type;
    getKeyByValue(VIV_METADATA_HIST_MODE, meta->hist.mode, strKey);
    node["mode"] = strKey;
    addRect(meta->hist.rc, node, "rect");
    addArray(meta->hist.mean, node, "mean");
    root[SECTION_NAME_HIST] = node;

    return true;
}