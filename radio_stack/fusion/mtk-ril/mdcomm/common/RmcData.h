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

#ifndef __RMC_DATA_H__
#define __RMC_DATA_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxBaseData.h"
#include "RfxBaseHandler.h"
#include "RmcAtInfo.h"

/*****************************************************************************
 * Class RmcBaseReqData
 *****************************************************************************/
class RmcBaseReqData : public RfxBaseData {
    // External Method
  public:
    // Set the RIL error number
    //
    // RETURNS: void
    void setError(RIL_Errno err  // [IN] Set RIL error number
    ) {
        m_error = err;
    }

    // Get the RIL error number
    //
    // RETURNS: the RIL error number
    RIL_Errno getError() const { return m_error; }

    // Constructor / Destructor
  public:
    RmcBaseReqData(void* data, int length) : RfxBaseData(data, length), m_error(RIL_E_SUCCESS) {}

    virtual ~RmcBaseReqData() {}

    // Overridable
  public:
    // Get the first AT send Info
    //
    // RETURNS: the first AT send info
    virtual RmcAtSendInfo* getFirstAt(RfxBaseHandler* h  // [IN] The pointer of the RfxBaseHandler
                                      ) = 0;

    // Get the next AT send info
    //
    // RETURNS: the next AT send info
    virtual RmcAtSendInfo* getNextAt(RmcAtRspInfo* cur,  // [IN] the current AT response info
                                     RfxBaseHandler* h   // [IN] the pointer of the RfxBaseHandler
                                     ) = 0;

    // Implementation
  private:
    RIL_Errno m_error;
};

/*****************************************************************************
 * Class RmcMultiAtReq
 *****************************************************************************/
class RmcMultiAtReq : public RmcBaseReqData {
    // Constructor / Destructor
  public:
    RmcMultiAtReq(void* data, int length) : RmcBaseReqData(data, length) {}
    virtual ~RmcMultiAtReq() {}

    // Override
  public:
    virtual RmcAtSendInfo* getFirstAt(RfxBaseHandler* h) { return onGetFirstAtInfo(h); }

    virtual RmcAtSendInfo* getNextAt(RmcAtRspInfo* cur, RfxBaseHandler* h);

    // Overridable
  protected:
    // Callback when get the first AT send info
    //
    // RETURNS: the AT send info
    virtual RmcAtSendInfo* onGetFirstAtInfo(
            RfxBaseHandler* h  // [IN] The pointer of the RfxBaseHandler
            ) = 0;

    // Callback when get the next AT send info
    //
    // RETURNS: the next AT send info
    virtual RmcAtSendInfo* onGetNextAtInfo(
            const String8& curCmd,  // [IN] The current AT command
            RfxBaseHandler* h       // [IN] The pointer of the RfxBaseHandler
    ) {
        RFX_UNUSED(curCmd);
        RFX_UNUSED(h);
        return NULL;
    };

    // Callback when handle the final response
    //
    // RETURNS: void
    virtual void onHandleFinalResponse(
            const String8& curCmd,  // [IN] the AT command
            RfxAtLine* line,        // [IN] the AT line after atToStart is invoked
            RfxBaseHandler* h       // [IN] the pointer of the RfxBaseHandler
    ) {
        RFX_UNUSED(curCmd);
        RFX_UNUSED(line);
        RFX_UNUSED(h);
    }

    // Callback when handle the final response in error case
    //
    // RETURNS: void
    virtual void onHandleFinalResponseForError(
            const String8& curCmd,  // [IN] the AT command
            RfxAtLine* line,        // [IN] the AT line after atToStart is invoked
            RfxBaseHandler* h       // [IN] the pointer of the RfxBaseHandler
    ) {
        RFX_UNUSED(curCmd);
        RFX_UNUSED(line);
        RFX_UNUSED(h);
    }

    // Callback when handle the intermediate
    //
    // RETURNS: return true to handle the next intermediates if there is
    virtual bool onHandleIntermediates(
            const String8& curCmd,  // [IN] the AT command
            RfxAtLine* line,        // [IN] the AT line after atToStart is invoked
            RfxBaseHandler* h       // [IN] the pointer of the RfxBaseHandler
    ) {
        RFX_UNUSED(curCmd);
        RFX_UNUSED(line);
        RFX_UNUSED(h);
        return true;
    }
};

/*****************************************************************************
 * Class RmcSingleAtReq
 *****************************************************************************/
class RmcSingleAtReq : public RmcMultiAtReq {
    // Constructor / Destructor
  public:
    RmcSingleAtReq(void* data, int length) : RmcMultiAtReq(data, length) {}

    virtual ~RmcSingleAtReq() {}

    // Override
  protected:
    virtual RmcAtSendInfo* onGetFirstAtInfo(RfxBaseHandler* h);
    virtual RmcAtSendInfo* onGetNextAtInfo(const String8& cmd, RfxBaseHandler* h);
    virtual void onHandleFinalResponse(const String8& cmd, RfxAtLine* line, RfxBaseHandler* h);
    virtual void onHandleFinalResponseForError(const String8& cmd, RfxAtLine* line,
                                               RfxBaseHandler* h);
    virtual bool onHandleIntermediates(const String8& cmd, RfxAtLine* line, RfxBaseHandler* h);

