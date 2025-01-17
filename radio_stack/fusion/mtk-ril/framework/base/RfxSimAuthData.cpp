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

#include "RfxSimAuthData.h"
#include <telephony/mtk_ril.h>

RFX_IMPLEMENT_DATA_CLASS(RfxSimAuthData);

RfxSimAuthData::RfxSimAuthData(void* _data, int _length) : RfxBaseData(_data, _length) {
    if (_data != NULL) {
        RIL_SimAuthentication* pSimAuth = (RIL_SimAuthentication*)_data;
        RIL_SimAuthentication* pData =
                (RIL_SimAuthentication*)calloc(1, sizeof(RIL_SimAuthentication));
        RFX_ASSERT(pData != NULL);

        pData->authContext = pSimAuth->authContext;

        if (pSimAuth->aid != NULL) {
            asprintf(&pData->aid, "%s", pSimAuth->aid);
        }
        if (pSimAuth->authData != NULL) {
            asprintf(&pData->authData, "%s", pSimAuth->authData);
        }

        m_data = (void*)pData;
        m_length = _length;
    }
}

RfxSimAuthData::~RfxSimAuthData() {
    // free memory
    if (m_data != NULL) {
        RIL_SimAuthentication* pData = (RIL_SimAuthentication*)m_data;
        if (pData->aid != NULL) {
            free(pData->aid);
            pData->aid = NULL;
        }
        if (pData->authData != NULL) {
            free(pData->authData);
            pData->authData = NULL;
        }
        free(m_data);
        m_data = NULL;
    }
}
