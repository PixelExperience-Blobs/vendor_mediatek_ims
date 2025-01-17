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

#ifndef __RFX_BASE_DATA__H__
#define __RFX_BASE_DATA__H__

//#include "RfxDataCloneManager.h"
#include <stdlib.h>
#include "RfxIdToStringUtils.h"
#include "RfxBasics.h"

// Predefined class
class RfxDataCloneManager;
class RfxBaseData;

typedef RfxBaseData* (*RfxCopyDataByDataFuncptr)(void* data, int length);
typedef RfxBaseData* (*RfxCopyDataByObjFuncptr)(const RfxBaseData* data);

#define RFX_DECLARE_DATA_CLASS(_class_name)                     \
  private:                                                      \
    _class_name(const _class_name& other);                      \
                                                                \
  public:                                                       \
    _class_name(void* data, int length);                        \
    virtual ~_class_name();                                     \
                                                                \
  public:                                                       \
    static RfxBaseData* copyDataByData(void* data, int length); \
    static RfxBaseData* copyDataByObj(const RfxBaseData* data)

#define RFX_IMPLEMENT_DATA_CLASS(_class_name)                                       \
    RfxBaseData* _class_name::copyDataByData(void* data, int length) {              \
        _class_name* ret = new _class_name(data, length);                           \
        return ret;                                                                 \
    }                                                                               \
                                                                                    \
    RfxBaseData* _class_name::copyDataByObj(const RfxBaseData* data) {              \
        _class_name* ret = new _class_name(data->getData(), data->getDataLength()); \
        return ret;                                                                 \
    }

#define RFX_REGISTER_DATA_TO_REQUEST_ID(dispatch_data_class_name, response_data_class_name, id)    \
    class dispatch_data_class_name##dispatch##id {                                                 \
      public:                                                                                      \
        dispatch_data_class_name##dispatch##id(int _id) {                                          \
            RfxIdToStringUtils::registerId(id, (char*)#id);                                        \
            RfxDataCloneManager::registerRequestId(&dispatch_data_class_name::copyDataByData,      \
                                                   &dispatch_data_class_name::copyDataByObj, _id); \
        }                                                                                          \
    };                                                                                             \
    class response_data_class_name##response##id {                                                 \
      public:                                                                                      \
        response_data_class_name##response##id(int _id) {                                          \
            RfxDataCloneManager::registerResponseId(&response_data_class_name::copyDataByData,     \
                                                    &response_data_class_name::copyDataByObj,      \
                                                    _id);                                          \
        }                                                                                          \
    };                                                                                             \
                                                                                                   \
    dispatch_data_class_name##dispatch##id s_init_val##dispatch_data_class_name##dispatch##id(id); \
    response_data_class_name##response##id s_init_val##response_data_class_name##response##id(id)

#define RFX_REGISTER_DATA_TO_URC_ID(data_class_name, id)                              \
    class data_class_name##id {                                                       \
      public:                                                                         \
        data_class_name##id(int _id) {                                                \
            RfxIdToStringUtils::registerId(id, (char*)#id);                           \
            RfxDataCloneManager::registerUrcId(&data_class_name::copyDataByData,      \
                                               &data_class_name::copyDataByObj, _id); \
        }                                                                             \
    };                                                                                \
                                                                                      \
    data_class_name##id s_init_val##data_class_name##id(id)

#define RFX_REGISTER_DATA_TO_EVENT_ID(data_class_name, id)                              \
    class data_class_name##id {                                                         \
      public:                                                                           \
        data_class_name##id(int _id) {                                                  \
            RfxIdToStringUtils::registerId(id, (char*)#id);                             \
            RfxDataCloneManager::registerEventId(&data_class_name::copyDataByData,      \
                                                 &data_class_name::copyDataByObj, _id); \
        }                                                                               \
    };                                                                                  \
                                                                                        \
    data_class_name##id s_init_val##data_class_name##id(id)

class RfxBaseData {
  public:
    RfxBaseData(void* data, int length) : m_data(NULL), m_length(0) {
        RFX_UNUSED(data);
        RFX_UNUSED(length);
    }

    virtual ~RfxBaseData() {}

  public:
    void* getData() const { return m_data; }

    int getDataLength() const { return m_length; }

  protected:
    void* m_data;
    int m_length;
};

#endif
