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

#ifndef __FRAMEWORK_CMD_HANDLER_H
#define __FRAMEWORK_CMD_HANDLER_H

#include "List.h"

class SocketClient;

class FrameworkCommand {
  private:
    const char* mCommand;

  public:
    FrameworkCommand(const char* cmd);
    virtual ~FrameworkCommand() {}

    virtual int runCommand(SocketClient* c, int argc, char** argv) = 0;

    const char* getCommand() { return mCommand; }
};

typedef android::sysutils::List<FrameworkCommand*> FrameworkCommandCollection;
#endif
