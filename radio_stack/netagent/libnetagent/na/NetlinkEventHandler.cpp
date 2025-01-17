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
#include "NetlinkEventHandler.h"

#define NA_LOG_TAG "NetLnkEventHdlr"

/*****************************************************************************
 * Class NetlinkEventHandler
 *****************************************************************************/
NetlinkEventHandler::NetlinkEventHandler(NetAgentService* na, int listenerSocket, int format)
    : NetlinkListener(listenerSocket, format), m_pLastNetlinkEventReqInfo(NULL) {
    m_pNetAgent = na;

    if (m_pLastNetlinkEventReqInfo == NULL) {
        m_pLastNetlinkEventReqInfo = new Vector<NetlinkEventReqInfo*>();
    }
}

NetlinkEventHandler::~NetlinkEventHandler() {
    if (m_pLastNetlinkEventReqInfo != NULL) {
        int size = m_pLastNetlinkEventReqInfo->size();
        for (int i = size - 1; i >= 0; i--) {
            NetlinkEventReqInfo* pTemp = m_pLastNetlinkEventReqInfo->itemAt(i);
            freeNetlinkEventObj(pTemp);
            m_pLastNetlinkEventReqInfo->removeAt(i);
        }
        delete m_pLastNetlinkEventReqInfo;
        m_pLastNetlinkEventReqInfo = NULL;
    }
}

int NetlinkEventHandler::start() { return startListener(); }

int NetlinkEventHandler::stop() { return stopListener(); }

void NetlinkEventHandler::onEvent(NetlinkEvent* evt) {
    int err = 0;
    const char* subsys = evt->getSubsystem();
    if (subsys == NULL) {
        NA_LOG_E("[%s] no subsystem found in netlink event", __FUNCTION__);
        return;
    }

    if (strcmp(subsys, "net") == 0) {
        NetlinkEvent::Action action = evt->getAction();
        char* interfaceName = const_cast<char*>(evt->findParam("INTERFACE"));
        const char* ccmniName = NetAgentService::getCcmniInterfaceName();
        int interfaceId = INVALID_INTERFACE_ID;

        NA_LOG_D("[%s] get netlink event, interfaceName: %s, action: %d", __FUNCTION__,
                 interfaceName, action);

        if (interfaceName != NULL && ccmniName != NULL) {
            if (strncmp(ccmniName, interfaceName, strlen(ccmniName)) == 0) {
                interfaceName += strlen(ccmniName);
                interfaceId = stringToBinaryBase(interfaceName, 10, &err);
                if (err < 0) {
                    NA_LOG_E(
                            "[%s] error occurs when converting interfaceId to binary"
                            ", err = %d",
                            __FUNCTION__, err);
                    return;
                }
            } else {
                NA_LOG_D("[%s] ignore this netlink event caused mismatched interface name",
                         __FUNCTION__);
                return;
            }
        }

        if (action == NetlinkEvent::Action::kAddressUpdated ||
            action == NetlinkEvent::Action::kAddressRemoved) {
            ACTION act = (action == NetlinkEvent::Action::kAddressUpdated) ? ACTION_ADDR_UPDATED
                                                                           : ACTION_ADDR_REMOVED;
            char* tmpAddress = const_cast<char*>(evt->findParam("ADDRESS"));
            NA_LOG_D("[%s] %s %s", __FUNCTION__, actionToString(act), tmpAddress);
            char* address = NULL;
            address = strsep(&tmpAddress, "/");
            int ipv6PrefixLength = stringToBinaryBase(tmpAddress, 10, &err);
            if (err < 0) {
                ipv6PrefixLength = INVALID_IPV6_PREFIX_LENGTH;
                NA_LOG_E(
                        "[%s] error occurs when converting ipv6PrefixLength to binary"
                        ", err = %d",
                        __FUNCTION__, err);
            }

            enqueueReqInfo(NETAGENT_IO_CMD_IPUPDATE, address, ipv6PrefixLength, interfaceId, act);
        }
    }
}

void NetlinkEventHandler::enqueueReqInfo(NA_CMD cmd, char* address, int ipv6PrefixLength,
                                         int interfaceId, ACTION action) {
    if (m_pNetAgent == NULL) {
        NA_LOG_E("[%s] m_pNetAgent is NULL", __FUNCTION__);
        return;
    }

    NetlinkEventReqInfo* pNetlinkEventObj =
            (NetlinkEventReqInfo*)calloc(1, sizeof(NetlinkEventReqInfo));
    if (pNetlinkEventObj == NULL) {
        NA_LOG_E("[%s] can't allocate netlink event obj", __FUNCTION__);
        return;
    }

    pNetlinkEventObj->cmd = cmd;
    pNetlinkEventObj->interfaceId = interfaceId;
    pNetlinkEventObj->ipv6PrefixLength = ipv6PrefixLength;
    pNetlinkEventObj->action = action;
    if (address != NULL) {
        strncpy(pNetlinkEventObj->address, address, INET6_ADDRSTRLEN - 1);
    }

    m_pNetAgent->enqueueReqInfo(pNetlinkEventObj, REQUEST_TYPE_NETLINK);
}

