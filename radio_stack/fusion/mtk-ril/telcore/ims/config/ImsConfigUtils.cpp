/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "ImsConfigUtils.h"
#include "rfx_properties.h"
#include "RfxBasics.h"
#include "RfxRilUtils.h"
#include "RtcImsConfigDef.h"

#include <sstream>
#include <vector>
#include <string>

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
#define RFX_LOG_TAG "TCL-ImsConfigUtils"

const char* ImsConfigUtils::PROPERTY_IMS_SUPPORT = "persist.vendor.ims_support";
const char* ImsConfigUtils::PROPERTY_VOLTE_ENALBE = "persist.vendor.mtk.volte.enable";
const char* ImsConfigUtils::PROPERTY_WFC_ENALBE = "persist.vendor.mtk.wfc.enable";
const char* ImsConfigUtils::PROPERTY_VILTE_ENALBE = "persist.vendor.mtk.vilte.enable";
const char* ImsConfigUtils::PROPERTY_VIWIFI_ENALBE = "persist.vendor.mtk.viwifi.enable";
const char* ImsConfigUtils::PROPERTY_VONR_ENALBE = "persist.vendor.mtk.vonr.enable";
const char* ImsConfigUtils::PROPERTY_VINR_ENALBE = "persist.vendor.mtk.vinr.enable";
const char* ImsConfigUtils::PROPERTY_IMS_VIDEO_ENALBE = "persist.vendor.mtk.ims.video.enable";
const char* ImsConfigUtils::PROPERTY_MULTI_IMS_SUPPORT = "persist.vendor.mims_support";
const char* ImsConfigUtils::PROPERTY_DYNAMIC_IMS_SWITCH_SUPPORT =
        "persist.vendor.mtk_dynamic_ims_switch";
const char* ImsConfigUtils::PROPERTY_CTVOLTE_ENABLE = "persist.vendor.mtk_ct_volte_support";

static ConfigOperatorTable operatorTable[] = {
#include "op/OperatorTable.h"
};

const char ImsConfigUtils::PROPERTY_ICCID_SIM[4][25] = {
        "vendor.ril.iccid.sim1", "vendor.ril.iccid.sim2", "vendor.ril.iccid.sim3",
        "vendor.ril.iccid.sim4"};

const char ImsConfigUtils::PROPERTY_TEST_SIM[4][30] = {
        "vendor.gsm.sim.ril.testsim", "vendor.gsm.sim.ril.testsim.2",
        "vendor.gsm.sim.ril.testsim.3", "vendor.gsm.sim.ril.testsim.4"};

const bool ImsConfigUtils::DEBUG_ENABLED = false;
bool ImsConfigUtils::mForceNotify[4] = {false, false, false, false};
int ImsConfigUtils::mForceValue[4] = {0, 0, 0, 0};

int ImsConfigUtils::getSystemPropValue(const char* propName) {
    char value[RFX_PROPERTY_VALUE_MAX] = {0};
    int result;

    rfx_property_get(propName, value, "0");
    result = atoi(value);

    printLog(DEBUG, String8::format("getSystemPropValue, propName:%s, value:%s", propName, value),
             0);

    return result;
}

std::string ImsConfigUtils::getSystemPropStringValue(const char* propName) {
    char value[RFX_PROPERTY_VALUE_MAX] = {0};

    rfx_property_get(propName, value, "");

    return std::string(value);
}

