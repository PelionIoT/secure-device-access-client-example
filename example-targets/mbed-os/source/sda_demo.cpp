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

#include "sda_demo.h"
#include "mbed.h"
#include "PinNames.h"
#include "mbed-trace/mbed_trace.h"
#include "mbed-trace-helper.h"
#include "mcc_common_setup.h"

/////////////////////// DEFINITIONS ///////////////////////

#define TRACE_GROUP           "sdae"

// Color value is simple bitfield of 3 led bits (blue, green, red)
#define LED_CL_BLACK  7 // OFF
#define LED_CL_RED    6
#define LED_CL_GREEN  5
#define LED_CL_YELLOW 4
#define LED_CL_BLUE   3
#define LED_CL_PINK   2
#define LED_CL_CYAN   1
#define LED_CL_WHITE  0

#define LED_OFF 1
#define LED_ON  0


/////////////////////// STRUCTURES ////////////////////////


///////////////////////// GLOBALS /////////////////////////

extern char *g_endpoint_name;

DigitalOut led_red(LED_RED, LED_OFF);
DigitalOut led_green(LED_GREEN, LED_OFF);
DigitalOut led_blue(LED_BLUE, LED_OFF);

//////////////////////////////////////////////////////////

static void set_led_color(uint32_t color)
{
    // extract bits from bitfield
    led_red = (color & 1);
    led_green = (color & 2) >> 1;
    led_blue = (color & 4) >> 2;
}

static uint32_t get_led_color()
{
    // combine bits and return bitfield
    return (uint32_t)(led_red.read() | (led_green.read() << 1) | (led_blue.read() << 2));
}

static void emulate_operation(const char* operation_name, uint32_t color, size_t duration_in_sec)
{
    uint32_t prev_color = get_led_color();

    if (operation_name) {
        tr_cmdline("proccess operation %s", operation_name);
    }

    for (size_t i = 0; i < duration_in_sec * 2; i++)
    {
        set_led_color(LED_CL_BLACK);
        mcc_platform_do_wait(250);
        set_led_color(color);
        mcc_platform_do_wait(250);
    }

    set_led_color(prev_color);
}

void demo_setup(void)
{
    set_led_color(LED_CL_GREEN);
}

void display_faulty_message(const char *fault_message)
{
    tr_error("error message: %s", fault_message);
    emulate_operation(NULL, LED_CL_RED, 10);
}

bool demo_callback_read_data(void)
{
    emulate_operation("read", LED_CL_BLUE, 10);
    return true;
}

bool demo_callback_configure(int64_t temperature)
{
    (void) temperature;
    emulate_operation("configure", LED_CL_WHITE, 5);
    tr_error("Outside temperature: 28");
    return true;
}

bool demo_callback_update(void)
{
    emulate_operation("read", LED_CL_CYAN, 10);
    return true;
}

bool demo_callback_restart(void)
{
    emulate_operation("read", LED_CL_YELLOW, 10);
    return true;
}

bool demo_callback_diagnostics(void)
{
    emulate_operation("read", LED_CL_PINK, 5);
    return true;
}
