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

#include <cutils/sockets.h>

#define LOG_TAG "socket-unix"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include <cutils/android_get_control_file.h>
#include <mtk_log.h>

#include <cutils/android_get_control_env.h>

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(exp) (exp)  // KISS implementation
#endif

#if defined(__ANDROID__)
/* For the socket trust (credentials) check */
#include <cutils/android_filesystem_config.h>
#define __android_unused
#else
#define __android_unused __attribute__((__unused__))
#endif

bool socket_peer_is_trusted(int fd __android_unused) {
#if defined(__ANDROID__)
    ucred cr;
    socklen_t len = sizeof(cr);
    int n = getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &cr, &len);

    if (n != 0) {
        ALOGE("could not get socket credentials: %s\n", strerror(errno));
        return false;
    }

    if ((cr.uid != AID_ROOT) && (cr.uid != AID_SHELL)) {
        ALOGE("untrusted userid on other end of socket: userid %d\n", cr.uid);
        return false;
    }
#endif

    return true;
}

int socket_close(int sock) { return close(sock); }

int socket_set_receive_timeout(cutils_socket_t sock, int timeout_ms) {
    timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

ssize_t socket_send_buffers(cutils_socket_t sock, const cutils_socket_buffer_t* buffers,
                            size_t num_buffers) {
    if (num_buffers > SOCKET_SEND_BUFFERS_MAX_BUFFERS) {
        return -1;
    }

    iovec iovec_buffers[SOCKET_SEND_BUFFERS_MAX_BUFFERS];
    for (size_t i = 0; i < num_buffers; ++i) {
        // It's safe to cast away const here; iovec declares non-const
        // void* because it's used for both send and receive, but since
        // we're only sending, the data won't be modified.
        iovec_buffers[i].iov_base = const_cast<void*>(buffers[i].data);
        iovec_buffers[i].iov_len = buffers[i].length;
    }

    return writev(sock, iovec_buffers, num_buffers);
}

int android_get_control_socket(const char* name) {
    int fd = __android_get_control_from_env(ANDROID_SOCKET_ENV_PREFIX, name);

    if (fd < 0) return fd;

    // Compare to UNIX domain socket name, must match!
    struct sockaddr_un addr;
    socklen_t addrlen = sizeof(addr);
    int ret = TEMP_FAILURE_RETRY(getsockname(fd, (struct sockaddr*)&addr, &addrlen));
    if (ret < 0) return -1;
    char* path = NULL;
    if (asprintf(&path, ANDROID_SOCKET_DIR "/%s", name) < 0) return -1;
    if (!path) return -1;
    int cmp = strcmp(addr.sun_path, path);
    free(path);
    if (cmp != 0) return -1;

    // It is what we think it is
    return fd;
}
