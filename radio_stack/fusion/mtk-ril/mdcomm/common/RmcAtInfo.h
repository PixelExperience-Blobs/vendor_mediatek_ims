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

#ifndef __RMC_AT_INFO_H__
#define __RMC_AT_INFO_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxBaseHandler.h"

/*****************************************************************************
 * Class RmcAtSendInfo
 *****************************************************************************/
class RmcAtSendInfo {
    // External Method
  public:
    // Get the AT comman string sent to modem
    //
    // RETURNS: the AT command string
    const String8& getCmd() const { return m_cmd; }

    // Constructor / Destructor
  public:
    // Constructor with the at command string
    RmcAtSendInfo(String8 cmd  // [IN] The at command string sent to modem
                  )
        : m_cmd(cmd) {}

    // Destructor
    virtual ~RmcAtSendInfo() {}

    // Overriable
  public:
    virtual sp<RfxAtResponse> sendAt(
            RfxBaseHandler* handler  // [IN] the pointer of the RfxBaseHandler
            ) = 0;

    // Implementation
  private:
    String8 m_cmd;
};

/*****************************************************************************
 * Class RmcNoLineAtSendInfo
 *****************************************************************************/
class RmcNoLineAtSendInfo : public RmcAtSendInfo {
    // Constructor / Desctrucotr
  public:
    RmcNoLineAtSendInfo(String8 cmd) : RmcAtSendInfo(cmd) {}

    virtual ~RmcNoLineAtSendInfo() {}

    // Override
  public:
    virtual sp<RfxAtResponse> sendAt(RfxBaseHandler* handler) {
        return handler->atSendCommand(getCmd());
    }
};

/*****************************************************************************
 * Class RmcNumericAtSendInfo
 *****************************************************************************/
class RmcNumericAtSendInfo : public RmcAtSendInfo {
    // Constructor / Desctrucotr
  public:
    RmcNumericAtSendInfo(String8 cmd) : RmcAtSendInfo(cmd) {}

    virtual ~RmcNumericAtSendInfo() {}

    // Override
  public:
    virtual sp<RfxAtResponse> sendAt(RfxBaseHandler* handler) {
        return handler->atSendCommandNumeric(getCmd());
    }
};

/*****************************************************************************
 * Class RmcRawAtSendInfo
 *****************************************************************************/
class RmcRawAtSendInfo : public RmcAtSendInfo {
    // Constructor / Desctrucotr
  public:
    RmcRawAtSendInfo(String8 cmd) : RmcAtSendInfo(cmd) {}

    virtual ~RmcRawAtSendInfo() {}

    // Override
  public:
    virtual sp<RfxAtResponse> sendAt(RfxBaseHandler* handler) {
        return handler->atSendCommandRaw(getCmd());
    }
};

/*****************************************************************************
 * Class RmcSingleLineAtSendInfo
 *****************************************************************************/
class RmcSingleLineAtSendInfo : public RmcNoLineAtSendInfo {
    // External Method
  public:
    // get prefix of the at response
    //
    // RETURNS: the prefix of at response
    const String8& getResponsePrefix() const { return m_responsePrefix; }

    // Constructor / Desctructor
  public:
    RmcSingleLineAtSendInfo(String8 cmd, String8 responsePrefix)
        : RmcNoLineAtSendInfo(cmd), m_responsePrefix(responsePrefix) {}

    virtual ~RmcSingleLineAtSendInfo() {}

    // Override
  public:
    virtual sp<RfxAtResponse> sendAt(RfxBaseHandler* handler) {
        return handler->atSendCommandSingleline(getCmd(), getResponsePrefix().string());
    }

    // Implementation
  private:
    String8 m_responsePrefix;
};

/*****************************************************************************
 * Class RmcMultiLineAtSendInfo
 *****************************************************************************/
class RmcMultiLineAtSendInfo : public RmcSingleLineAtSendInfo {
    // Constructor / Desctructor
  public:
    RmcMultiLineAtSendInfo(String8 cmd, String8 responsePrefix)
        : RmcSingleLineAtSendInfo(cmd, responsePrefix) {}

    virtual ~RmcMultiLineAtSendInfo() {}

    // Override
  public:
    virtual sp<RfxAtResponse> sendAt(RfxBaseHandler* handler) {
        return handler->atSendCommandMultiline(getCmd(), getResponsePrefix().string());
    }
};

/*****************************************************************************
 * Class RmcAtRspInfo
 *****************************************************************************/
class RmcAtRspInfo {
    // External Method
  public:
    // Get the AT command
    //
    // RETURNS: the AT command
    const String8& getCmd() const { return m_cmd; }

    // Get the response of the AT
    //
    // RETURNS: the response of the AT
    const sp<RfxAtResponse>& getResponse() { return m_response; }

    // Constructor / Destructor
  public:
    RmcAtRspInfo(String8 cmd,                // [IN] the AT command
                 sp<RfxAtResponse> response  // [IN] the response of the AT command
                 )
        : m_cmd(cmd), m_response(response) {}

    virtual ~RmcAtRspInfo() {}

    // Implementation
  private:
    String8 m_cmd;
    sp<RfxAtResponse> m_response;
};

#endif /* __RMC_AT_INFO_H__ */
