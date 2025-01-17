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

#ifndef __RFX_PCO_IA_DATA_H__
#define __RFX_PCO_IA_DATA_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxBaseData.h"
#include <telephony/mtk_ril.h>

/*****************************************************************************
 * Class RfxPcoIaData
 *****************************************************************************/
class RfxPcoIaData : public RfxBaseData {
    RFX_DECLARE_DATA_CLASS(RfxPcoIaData);

  private:
    void copyString(char** dst, char* src);
};
#endif /* __RFX_PCO_IA_DATA_H__ */