int NetlinkEventHandler::getCommandType(void* obj, NA_CMD* cmd) {
    NetlinkEventReqInfo* pReqInfo = reinterpret_cast<NetlinkEventReqInfo*>(obj);

    if (pReqInfo == NULL) {
        NA_LOG_E("[%s] pReqInfo is NULL", __FUNCTION__);
        return NETLINK_RET_GENERIC_FAILURE;
    }

    *cmd = pReqInfo->cmd;
    return NETLINK_RET_SUCCESS;
}

int NetlinkEventHandler::getAddress(void* obj, char* address) {
    NetlinkEventReqInfo* pReqInfo = reinterpret_cast<NetlinkEventReqInfo*>(obj);

    if (pReqInfo == NULL) {
        NA_LOG_E("[%s] pReqInfo is NULL", __FUNCTION__);
        return NETLINK_RET_GENERIC_FAILURE;
    }

    strncpy(address, pReqInfo->address, INET6_ADDRSTRLEN);
    return NETLINK_RET_SUCCESS;
}

int NetlinkEventHandler::getInterfaceId(void* obj, unsigned int* interfaceId) {
    NetlinkEventReqInfo* pReqInfo = reinterpret_cast<NetlinkEventReqInfo*>(obj);

    if (pReqInfo == NULL) {
        NA_LOG_E("[%s] pReqInfo is NULL", __FUNCTION__);
        return NETLINK_RET_GENERIC_FAILURE;
    }

    *interfaceId = pReqInfo->interfaceId;
    return NETLINK_RET_SUCCESS;
}

int NetlinkEventHandler::getIpv6PrefixLength(void* obj, int* ipv6PrefixLength) {
    NetlinkEventReqInfo* pReqInfo = reinterpret_cast<NetlinkEventReqInfo*>(obj);

    if (pReqInfo == NULL) {
        NA_LOG_E("[%s] pReqInfo is NULL", __FUNCTION__);
        return NETLINK_RET_GENERIC_FAILURE;
    }

    *ipv6PrefixLength = pReqInfo->ipv6PrefixLength;
    return NETLINK_RET_SUCCESS;
}

int NetlinkEventHandler::getAction(void* obj, ACTION* action) {
    NetlinkEventReqInfo* pReqInfo = reinterpret_cast<NetlinkEventReqInfo*>(obj);

    if (pReqInfo == NULL) {
        NA_LOG_E("[%s] pReqInfo is NULL", __FUNCTION__);
        return NETLINK_RET_GENERIC_FAILURE;
    }

    *action = pReqInfo->action;
    return NETLINK_RET_SUCCESS;
}

void NetlinkEventHandler::removeLastReqInfo(int interfaceId) {
    if (m_pLastNetlinkEventReqInfo == NULL) {
        NA_LOG_E("[%s] m_pLastNetlinkEventReqInfo is NULL", __FUNCTION__);
        return;
    }

    int size = m_pLastNetlinkEventReqInfo->size();
    for (int i = 0; i < size; i++) {
        NetlinkEventReqInfo* pTemp = m_pLastNetlinkEventReqInfo->itemAt(i);
        if (pTemp->interfaceId == interfaceId) {
            freeNetlinkEventObj(pTemp);
            m_pLastNetlinkEventReqInfo->removeAt(i);
            break;
        }
    }
}

int NetlinkEventHandler::setLastReqInfo(void* obj) {
    NetlinkEventReqInfo* pReqInfo = reinterpret_cast<NetlinkEventReqInfo*>(obj);

    if (pReqInfo == NULL) {
        NA_LOG_E("[%s] pReqInfo is NULL", __FUNCTION__);
        return NETLINK_RET_GENERIC_FAILURE;
    }

    if (pReqInfo->interfaceId == INVALID_INTERFACE_ID) {
        NA_LOG_E("[%s] new pReqInfo interfaceId is invalid", __FUNCTION__);
        return NETLINK_RET_GENERIC_FAILURE;
    }

    if (m_pLastNetlinkEventReqInfo == NULL) {
        NA_LOG_E("[%s] m_pLastNetlinkEventReqInfo is NULL", __FUNCTION__);
        return NETLINK_RET_GENERIC_FAILURE;
    }

    int size = m_pLastNetlinkEventReqInfo->size();
    for (int i = 0; i < size; i++) {
        NetlinkEventReqInfo* pTemp = m_pLastNetlinkEventReqInfo->itemAt(i);
        if (pTemp->interfaceId == pReqInfo->interfaceId) {
            freeNetlinkEventObj(pTemp);
            m_pLastNetlinkEventReqInfo->removeAt(i);
            break;
        }
    }

    NetlinkEventReqInfo* pNetlinkEventObj =
            (NetlinkEventReqInfo*)calloc(1, sizeof(NetlinkEventReqInfo));
    if (pNetlinkEventObj == NULL) {
        NA_LOG_E("[%s] can't allocate netlink event obj", __FUNCTION__);
        return NETLINK_RET_GENERIC_FAILURE;
    }

    pNetlinkEventObj->interfaceId = pReqInfo->interfaceId;
    pNetlinkEventObj->ipv6PrefixLength = pReqInfo->ipv6PrefixLength;
    pNetlinkEventObj->cmd = pReqInfo->cmd;
    pNetlinkEventObj->action = pReqInfo->action;
    strncpy(pNetlinkEventObj->address, pReqInfo->address, INET6_ADDRSTRLEN);

    NA_LOG_D("[%s] %s", __FUNCTION__, reqInfoToString(pNetlinkEventObj).string());

    m_pLastNetlinkEventReqInfo->add(pNetlinkEventObj);
    return NETLINK_RET_SUCCESS;
}

