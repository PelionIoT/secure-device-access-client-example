// ----------------------------------------------------------------------------
// Copyright 2017-2019 ARM Ltd.
//  
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//  
//     http://www.apache.org/licenses/LICENSE-2.0
//  
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

#ifndef __SDA_COMMON_HELPER_H__
#define __SDA_COMMON_HELPER_H__

#include <stdbool.h>
#include <stdlib.h>
#include "ftcd_comm_base.h"

/**
* The communication socket port
* If SDA_DAEMON_TCP_PORT defined as 0, random port will be generated.
*/
#define SDA_DAEMON_TCP_PORT 1111


#ifdef __cplusplus
extern "C" {
#endif

/**
* Instantiate new factory communication object.
* The communication object may be through serial, mbed-os-socket or Linux native socket.
*/
FtcdCommBase *sda_create_comm_interface(void);

/**
* Releases any OS specific resources that were allocated in CreateCommInterface().
* This function may be left empty if there is nothing to destroy or evacuate.
*/
void sda_destroy_comm_interface(void);


#ifdef __cplusplus
}
#endif

#endif //__SDA_COMMON_HELPER_H__
