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

#ifndef __RFX_STRINGS_DATA__H__
#define __RFX_STRINGS_DATA__H__

#include <string.h>
#include "RfxBaseData.h"

class RfxStringsData : public RfxBaseData {
    RFX_DECLARE_DATA_CLASS(RfxStringsData);

  public:
    RfxStringsData();
    RfxStringsData(char** data, int countStrings);

  private:
    void copyStrings(char** data, int countStrings);
};
#endif