    // Overridable
  protected:
    // Get the AT send info
    //
    // RETURNS: the AT send info
    virtual RmcAtSendInfo* onGetAtInfo(RfxBaseHandler* h  // [IN] the pointer of the RfxBaseHandler
                                       ) = 0;

    // Callback when handle the final response
    //
    // RETURNS: void
    virtual void onHandleFinalResponse(
            RfxAtLine* line,   // [IN] the AT line after atToStart is invoked
            RfxBaseHandler* h  // [IN] the pointer of the RfxBaseHandler
    ) {
        RFX_UNUSED(line);
        RFX_UNUSED(h);
    }

    // Callback when handle the final response in error case
    //
    // RETURNS: void
    virtual void onHandleFinalResponseForError(
            RfxAtLine* line,   // [IN] the AT line after atToStart is invoked
            RfxBaseHandler* h  // [IN] the pointer of the RfxBaseHandler
    ) {
        RFX_UNUSED(line);
        RFX_UNUSED(h);
    }

    // Callback when handle the intermediate
    //
    // RETURNS: return true to handle the next intermediates if there is
    virtual bool onHandleIntermediates(
            RfxAtLine* line,   // [IN] the AT line after atToStart is invoked
            RfxBaseHandler* h  // [IN] the pointer of the RfxBaseHandler
    ) {
        RFX_UNUSED(line);
        RFX_UNUSED(h);
        return true;
    }
};

/*****************************************************************************
 * Class RmcBaseRspData
 *****************************************************************************/
class RmcBaseRspData : public RfxBaseData {
    // External Method
  public:
    // Generate the response message to telcore
    //
    // RETURNS: the response message
    sp<RfxMclMessage> toMessage(
            const sp<RfxMclMessage>& msg  // [IN] the request message from telcore
    );

    // Constructor / Destructor
  public:
    RmcBaseRspData(void* data, int length) : RfxBaseData(data, length) {}
    virtual ~RmcBaseRspData() {}

    // Overriable
  protected:
    // Get the RIL error number
    //
    // RETURNS: the RIL error number
    virtual RIL_Errno getError() const = 0;

  protected:
    // Get the reference of the data
    //
    // RETURNS: the reference of the data
    const RfxBaseData& getData() const { return *this; }
};

/*****************************************************************************
 * Class RmcVoidRsp
 *****************************************************************************/
class RmcVoidRsp : public RmcBaseRspData {
    RFX_DECLARE_DATA_CLASS(RmcVoidRsp);

    // Constructor / Destructor
  public:
    RmcVoidRsp(RIL_Errno e) : RmcBaseRspData(NULL, 0), m_error(e) {}

    // Override
  protected:
    virtual RIL_Errno getError() const { return m_error; }

    // Implementation
  private:
    RIL_Errno m_error;
};

/*****************************************************************************
 * Class RmcBaseUrspData
 *****************************************************************************/
class RmcBaseUrspData : public RfxBaseData {
    // External Method
  public:
    // Generate the URC message to telcore
    //
    // RETURNS: the URC message
    sp<RfxMclMessage> toMessage();

    // Constructor / Destructor
  public:
    RmcBaseUrspData(void* data, int length) : RfxBaseData(data, length) {}
    virtual ~RmcBaseUrspData() {}

    // Overridable
  protected:
    // Get the URC ID
    //
    // RETURNS: the URC ID
    virtual int getUrcId() const = 0;

    // Get the Slot ID
    //
    // RETURNS The slot ID
    virtual int getSlotId() const = 0;

  protected:
    // Get the reference of the data
    //
    // The reference of the data
    const RfxBaseData& getData() const { return *this; }
};

/*****************************************************************************
 * Class RmcVoidUrsp
 *****************************************************************************/
class RmcVoidUrsp : public RmcBaseUrspData {
    RFX_DECLARE_DATA_CLASS(RmcVoidUrsp);

    // Constructor / Destructor
  public:
    RmcVoidUrsp(int urc, int slotId) : RmcBaseUrspData(NULL, 0), m_urc(urc), m_slotId(slotId) {}

    // Override
  protected:
    virtual int getUrcId() const { return m_urc; }

    virtual int getSlotId() const { return m_slotId; }

    // Implementation
  private:
    int m_urc;
    int m_slotId;
};

/*****************************************************************************
 * Class RmcStringUrsp
 *****************************************************************************/
class RmcStringUrsp : public RmcVoidUrsp {
    RFX_DECLARE_DATA_CLASS(RmcStringUrsp);

    // Constructor / Destructor
  public:
    RmcStringUrsp(int urc, int slotId, String8 str) : RmcVoidUrsp(urc, slotId), m_string(str) {
        m_data = (void*)m_string.string();
        m_length = m_string.length();
    }

    // Implementation
  private:
    String8 m_string;
};

#endif /* __RMC_DATA_H__ */
