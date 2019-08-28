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

#ifdef SDA_SERIAL_INTERFACE

#include "sda_comm_helper.h"
#include "ftcd_comm_serial.h"

FtcdCommBase *sda_create_comm_interface(void)
{
    const uint8_t msg_header_token[] = FTCD_MSG_HEADER_TOKEN_SDA;
    return new FtcdCommSerial(FTCD_COMM_NET_ENDIANNESS_BIG, msg_header_token, true);
}

void sda_destroy_comm_interface(void)
{
}

#endif // SDA_SERIAL_INTERFACE