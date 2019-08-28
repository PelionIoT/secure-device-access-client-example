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


#ifndef __MBED_CLOUD_TRUST_ANCHOR_CREDENTIALS_H__
#define __MBED_CLOUD_TRUST_ANCHOR_CREDENTIALS_H__

#if MBED_CONF_APP_DEVELOPER_MODE == 1
#error "Replace mbed_cloud_trust_anchor_credentials.c with your own developer cert."
#endif

#include <inttypes.h>

const uint8_t  MBED_CLOUD_TRUST_ANCHOR_PK[] = { 0 };
const uint32_t MBED_CLOUD_TRUST_ANCHOR_PK_SIZE = sizeof(MBED_CLOUD_TRUST_ANCHOR_PK);

const char     MBED_CLOUD_TRUST_ANCHOR_PK_NAME[] = "";


#endif //__MBED_CLOUD_TRUST_ANCHOR_CREDENTIALS_H__
