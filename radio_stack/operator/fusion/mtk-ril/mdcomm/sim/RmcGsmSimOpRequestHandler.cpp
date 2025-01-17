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
#include "RmcSimBaseHandler.h"
#include "RmcCommSimDefs.h"
#include "RmcGsmSimRequestHandler.h"
#include "RmcGsmSimOpRequestHandler.h"
#include "RfxStringData.h"
#include "RfxVoidData.h"
#include <telephony/mtk_ril.h>
#include "RfxMessageId.h"
#include <time.h>
#include <sys/time.h>
#include "RfxIntsData.h"

static const int ch1OpReqList[] = {
        RFX_MSG_REQUEST_GET_SHARED_KEY,    RFX_MSG_REQUEST_UPDATE_SIM_LOCK_SETTINGS,
        RFX_MSG_REQUEST_GET_SIM_LOCK_INFO, RFX_MSG_REQUEST_RESET_SIM_LOCK_SETTINGS,
        RFX_MSG_REQUEST_GET_MODEM_STATUS,
};

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringData, RFX_MSG_REQUEST_GET_SHARED_KEY);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxVoidData,
                                RFX_MSG_REQUEST_UPDATE_SIM_LOCK_SETTINGS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringData, RFX_MSG_REQUEST_GET_SIM_LOCK_INFO);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData, RFX_MSG_REQUEST_RESET_SIM_LOCK_SETTINGS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringData, RFX_MSG_REQUEST_GET_MODEM_STATUS);

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

RmcGsmSimOpRequestHandler::RmcGsmSimOpRequestHandler(int slot_id, int channel_id)
    : RmcGsmSimRequestHandler(slot_id, channel_id) {
    setTag(String8("RmcGsmSimOpRequestHandler"));
}

RmcGsmSimOpRequestHandler::~RmcGsmSimOpRequestHandler() {}

const int* RmcGsmSimOpRequestHandler::queryTable(int channel_id, int* record_num) {
    const int* superTable = RmcGsmSimRequestHandler::queryTable(channel_id, record_num);
    int subRecordNumber = 0;
    int* bufTable = NULL;
    int index = 0;
    int supRecordNumber = *record_num;

    if (channel_id == RIL_CMD_PROXY_1) {
        subRecordNumber = sizeof(ch1OpReqList) / sizeof(int);
        bufTable = (int*)calloc(1, sizeof(int) * (subRecordNumber + supRecordNumber));
        RFX_ASSERT(bufTable != NULL);
        if (NULL != superTable) {
            memcpy(bufTable, superTable, sizeof(int) * supRecordNumber);
            index += supRecordNumber;
        } else {
            logE(mTag, "Null superTable", channel_id);
        }
        memcpy(bufTable + index, ch1OpReqList, sizeof(int) * subRecordNumber);
        *record_num += subRecordNumber;
    } else {
        bufTable = (int*)calloc(1, sizeof(int) * supRecordNumber);
        if (NULL != superTable) {
            memcpy(bufTable, superTable, sizeof(int) * supRecordNumber);
        } else {
            logE(mTag, "Null superTable", channel_id);
        }
    }
    return bufTable;
}

RmcSimBaseHandler::SIM_HANDLE_RESULT RmcGsmSimOpRequestHandler::needHandle(
        const sp<RfxMclMessage>& msg) {
    int request = msg->getId();
    RmcSimBaseHandler::SIM_HANDLE_RESULT result = RmcSimBaseHandler::RESULT_IGNORE;

    switch (request) {
        case RFX_MSG_REQUEST_GET_SHARED_KEY:
        case RFX_MSG_REQUEST_UPDATE_SIM_LOCK_SETTINGS:
        case RFX_MSG_REQUEST_GET_SIM_LOCK_INFO:
        case RFX_MSG_REQUEST_RESET_SIM_LOCK_SETTINGS:
        case RFX_MSG_REQUEST_GET_MODEM_STATUS:
            result = RmcSimBaseHandler::RESULT_NEED;
            break;
        default:
            result = RmcGsmSimRequestHandler::needHandle(msg);
            break;
    }

    return result;
}

