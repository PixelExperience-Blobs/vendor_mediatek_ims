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

#include <cutils/trace.h>

#ifndef __unused
#define __unused __attribute__((__unused__))
#endif

atomic_bool atrace_is_ready = ATOMIC_VAR_INIT(true);
int atrace_marker_fd = -1;
uint64_t atrace_enabled_tags = 0;

void atrace_set_debuggable(bool debuggable __unused) {}
void atrace_set_tracing_enabled(bool enabled __unused) {}
void atrace_update_tags() {}
void atrace_setup() {}
void atrace_begin_body(const char* name __unused) {}
void atrace_async_begin_body(const char* name __unused, int32_t cookie __unused) {}
void atrace_async_end_body(const char* name __unused, int32_t cookie __unused) {}
void atrace_int_body(const char* name __unused, int32_t value __unused) {}
void atrace_int64_body(const char* name __unused, int64_t value __unused) {}
