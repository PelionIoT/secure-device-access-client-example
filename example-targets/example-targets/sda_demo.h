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

#ifndef __SDA_DEMO_H__
#define __SDA_DEMO_H__

#include <inttypes.h>

void demo_setup(void);
void display_faulty_message(const char *fault_message);
bool demo_callback_configure(int64_t temperature);
bool demo_callback_read_data(void);
bool demo_callback_update(void);
bool demo_callback_restart(void);
bool demo_callback_diagnostics(void);

#endif //__SDA_DEMO_H__