int NetlinkEventHandler::hasLastReqInfoChanged(void* obj) {
    NetlinkEventReqInfo* pReqInfo = reinterpret_cast<NetlinkEventReqInfo*>(obj);
    NETLINK_RET_STATUS ret = NETLINK_RET_REQ_INFO_NEW;

    if (pReqInfo == NULL) {
        NA_LOG_E("[%s] pReqInfo is NULL", __FUNCTION__);
        return NETLINK_RET_GENERIC_FAILURE;
    }

    if (pReqInfo->interfaceId == INVALID_INTERFACE_ID) {
        NA_LOG_E("[%s] new pReqInfo interfaceId is invalid", __FUNCTION__);
        return NETLINK_RET_GENERIC_FAILURE;
    }

    if (m_pLastNetlinkEventReqInfo == NULL) {
        NA_LOG_E("[%s] m_pLastNetlinkEventReqInfo is NULL", __FUNCTION__);
        return NETLINK_RET_GENERIC_FAILURE;
    }

    int size = m_pLastNetlinkEventReqInfo->size();
    for (int i = 0; i < size; i++) {
        NetlinkEventReqInfo* pTemp = m_pLastNetlinkEventReqInfo->itemAt(i);

        if (pTemp->interfaceId == pReqInfo->interfaceId) {
            if ((pTemp->cmd != pReqInfo->cmd) || (pTemp->action != pReqInfo->action) ||
                (pTemp->ipv6PrefixLength != pReqInfo->ipv6PrefixLength) ||
                (strncasecmp(pTemp->address, pReqInfo->address, INET6_ADDRSTRLEN) != 0)) {
                NA_LOG_D("[%s] change, old %s, new %s", __FUNCTION__,
                         reqInfoToString(pTemp).string(), reqInfoToString(pReqInfo).string());
                ret = NETLINK_RET_REQ_INFO_CHANGED;
            } else {
                ret = NETLINK_RET_REQ_INFO_NO_CHANGED;
            }
            break;
        }
    }

    return ret;
}

void NetlinkEventHandler::freeNetlinkEventObj(void* obj) {
    NetlinkEventReqInfo* pReqInfo = reinterpret_cast<NetlinkEventReqInfo*>(obj);
    freeNetlinkEventObj(pReqInfo);
}

void NetlinkEventHandler::freeNetlinkEventObj(NetlinkEventReqInfo* pReqInfo) {
    if (pReqInfo == NULL) {
        NA_LOG_E("[%s] pReqInfo is NULL", __FUNCTION__);
        return;
    }
    free(pReqInfo);
}

int NetlinkEventHandler::stringToBinaryBase(char* str, int base, int* err) {
    int out;
    long l;
    char* end;
    *err = 0;

    if (str == NULL) {
        *err = -1;
        return 0;
    }

    l = strtoul(str, &end, base);
    out = (int)l;

    if (end == str) {
        *err = -2;
        return 0;
    }
    return out;
}

const char* NetlinkEventHandler::actionToString(ACTION action) {
    switch (action) {
        case ACTION_ADDR_UPDATED:
            return "AddressUpdated";
        case ACTION_ADDR_REMOVED:
            return "AddressRemoved";
        default:
            return "Unknown";
    }
}

String8 NetlinkEventHandler::reqInfoToString(NetlinkEventReqInfo* pReqInfo) {
    return String8::format(
            "ReqInfo <- {interfaceId=%d, cmd=%s, action=%s"
            ", address=%s, ipv6PrefixLength=%d}",
            pReqInfo->interfaceId, NetAgentService::cmdToString(pReqInfo->cmd),
            actionToString(pReqInfo->action), pReqInfo->address, pReqInfo->ipv6PrefixLength);
}

int NetlinkEventHandler::getTransIntfId(int interfaceId) {
    if (m_pNetAgent == NULL) {
        NA_LOG_E("[%s] m_pNetAgent is NULL", __FUNCTION__);
        return INVALID_TRANS_INTF_ID;
    }

    return m_pNetAgent->getTransIntfId(interfaceId);
}
