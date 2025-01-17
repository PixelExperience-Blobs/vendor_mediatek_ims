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

#include "ares_setup.h"

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include "ares.h"
#include "ares_data.h"
#include "inet_net_pton.h"
#include "ares_private.h"

int ares_get_servers(ares_channel channel, struct ares_addr_node** servers) {
    struct ares_addr_node* srvr_head = NULL;
    struct ares_addr_node* srvr_last = NULL;
    struct ares_addr_node* srvr_curr;
    int status = ARES_SUCCESS;
    int i;

    if (!channel) return ARES_ENODATA;

    for (i = 0; i < channel->nservers; i++) {
        /* Allocate storage for this server node appending it to the list */
        srvr_curr = ares_malloc_data(ARES_DATATYPE_ADDR_NODE);
        if (!srvr_curr) {
            status = ARES_ENOMEM;
            break;
        }
        if (srvr_last) {
            srvr_last->next = srvr_curr;
        } else {
            srvr_head = srvr_curr;
        }
        srvr_last = srvr_curr;

        /* Fill this server node data */
        srvr_curr->family = channel->servers[i].addr.family;
        if (srvr_curr->family == AF_INET)
            memcpy(&srvr_curr->addrV4, &channel->servers[i].addr.addrV4, sizeof(srvr_curr->addrV4));
        else
            memcpy(&srvr_curr->addrV6, &channel->servers[i].addr.addrV6, sizeof(srvr_curr->addrV6));
    }

    if (status != ARES_SUCCESS) {
        if (srvr_head) {
            ares_free_data(srvr_head);
            srvr_head = NULL;
        }
    }

    *servers = srvr_head;

    return status;
}

int ares_set_servers(ares_channel channel, struct ares_addr_node* servers) {
    struct ares_addr_node* srvr;
    int num_srvrs = 0;
    int i;

    if (ares_library_initialized() != ARES_SUCCESS) return ARES_ENOTINITIALIZED;

    if (!channel) return ARES_ENODATA;

    ares__destroy_servers_state(channel);

    for (srvr = servers; srvr; srvr = srvr->next) {
        num_srvrs++;
    }

    if (num_srvrs > 0) {
        /* Allocate storage for servers state */
        channel->servers = malloc(num_srvrs * sizeof(struct server_state));
        if (!channel->servers) {
            return ARES_ENOMEM;
        }
        channel->nservers = num_srvrs;
        /* Fill servers state address data */
        for (i = 0, srvr = servers; srvr; i++, srvr = srvr->next) {
            channel->servers[i].addr.family = srvr->family;
            if (srvr->family == AF_INET)
                memcpy(&channel->servers[i].addr.addrV4, &srvr->addrV4, sizeof(srvr->addrV4));
            else
                memcpy(&channel->servers[i].addr.addrV6, &srvr->addrV6, sizeof(srvr->addrV6));
        }
        /* Initialize servers state remaining data */
        ares__init_servers_state(channel);
    }

    return ARES_SUCCESS;
}

/* Incomming string format: host[:port][,host[:port]]... */
int ares_set_servers_csv(ares_channel channel, const char* _csv) {
    size_t i;
    char* csv = NULL;
    char* ptr;
    char* start_host;
    int rv = ARES_SUCCESS;
    struct ares_addr_node* servers = NULL;
    struct ares_addr_node* last = NULL;

    if (ares_library_initialized() != ARES_SUCCESS) return ARES_ENOTINITIALIZED;

    if (!channel) return ARES_ENODATA;

    ares__destroy_servers_state(channel);

    i = strlen(_csv);
    if (i == 0) return ARES_SUCCESS; /* blank all servers */

    csv = malloc(i + 2);
    if (csv == NULL) return ARES_ENOMEM;
    strcpy(csv, _csv);
    if (csv[i - 1] != ',') { /* make parsing easier by ensuring ending ',' */
        csv[i] = ',';
        csv[i + 1] = 0;
    }

    start_host = csv;
    for (ptr = csv; *ptr; ptr++) {
        if (*ptr == ',') {
            char* pp = ptr - 1;
            struct in_addr in4;
            struct ares_in6_addr in6;
            struct ares_addr_node* s = NULL;

            *ptr = 0; /* null terminate host:port string */
            /* Got an entry..see if port was specified. */
            while (pp > start_host) {
                if (*pp == ':') break; /* yes */
                if (!ISDIGIT(*pp)) {
                    /* Found end of digits before we found :, so wasn't a port */
                    pp = ptr;
                    break;
                }
                pp--;
            }
            if ((pp != start_host) && ((pp + 1) < ptr)) {
                /* Found it. Parse over the port number */
                (void)strtol(pp + 1, NULL, 10);
                *pp = 0; /* null terminate host */
            }
            /* resolve host, try ipv4 first, rslt is in network byte order */
            rv = ares_inet_pton(AF_INET, start_host, &in4);
            if (!rv) {
                /* Ok, try IPv6 then */
                rv = ares_inet_pton(AF_INET6, start_host, &in6);
                if (!rv) {
                    rv = ARES_EBADSTR;
                    goto out;
                }
                /* was ipv6, add new server */
                s = malloc(sizeof(*s));
                if (!s) {
                    rv = ARES_ENOMEM;
                    goto out;
                }
                s->family = AF_INET6;
                memcpy(&s->addr, &in6, sizeof(struct ares_in6_addr));
            } else {
                /* was ipv4, add new server */
                s = malloc(sizeof(*s));
                if (!s) {
                    rv = ARES_ENOMEM;
                    goto out;
                }
                s->family = AF_INET;
                memcpy(&s->addr, &in4, sizeof(struct in_addr));
            }
            if (s) {
                /* TODO:  Add port to ares_addr_node and assign it here. */

                s->next = NULL;
                if (last) {
                    last->next = s;
                } else {
                    servers = s;
                    last = s;
                }
            }

            /* Set up for next one */
            start_host = ptr + 1;
        }
    }

    rv = ares_set_servers(channel, servers);

out:
    if (csv) free(csv);
    while (servers) {
        struct ares_addr_node* s = servers;
        servers = servers->next;
        free(s);
    }

    return rv;
}
