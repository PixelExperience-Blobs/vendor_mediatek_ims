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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <linux/capability.h>
#include <linux/ipsec.h>
#include <sys/capability.h>
#include <cutils/properties.h>
#include "utils_xfrm.h"

struct typeent {
    const char* t_name;
    int t_type;
};

extern __u8 xfrm_xfrmproto_getbyname(char* name);
extern int xfrm_id_parse(xfrm_address_t* saddr_xfrm, struct xfrm_id* id, __u16* family, char* src,
                         char* dst, char* ipsec_type);
extern int xfrm_algo_parse(struct xfrm_algo* alg, char* name, char* key, char* buf, int max);
extern __u8 xfrm_dir_parse(char* dir_str);
extern int xfrm_mode_parse(__u8* mode, char* mode_str);
extern void xfrm_selector_parse(struct xfrm_selector* sel, char* src, char* dst,
                                enum PROTOCOL_TYPE protocol, char* src_port, char* dst_port);
extern void xfrm_encry_algo_parse(char* encry_src, char* name);
extern void xfrm_interg_algo_parse(char* interg_src, char* name);
