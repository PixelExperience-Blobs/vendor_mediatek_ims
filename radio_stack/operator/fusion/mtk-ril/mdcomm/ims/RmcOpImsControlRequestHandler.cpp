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

// MTK fusion include
#include <vector>

#include "RfxVoidData.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RfxIntsData.h"
#include "RfxMessageId.h"

// Ims Op Control Local include
#include "RmcOpImsControlRequestHandler.h"
#include "RmcOpImsControlInterface.h"

#include "cc/RmcOpCallControlCommonRequestHandler.h"

#define RFX_LOG_TAG "RmcOpImsControl"

// register handler to channel
RFX_IMPLEMENT_OP_HANDLER_CLASS(RmcOpImsControlRequestHandler, RIL_CMD_PROXY_1);

// register request to RfxData
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SET_DIGITS_LINE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxVoidData, RFX_MSG_REQUEST_SET_DIGITS_REG_STATUS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SWITCH_RCS_ROI_STATUS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData,
                                RFX_MSG_REQUEST_UPDATE_RCS_CAPABILITIES);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_UPDATE_RCS_SESSION_INFO);

static const int requests[] = {
        RFX_MSG_REQUEST_SET_DIGITS_LINE, RFX_MSG_REQUEST_SET_DIGITS_REG_STATUS,
        RFX_MSG_REQUEST_SWITCH_RCS_ROI_STATUS, RFX_MSG_REQUEST_UPDATE_RCS_CAPABILITIES,
        RFX_MSG_REQUEST_UPDATE_RCS_SESSION_INFO};

RmcOpImsControlRequestHandler::RmcOpImsControlRequestHandler(int slot_id, int channel_id)
    : RfxBaseHandler(slot_id, channel_id) {
    // register to handle request
    registerToHandleRequest(requests, sizeof(requests) / sizeof(int));
}

RmcOpImsControlRequestHandler::~RmcOpImsControlRequestHandler() {}

void RmcOpImsControlRequestHandler::onHandleTimer() {}

void RmcOpImsControlRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    int requestId = msg->getId();
    // logD(RFX_LOG_TAG, "onHandleRequest: %s", RFX_ID_TO_STR(requestId));
    switch (requestId) {
        case RFX_MSG_REQUEST_SET_DIGITS_LINE:
            logD(RFX_LOG_TAG, "onHandleRequest: RFX_MSG_REQUEST_SET_DIGITS_LINE");
            setDigitsLine(msg);
            break;
        case RFX_MSG_REQUEST_SET_DIGITS_REG_STATUS:
            logD(RFX_LOG_TAG, "onHandleRequest: RFX_MSG_REQUEST_SET_DIGITS_REG_STATUS");
            setDigitsRegStatus(msg);
            break;
        case RFX_MSG_REQUEST_SWITCH_RCS_ROI_STATUS:
            switchRcsRoiStatus(msg);
            break;
        case RFX_MSG_REQUEST_UPDATE_RCS_CAPABILITIES:
            updateRcsCapabilities(msg);
            break;
        case RFX_MSG_REQUEST_UPDATE_RCS_SESSION_INFO:
            updateRcsSessionInfo(msg);
            break;
        default:
            break;
    }
}

void RmcOpImsControlRequestHandler::setDigitsLine(const sp<RfxMclMessage>& msg) {
    /* AT+DIGITLINE=<account_id>,<serial_number>,<is_logout>,<has_next>,<is_native>,"<msisdn>","<token>"
     * <account_id>: 0: SIM1; 2: SIM2; 4:SIM3
     * <serial_number> a sequence number
     * <is_logout> 0: update cause by user add/del virtual lines; 1: update cause by user logout
     * <has_next>  0: no next line info; 1: has next line info
     * <is_native> 0: This is a virtual line; 1: This is a native line
     * <msisdn> msisdn
     * <token> sit
     */

    char** params = (char**)msg->getData()->getData();

    char* atCmd = AT_SET_DIGITS_LINE;
    int account = atoi(params[0]);
    int serial = atoi(params[1]);
    int isLogout = atoi(params[2]);
    int hasNext = atoi(params[3]);
    int isNative = atoi(params[4]);
    char* msisdn = params[5];

    // Google HIDL service changes "" in java as null in cpp
    char* sit = (params[6] == NULL) ? (char*)"" : params[6];

    /* send RFX_MSG_UNSOL_RCS_DIGITS_LINE_INFO to rcs volte stack */
    /* -- start -- */
    String8 urc = String8::format("%d,%d,%d,%d,%d,\"%s\",\"%s\"", account, serial, isLogout,
                                  hasNext, isNative, msisdn, sit);
    sp<RfxMclMessage> urc_response;
    urc_response = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_RCS_DIGITS_LINE_INFO, m_slot_id,
                                            RfxStringData((char*)urc.string()));
    responseToTelCore(urc_response);
    /* -- end -- */

    String8 cmd = String8::format("%s=%d,%d,%d,%d,%d,\"%s\",\"%s\"", atCmd, account, serial,
                                  isLogout, hasNext, isNative, msisdn, sit);

    handleCmdWithVoidResponse(msg, cmd);
}