void RmcGsmSimOpRequestHandler::handleRequest(const sp<RfxMclMessage>& msg) {
    int request = msg->getId();
    switch (request) {
        case RFX_MSG_REQUEST_GET_SHARED_KEY:
            handleGetSharedKey(msg);
            break;
        case RFX_MSG_REQUEST_UPDATE_SIM_LOCK_SETTINGS:
            handleUpdateSimLockSettings(msg);
            break;
        case RFX_MSG_REQUEST_GET_SIM_LOCK_INFO:
            handleGetSimLockInfo(msg);
            break;
        case RFX_MSG_REQUEST_RESET_SIM_LOCK_SETTINGS:
            handleResetSimLockSettings(msg);
            break;
        case RFX_MSG_REQUEST_GET_MODEM_STATUS:
            handleGetModemStatus(msg);
            break;
        default:
            RmcGsmSimRequestHandler::handleRequest(msg);
            break;
    }
}

void RmcGsmSimOpRequestHandler::handleGetSharedKey(const sp<RfxMclMessage>& msg) {
    String8 cmd("");
    RfxAtLine* line = NULL;
    int err = -1;
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxMclMessage> response;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int key_len;
    char* key = NULL;

    logD(mTag, "[SIM ME Lock]handleGetSharedKey.\n");

    cmd.append(String8::format("AT+ETMOSK=0"));
    p_response = atSendCommandSingleline(cmd, "+ETMOSK:");
    cmd.clear();
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        goto done;
    }
    line = p_response->getIntermediates();
    line->atTokStart(&err);
    if (err < 0) {
        goto done;
    }

    // Get shared key len
    key_len = line->atTokNextint(&err);
    if (err < 0) {
        goto done;
    }

    // Get shared key
    key = line->atTokNextstr(&err);
    if (err < 0) {
        goto done;
    }
    ret = RIL_E_SUCCESS;
done:
    logD(mTag, "[SIM ME Lock]done.\n");
    if (key != NULL) {
        logD(mTag, "[SIM ME Lock]key %s.\n", key);
        response = RfxMclMessage::obtainResponse(
                msg->getId(), ret, RfxStringData((void*)key, strlen(key)), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxStringData(), msg, false);
    }

    responseToTelCore(response);
}

void RmcGsmSimOpRequestHandler::handleUpdateSimLockSettings(const sp<RfxMclMessage>& msg) {
    //    RfxBaseData *rbd = msg->getData();
    //    const char** strings = (const char**)rbd->getData();
    String8 pString((char*)(msg->getData()->getData()));
    String8 cmd("");
    RfxAtLine* line = NULL;
    int err = -1;
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxMclMessage> response;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    logD(mTag, "[SIM ME Lock]handleUpdateSimLockSettings strings %s\n", pString.string());

    cmd.append(String8::format("AT+ETMOSLB=1,%d,\"%s\"", (unsigned int)((pString.size() + 1) / 2),
                               pString.string()));

    p_response = atSendCommand(cmd.string());
    cmd.clear();
    err = p_response->getError();
    if (err < 0) {
        goto done;
    }

    if (p_response->getSuccess() == 0) {
        switch (p_response->atGetCmeError()) {
            logD(mTag, "p_response %d\n", p_response->atGetCmeError());
            case CME_VERIFICATION_FAILED:
                ret = RIL_E_VERIFICATION_FAILED;
                logD(mTag, "p_response CME_SIM_LOCK_BLOB_VERIFICATION_FAILED\n");
                break;
            case CME_REBOOT_REQUEST:
                ret = RIL_E_REBOOT_REQUEST;
                logD(mTag, "p_response CME_REBOOT_REQUEST\n");
                break;
            default:
                goto done;
        }
    } else {
        ret = RIL_E_SUCCESS;
        sp<RfxMclMessage> unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_SIM_STATUS_CHANGED,
                                                           m_slot_id, RfxVoidData());
        responseToTelCore(unsol);
    }

done:
    response = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxVoidData(), msg, false);
    responseToTelCore(response);
}

