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

#ifndef __RFX_EMBMS_ENABLE_RESP_DATA__H__
#define __RFX_EMBMS_ENABLE_RESP_DATA__H__

#include "RfxBaseData.h"
#include <telephony/mtk_ril.h>

class RfxEmbmsEnableRespData : public RfxBaseData {
    RFX_DECLARE_DATA_CLASS(RfxEmbmsEnableRespData);

  public:
    RfxEmbmsEnableRespData(RIL_EMBMS_EnableResp* data, int length);

  private:
    void copyData(RIL_EMBMS_EnableResp* data, int length);
};
#endif /*__RFX_EMBMS_ENABLE_RESP_DATA__H__*/