void RmcOpImsControlRequestHandler::setDigitsRegStatus(const sp<RfxMclMessage>& msg) {
    char* digitsinfo = (char*)msg->getData()->getData();

    logD(RFX_LOG_TAG, "setDigitsRegStatus: %s", digitsinfo);

    // start to access token
    std::vector<char*> data;

    const int maxLen = 23;
    int i = 0;
    bool appendPhoneId = true;
    int rfxMsg = RFX_MSG_UNSOL_DIGITS_LINE_INDICATION;
    RfxAtLine* atLine = new RfxAtLine(digitsinfo, NULL);

    int err = 0;

    for (i; i < maxLen; i++) {
        char* str = atLine->atTokNextstr(&err);

        if (str == NULL) {
            logE(RFX_LOG_TAG, "There is something wrong with the digitsinfo");
        }
        // logD(RFX_LOG_TAG, "setDigitsRegStatus: token[%d]=%s", i, str);
        data.push_back(str);
    }

    RfxStringsData rfxData((char**)data.data(), (int)data.size());

    sp<RfxMclMessage> unsol = RfxMclMessage::obtainUrc(rfxMsg, m_slot_id, rfxData);
    responseToTelCore(unsol);

    delete atLine;
    atLine = NULL;
}

void RmcOpImsControlRequestHandler::switchRcsRoiStatus(const sp<RfxMclMessage>& msg) {
    int status = ((int*)msg->getData()->getData())[0];
    logD(RFX_LOG_TAG, "switchRcsRoiStatus:%d", status);
    int err = RIL_E_GENERIC_FAILURE;
    switch (status) {
        case 0:
        case 1: {
            sp<RfxAtResponse> p_response =
                    atSendCommand(String8::format("AT+EIMSRCS=%d", status + 3));
            err = p_response->getError();
            if (err < 0 || p_response->getSuccess() == 0) {
                logD(RFX_LOG_TAG, "switchRcsRoiStatus error:%d", err);
            }
            break;
        }

        default:
            logD(RFX_LOG_TAG, "switchRcsRoiStatus error, unsupport status");
            break;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(
            msg->getId(), err >= 0 ? RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    responseToTelCore(response);
}

void RmcOpImsControlRequestHandler::updateRcsCapabilities(const sp<RfxMclMessage>& msg) {
    char** data = (char**)msg->getData()->getData();
    char* mode = data[0];
    char* featureTags = data[1];
    int err = RIL_E_GENERIC_FAILURE;
    logD(RFX_LOG_TAG, "updateRcsCapabilities mode:%s, featureTags:%s", mode, featureTags);

    sp<RfxAtResponse> p_response =
            atSendCommand(String8::format("AT+EIMSRCS=%s,%s", mode, featureTags));
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        logD(RFX_LOG_TAG, "switchRcsRoiStatus error:%d", err);
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(
            msg->getId(), err >= 0 ? RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    responseToTelCore(response);
}

void RmcOpImsControlRequestHandler::updateRcsSessionInfo(const sp<RfxMclMessage>& msg) {
    int count = ((int*)msg->getData()->getData())[0];
    logD(RFX_LOG_TAG, "updateRcsSessionInfo count:%d", count);
    int err = RIL_E_GENERIC_FAILURE;

    switch (count) {
        case 0:
        case 1:
        case 254: {
            sp<RfxAtResponse> p_response =
                    atSendCommand(String8::format("AT+EIMSRCSCONN=%d", count));
            err = p_response->getError();
            if (err < 0 || p_response->getSuccess() == 0) {
                logD(RFX_LOG_TAG, "switchRcsRoiStatus error:%d", err);
            }
            break;
        }

        default:
            logD(RFX_LOG_TAG, "updateRcsSessionInfo error, unsupport count");
            break;
    }
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(
            msg->getId(), err >= 0 ? RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    responseToTelCore(response);
}