void RmcGsmSimOpRequestHandler::handleGetSimLockInfo(const sp<RfxMclMessage>& msg) {
    String8 cmd("");
    RfxAtLine* line = NULL;
    int err = -1;
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxMclMessage> response;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int info_len;
    char* info = NULL;

    logD(mTag, "[SIM ME Lock]handleGetSimLockInfo\n");

    cmd.append(String8::format("AT+ETMOSLB=3"));
    p_response = atSendCommandSingleline(cmd, "+ETMOSLB:");
    cmd.clear();
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        logD(mTag, "handleGetSimLockInfo fail.\n");
        goto done;
    }
    line = p_response->getIntermediates();
    line->atTokStart(&err);
    if (err < 0) {
        goto done;
    }

    // Get info len
    info_len = line->atTokNextint(&err);
    if (err < 0) {
        goto done;
    }

    // Get info
    info = line->atTokNextstr(&err);
    if (err < 0) {
        goto done;
    }
    ret = RIL_E_SUCCESS;
done:
    logD(mTag, "[SIM ME Lock]done.\n");
    if (info != NULL) {
        logD(mTag, "[SIM ME Lock]info %s.\n", info);
        response = RfxMclMessage::obtainResponse(
                msg->getId(), ret, RfxStringData((void*)info, strlen(info)), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxStringData(), msg, false);
    }
    responseToTelCore(response);
}

void RmcGsmSimOpRequestHandler::handleResetSimLockSettings(const sp<RfxMclMessage>& msg) {
    String8 cmd("");
    RfxAtLine* line = NULL;
    int err = -1;
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxMclMessage> response;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    logD(mTag, "[SIM ME Lock]handleResetSimLockSettings\n");

    cmd.append(String8::format("AT+ETMOSLB=2"));

    p_response = atSendCommand(cmd.string());
    cmd.clear();
    err = p_response->getError();
    if (err < 0) {
        goto done;
    }

    if (p_response->getSuccess() == 0) {
        switch (p_response->atGetCmeError()) {
            logD(mTag, "p_response %d\n", p_response->atGetCmeError());
            case CME_VERIFICATION_FAILED:
                ret = RIL_E_VERIFICATION_FAILED;
                logD(mTag, "p_response CME_SIM_LOCK_BLOB_VERIFICATION_FAILED\n");
                break;
            case CME_REBOOT_REQUEST:
                ret = RIL_E_REBOOT_REQUEST;
                logD(mTag, "p_response CME_REBOOT_REQUEST\n");
                break;
            default:
                goto done;
        }
    } else {
        sp<RfxMclMessage> unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_SIM_STATUS_CHANGED,
                                                           m_slot_id, RfxVoidData());
        responseToTelCore(unsol);
    }
    ret = RIL_E_SUCCESS;
done:
    response = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxVoidData(), msg, false);
    responseToTelCore(response);
}

void RmcGsmSimOpRequestHandler::handleGetModemStatus(const sp<RfxMclMessage>& msg) {
    String8 cmd("");
    RfxAtLine* line = NULL;
    int err = -1;
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxMclMessage> response;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int status_len;
    char* status = NULL;

    logD(mTag, "[SIM ME Lock]handleGetModemStatus\n");

    cmd.append(String8::format("AT+ETMOSLB=4"));
    p_response = atSendCommandSingleline(cmd, "+ETMOSLB:");
    cmd.clear();
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        logD(mTag, "handleGetModemStatus fail.\n");
        goto done;
    }
    line = p_response->getIntermediates();
    line->atTokStart(&err);
    if (err < 0) {
        goto done;
    }

    // Get status len
    status_len = line->atTokNextint(&err);
    if (err < 0) {
        goto done;
    }

    // Get status
    status = line->atTokNextstr(&err);
    if (err < 0) {
        goto done;
    }
    ret = RIL_E_SUCCESS;
done:
    logD(mTag, "[SIM ME Lock]done.\n");
    if (status != NULL) {
        logD(mTag, "[SIM ME Lock]status %s.\n", status);
        response = RfxMclMessage::obtainResponse(
                msg->getId(), ret, RfxStringData((void*)status, strlen(status)), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxStringData(), msg, false);
    }
    responseToTelCore(response);
}