int ImsConfigUtils::getFeaturePropValue(const char* propName, int slot_id) {
    RFX_UNUSED(slot_id);
    int defaultPropValue = 0;
    int featureValue = 0;
    int propResult = 0;
    char value[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get(propName, value, "0");
    std::stringstream ss;
    ss << value;
    ss >> featureValue;

    if (!checkIsPhoneIdValid(slot_id)) {
        printLog(
                INFO,
                String8::format("getFeaturePropValue(): = %s, slot_id invalid return default value",
                                propName),
                slot_id);

        return defaultPropValue;
    }
    if (isMultiImsSupport()) {
        propResult = ((featureValue & (1 << slot_id)) > 0) ? 1 : 0;
    } else {
        // Backward compatibility, only use phone Id 0 to get.
        propResult = ((featureValue & (1 << 0)) > 0) ? 1 : 0;
    }

    printLog(INFO,
             String8::format("getFeaturePropValue() featureValue:%d, propName:%s, propResult:%d",
                             featureValue, propName, propResult),
             slot_id);

    return propResult;
}

void ImsConfigUtils::setFeaturePropValue(const char* propName, char* enabled, int slot_id) {
    int defaultPropValue = 0;
    int featureValue = 0;
    int enabledValue = 0;
    int sumFeatureValue = 0;
    char value[RFX_PROPERTY_VALUE_MAX] = {0};
    std::stringstream ss;
    std::string sumFeatureVal = "";

    rfx_property_get(propName, value, "0");

    ss << value;
    ss >> featureValue;

    if (!checkIsPhoneIdValid(slot_id)) {
        printLog(INFO,
                 String8::format(
                         "SetFeaturePropValue() propName:%s, slot_id invalid don't set and return",
                         propName),
                 slot_id);

        return;
    }
    ss.clear();
    ss.str("");
    ss << enabled;
    ss >> enabledValue;
    if (isMultiImsSupport()) {
        sumFeatureValue = setBitForPhone(featureValue, enabledValue, slot_id);
    } else {
        // Backward compatibility, only use phone Id 0 to save.
        sumFeatureValue = setBitForPhone(featureValue, enabledValue, 0);
    }

    ss.clear();
    ss.str("");
    ss << sumFeatureValue;
    ss >> sumFeatureVal;
    rfx_property_set(propName, sumFeatureVal.c_str());

    printLog(DEBUG,
             String8::format("SetFeaturePropValue() featureValue:%d, propName:%s, "
                             "sumFeatureValue:%d, enabledValue:%d",
                             featureValue, propName, sumFeatureValue, enabledValue),
             slot_id);
}

int ImsConfigUtils::setBitForPhone(int featureValue, int enabled, int slot_id) {
    int result;
    if (enabled == 1) {
        result = featureValue | (1 << slot_id);
    } else {
        result = featureValue & ~(1 << slot_id);
    }
    return result;
}

bool ImsConfigUtils::checkIsPhoneIdValid(int slot_id) {
    if (isMultiImsSupport()) {
        if (slot_id > 3 || slot_id < 0) {
            printLog(INFO,
                     String8::format("Multi IMS support but phone id invalid, slot_id:%d", slot_id),
                     slot_id);
            return false;
        }
    } else {
        if (slot_id > 3 || slot_id < 0) {
            printLog(INFO,
                     String8::format("Multi IMS not support but phone id invalid, slot_id:%d",
                                     slot_id),
                     slot_id);
            return false;
        }
    }
    return true;
}

bool ImsConfigUtils::isMultiImsSupport() {
    char value[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get(PROPERTY_MULTI_IMS_SUPPORT, value, "1");
    std::stringstream ss;
    int val = 0;
    ss << value;
    ss >> val;

    if (val == 1) {
        return false;
    } else {
        return true;
    }
}

bool ImsConfigUtils::isPhoneIdSupportIms(int slot_id) {
    char imsSupport[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get(PROPERTY_IMS_SUPPORT, imsSupport, "1");

    if (atoi(imsSupport) == 0) {
        return false;
    } else {
        char msimCount[RFX_PROPERTY_VALUE_MAX] = {0};
        rfx_property_get(PROPERTY_MULTI_IMS_SUPPORT, msimCount, "1");
        int count = atoi(msimCount);
        if (count == 1) {
            printLog(DEBUG,
                     String8::format("isPhoneIdSupportIms, msimcount:%d, major slot:%d", count,
                                     RfxRilUtils::getMajorSim() - 1),
                     slot_id);
            return slot_id == RfxRilUtils::getMajorSim() - 1;
        } else {
            int protocalStackId = RfxRilUtils::getProtocolStackId(slot_id);
            printLog(DEBUG,
                     String8::format("isPhoneIdSupportIms, msimcount:%d, protocalStackId:%d", count,
                                     protocalStackId),
                     slot_id);
            return protocalStackId <= count;
        }
    }
}

bool ImsConfigUtils::isTestSim(int slot_id) {
    return getSystemPropValue(PROPERTY_TEST_SIM[slot_id]) == 1;
}

int ImsConfigUtils::getOperatorId(int mccmnc) {
    int len = sizeof(operatorTable) / sizeof(operatorTable[0]);
    for (int i = 0; i < len; i++) {
        if (operatorTable[i].mccmnc == mccmnc) {
            return operatorTable[i].opId;
        }
    }
    return Operator::OP_NONE;
}

bool ImsConfigUtils::isCtVolteDisabled(int mccmnc) {
    int op_id = getOperatorId(mccmnc);
    // NOTE that PROPERTY_CTVOLTE_ENABLE can be 0/1/2, 2 means hVOLTE enabled
    return op_id == Operator::OP_09 && getSystemPropValue(PROPERTY_CTVOLTE_ENABLE) == 0;
}

void ImsConfigUtils::printLog(int level, String8 log, int slot_id) {
    if (DEBUG_ENABLED) {
        RfxRilUtils::printLog(level, String8::format("%s", RFX_LOG_TAG), log, slot_id);
    }
}

bool ImsConfigUtils::isAllowForceNotify(int slot_id, int value) {
    if (value == mForceValue[slot_id]) {
        return mForceNotify[slot_id];
    }
    return false;
}

void ImsConfigUtils::setAllowForceNotify(int slot_id, bool allow, int value) {
    mForceNotify[slot_id] = allow;
    mForceValue[slot_id] = value;
    printLog(DEBUG, String8::format("setAllowForceNotify: mForceNotify[%d] = %d", slot_id, allow),
             slot_id);
}
