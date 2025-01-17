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

#include "RfxCdmaInfoRecData.h"

RFX_IMPLEMENT_DATA_CLASS(RfxCdmaInfoRecData);

RfxCdmaInfoRecData::RfxCdmaInfoRecData(void* data, int length) : RfxBaseData(data, length) {
    if (data == NULL) {
        m_data = NULL;
        return;
    }

    m_length = length;
    m_data = (RIL_CDMA_InformationRecords*)calloc(1, sizeof(RIL_CDMA_InformationRecords));
    RFX_ASSERT(m_data != NULL);
    memcpy(m_data, data, sizeof(RIL_CDMA_InformationRecords));
}

RfxCdmaInfoRecData::~RfxCdmaInfoRecData() {
    // free
    if (m_data) {
        free(m_data);
    }
    m_data = NULL;
}
