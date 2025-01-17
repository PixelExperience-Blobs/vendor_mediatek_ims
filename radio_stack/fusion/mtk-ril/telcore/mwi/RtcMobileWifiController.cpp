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
#include <vector>
#include "RtcMobileWifiController.h"
#include "RfxRootController.h"

#include "RfxIntsData.h"
#include "RfxStringsData.h"
#include "RfxVoidData.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

#define RFX_LOG_TAG "RtcMwi"

RFX_IMPLEMENT_CLASS("RtcMobileWifiController", RtcMobileWifiController, RfxController);

RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_URC_WFC_PDN_STATE);

RtcMobileWifiController::RtcMobileWifiController() {}

RtcMobileWifiController::~RtcMobileWifiController() {}

void RtcMobileWifiController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();
    logD(RFX_LOG_TAG, "[onInit]");
    const int request_id_list[] = {
            RFX_MSG_REQUEST_SET_WIFI_ENABLED, RFX_MSG_REQUEST_SET_WIFI_ASSOCIATED,
            RFX_MSG_REQUEST_SET_WIFI_SIGNAL_LEVEL, RFX_MSG_REQUEST_SET_GEO_LOCATION,
            RFX_MSG_REQUEST_SET_WIFI_IP_ADDRESS, RFX_MSG_REQUEST_SET_EMERGENCY_ADDRESS_ID,
            RFX_MSG_REQUEST_SET_NATT_KEEP_ALIVE_STATUS, RFX_MSG_REQUEST_SET_WIFI_PING_RESULT,
            /// M: Notify ePDG screen state
            RFX_MSG_REQUEST_NOTIFY_EPDG_SCREEN_STATE};

    const int urc_id_list[] = {RFX_MSG_UNSOL_MOBILE_WIFI_ROVEOUT,
                               RFX_MSG_UNSOL_MOBILE_WIFI_HANDOVER,
                               RFX_MSG_UNSOL_ACTIVE_WIFI_PDN_COUNT,
                               RFX_MSG_UNSOL_WIFI_RSSI_MONITORING_CONFIG,
                               RFX_MSG_UNSOL_WIFI_PDN_ERROR,
                               RFX_MSG_UNSOL_REQUEST_GEO_LOCATION,
                               RFX_MSG_UNSOL_NATT_KEEP_ALIVE_CHANGED,
                               RFX_MSG_UNSOL_WIFI_PING_REQUEST,
                               RFX_MSG_UNSOL_WIFI_PDN_OOS};

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    registerToHandleRequest(request_id_list, sizeof(request_id_list) / sizeof(const int), DEFAULT);
    registerToHandleUrc(urc_id_list, sizeof(urc_id_list) / sizeof(const int));

    getStatusManager()->registerStatusChanged(
            RFX_STATUS_KEY_WFC_STATE,
            RfxStatusChangeCallback(this, &RtcMobileWifiController::onWFCPDNStatusChanged));
}

void RtcMobileWifiController::onDeinit() {
    logD(RFX_LOG_TAG, "onDeinit");

    getStatusManager()->unRegisterStatusChanged(
            RFX_STATUS_KEY_WFC_STATE,
            RfxStatusChangeCallback(this, &RtcMobileWifiController::onWFCPDNStatusChanged));

    RfxController::onDeinit();
}

bool RtcMobileWifiController::onHandleRequest(const sp<RfxMessage>& message) {
    requestToMcl(message);
    return true;
}

bool RtcMobileWifiController::onHandleUrc(const sp<RfxMessage>& message) {
    responseToRilj(message);
    return true;
}

bool RtcMobileWifiController::onHandleResponse(const sp<RfxMessage>& response) {
    responseToRilj(response);
    return true;
}

bool RtcMobileWifiController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
                                                     bool isModemPowerOff, int radioState) {
    /* Reject the request when modem off */
    if (isModemPowerOff == true) {
        logD(RFX_LOG_TAG, "onCheckIfRejectMessage, id = %d, isModemPowerOff = %d, rdioState = %d",
             message->getId(), isModemPowerOff, radioState);
        return true;
    }

    return false;
}

void RtcMobileWifiController::onWFCPDNStatusChanged(RfxStatusKeyEnum key, RfxVariant old_value,
                                                    RfxVariant new_value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    int state = new_value.asInt();
    sp<RfxMessage> msg;
    std::vector<int> params;
    params.push_back(state);
    params.push_back(getSlotId());

    logD(RFX_LOG_TAG, "onWFCPDNStatusChanged() state:%d simIdx:%d", params.at(0), params.at(1));
    msg = RfxMessage::obtainUrc(params.at(1), RFX_MSG_URC_WFC_PDN_STATE,
                                RfxIntsData(params.data(), (int)params.size()));
    RtcMobileWifiController::onHandleUrc(msg);
}
