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

#ifndef __RTC_DATA_CONTROLLER_H__
#define __RTC_DATA_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include "RfxAction.h"
#include "RfxSetDataProfileData.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RfxVoidData.h"
#include "RfxIntsData.h"
#include "RfxDataCallResponseData.h"
#include "RfxIaApnData.h"
#include "RfxLceStatusResponseData.h"
#include "RfxLceDataResponseData.h"
#include "RfxFdModeData.h"
#include "RfxLinkCapacityEstimateData.h"
#include "RfxLinkCapacityReportingCriteriaData.h"
#include <list>
#include <map>
#include "RfxKeepaliveRequestData.h"
#include "RfxKeepaliveStatusData.h"

/*****************************************************************************
 * Class RtcDataController
 *************************************************
 ****************************/

#define DATA_SETTING_NUMBERS 5
#define SKIP_DATA_SETTINGS -2
#define INVALID_VALUE -1

typedef enum {
    MOBILE_DATA = 0,
    ROAMING_DATA = 1,
    DEFAULT_DATA_SIM = 2,
    DOMESTIC_ROAMING_DATA = 3,
    INTERNATIONAL_ROAMING_DATA = 4
} DATA_SETTING_ITEM;

class RtcDataController : public RfxController {
    RFX_DECLARE_CLASS(RtcDataController);  // Required: declare this class

  public:
    RtcDataController();

    virtual ~RtcDataController();

    // Override
  protected:
    virtual void onInit();
    virtual void onDeinit();
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual bool onCheckIfRejectMessage(const sp<RfxMessage>& message, bool isModemPowerOff,
                                        int radioState);
    virtual bool onPreviewMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfResumeMessage(const sp<RfxMessage>& message);
    virtual void preCheckIfNeedDisableIms(const sp<RfxMessage>& message);
    virtual void handleSyncDataSettingsToMD(const sp<RfxMessage>& response);
    virtual void handleSetupDataRequest(const sp<RfxMessage>& message);
    virtual void handleSetupDataResponse(const sp<RfxMessage>& response);
    virtual void handleDeactivateDataRequest(const sp<RfxMessage>& message);
    virtual void handleSetPreferredDataModem(const sp<RfxMessage>& message);
    virtual void onImsConfirmed(const sp<RfxMessage> message);
    virtual void requestResumeIms(const sp<RfxMessage> message);

  private:
    // Private functions
    void registerForStatusChange();
    void onWorldModeStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);

    bool canHandleRequest(const sp<RfxMessage>& message);
    void onModemOffStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onUiccMccMncChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onSpnChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onImsiChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onGid1Changed(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onPnnChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onImpiChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onDataSettingStatusChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);

    void onAllowedChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    bool isPreferredDataMode();
    void enqueueForPreferredDataMode(const sp<RfxMessage>& message);
    void dequeueForPreferredDataMode(const sp<RfxMessage>& message);
    int getProfileID(int apnTypeId);

  private:
    bool isUnderCapabilitySwitch;
    int requestTokenIdForDisableIms;
    int transIdForDisableIms;
    int mIsPreferredDataMode;
    std::map<int, int> m_mapProfileIdToken;  // <ProfileId, Token>
    std::map<int, int> m_mapCidProfileId;    // <Cid, ProfileId>
};

#endif /* __RTC_DATA_CONTROLLER_H__ */
