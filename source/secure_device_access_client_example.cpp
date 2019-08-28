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

// Note: this macro is needed on armcc to get the the PRI*32 macros
// from inttypes.h in a C++ code.
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "mcc_common_setup.h"
#include "sda_macros.h"

#include "pal.h"

#include "factory_configurator_client.h"
#include "key_config_manager.h"

#include "sda_status.h"
#include "secure_device_access.h"
#include "ftcd_comm_base.h"
#include "sda_comm_helper.h"
#include "mbed-trace/mbed_trace.h"
#include "mbed-trace-helper.h"
#include "mbed_stats_helper.h"
#include "sda_demo.h"

/////////////////////// DEFINITIONS ///////////////////////

#define TRACE_GROUP           "sdae"

///////////////////////// GLOBALS /////////////////////////

static int g_demo_main_status = EXIT_FAILURE;   // holds the demo main task return code

extern const uint8_t MBED_CLOUD_TRUST_ANCHOR_PK[];
extern const uint32_t MBED_CLOUD_TRUST_ANCHOR_PK_SIZE;
extern const char MBED_CLOUD_TRUST_ANCHOR_PK_NAME[];

char *g_endpoint_name = NULL;

static uint8_t g_app_user_response_buff[] = "This is app data buffer";

/** Checks if access allowed for the target operation
*
* @param operation_context[in] - The operation context
* @param func_name[in] - Function name in its string representation
* @param func_name_size[in] - Function name length
*
* @return SDA_STATUS_SUCCESS in case of success or one of sda_status_e otherwise.
*/
static sda_status_e is_operation_permitted(sda_operation_ctx_h operation_context, const uint8_t *func_name, size_t func_name_size)
{
    sda_status_e status;
    const uint8_t *scope;
    size_t scope_size;

    do {

        scope = NULL;
        scope_size = 0;

        // Get next available scope in list
        status = sda_scope_get_next(operation_context, &scope, &scope_size);

        // Check if end of scope list reached
        if (status == SDA_STATUS_NO_MORE_SCOPES) {
            tr_error("No match found for operation, permission denied");
            goto access_denied;
        }

        if (status != SDA_STATUS_SUCCESS) {
            tr_error("Failed getting scope, permission denied");
            goto access_denied;
        }

        if ((scope == NULL) || (scope_size == 0)) {
            tr_warn("Got empty or invalid scope, skipping this scope");
            continue;
        }

        // Check operation is in scope

        // Check that function name has the exact scope size
        if (scope_size != func_name_size) {
            continue;
        }

        // Check that function name and scope are binary equal
        if (memcmp(func_name, scope, func_name_size) != 0) {
            continue;
        }

        tr_info("Operation in scope, access granted");

        return SDA_STATUS_SUCCESS; // operation permitted

    } while (true);

access_denied:

    // Access denied

    tr_error("Operation not in scope, access denied");

    display_faulty_message("Access Denied");

    return status;
}


sda_status_e application_callback(sda_operation_ctx_h handle, void *callback_param)
{
    sda_status_e sda_status = SDA_STATUS_SUCCESS;
    sda_status_e sda_status_for_response = SDA_STATUS_SUCCESS;
    sda_command_type_e command_type = SDA_OPERATION_NONE;
    const uint8_t *func_callback_name;
    size_t func_callback_name_size;
    bool success = false; // assume error

    SDA_UNUSED_PARAM(callback_param);

    sda_status = sda_command_type_get(handle, &command_type);
    if (sda_status != SDA_STATUS_SUCCESS) {
        tr_error("Secure-Device-Access failed getting command type (%u)", sda_status);
        sda_status_for_response = sda_status;
        goto out;
    }

    // Currently only SDA_OPERATION_FUNC_CALL is supported
    if (command_type != SDA_OPERATION_FUNC_CALL) {
        tr_error("Got invalid command-type (%u)", command_type);
        sda_status_for_response = SDA_STATUS_INVALID_REQUEST;
        goto out;
    }

    func_callback_name = NULL;
    func_callback_name_size = 0;

    sda_status = sda_func_call_name_get(handle, &func_callback_name, &func_callback_name_size);
    if (sda_status != SDA_STATUS_SUCCESS) {
        tr_error("Secure-Device-Access failed getting function callback name (%u)", sda_status);
        sda_status_for_response = sda_status;
        goto out;
    }

    tr_info("Function callback is %.*s", func_callback_name_size, func_callback_name);

    /***
    * The following commands represents two demos as listed below:
    *   - MWC (Mobile World Congress):
    *     1. "configure"
    *     2. "read-data"
    *     3. "update"
    *
    *   - Hannover Mess:
    *     1. "diagnostics"
    *     2. "restart"
    *     3. "update"
    *
    *   - Note that "update" is a common command for both.
    */

    if (memcmp(func_callback_name, "configure", func_callback_name_size) == 0) {

        /***
        * This function accesses the LCD peripheral and sets the current outside temperature.
        * The provided scope must be in form of demo_callback_update_temperature.
        * It gets the temperature value to set as a parameter
        */

        int64_t temperature = 0;

        // Check permission
        sda_status = is_operation_permitted(handle, func_callback_name, func_callback_name_size);
        if (sda_status != SDA_STATUS_SUCCESS) {
            tr_error("demo_callback_configure() operation not permitted (%u)", sda_status);
            sda_status_for_response = sda_status;
            goto out;
        }

        // Get the temperature to display
        sda_status = sda_func_call_numeric_parameter_get(handle, 0, &temperature);
        if (sda_status != SDA_STATUS_SUCCESS) {
            tr_error("Failed getting demo_callback_configure() numeric param[0] (%u)", sda_status);
            sda_status_for_response = sda_status;
            goto out;
        }

        // Dispatch function callback
        success = demo_callback_configure(temperature);
        if (!success) {
            tr_error("demo_callback_configure() failed");
            sda_status_for_response = SDA_STATUS_OPERATION_EXECUTION_ERROR;
            goto out;
        }

    }
    else if (memcmp(func_callback_name, "read-data", func_callback_name_size) == 0) {

        /***
        * This function accesses the TEMPERATURE peripheral and query current outside temperature,
        * and displays it on LCD if the provided scope is in form of demo_callback_read_temperature.
        * This function has no inbound parameters.
        */

        // Check permission
        sda_status = is_operation_permitted(handle, func_callback_name, func_callback_name_size);
        if (sda_status != SDA_STATUS_SUCCESS) {
            tr_error("demo_callback_read_data() operation not permitted (%u)", sda_status);
            sda_status_for_response = sda_status;
            goto out;
        }

        // Dispatch function callback
        success = demo_callback_read_data();
        if (!success) {
            tr_error("demo_callback_read_data() failed");
            sda_status_for_response = SDA_STATUS_OPERATION_EXECUTION_ERROR;
            goto out;
        }

    }
    else if (memcmp(func_callback_name, "update", func_callback_name_size) == 0) {

        /***
        * Shows progress indicator on the LCD with percentages, and after a few seconds displays
        * "Firmware update successful" with a success LED.
        * This function has no inbound parameters.
        */

        // Check permission
        sda_status = is_operation_permitted(handle, func_callback_name, func_callback_name_size);
        if (sda_status != SDA_STATUS_SUCCESS) {
            tr_error("demo_callback_update() operation not permitted (%u)", sda_status);
            sda_status_for_response = sda_status;
            goto out;
        }

        // Dispatch function callback
        demo_callback_update();

    }
    else if (memcmp(func_callback_name, "diagnostics", func_callback_name_size) == 0) {

        /***
        * This function accesses the TEMPERATURE peripheral and query current outside temperature,
        * and displays it on LCD if the provided scope is in form of demo_callback_read_temperature.
        * This function has no inbound parameters.
        */

        // Check permission
        sda_status = is_operation_permitted(handle, func_callback_name, func_callback_name_size);
        if (sda_status != SDA_STATUS_SUCCESS) {
            tr_error("demo_callback_diagnostics() operation not permitted (%u)", sda_status);
            sda_status_for_response = sda_status;
            goto out;
        }

        // Dispatch function callback
        success = demo_callback_diagnostics();
        if (!success) {
            tr_error("demo_callback_diagnostics() failed");
            sda_status_for_response = SDA_STATUS_OPERATION_EXECUTION_ERROR;
            goto out;
        }

    }
    else if (memcmp(func_callback_name, "restart", func_callback_name_size) == 0) {

        /***
        * This function accesses the LCD peripheral and sets the current outside temperature.
        * The provided scope must be in form of demo_callback_update_temperature.
        * It gets the temperature value to set as a parameter
        */

        // Check permission
        sda_status = is_operation_permitted(handle, func_callback_name, func_callback_name_size);
        if (sda_status != SDA_STATUS_SUCCESS) {
            tr_error("demo_callback_restart() operation not permitted (%u)", sda_status);
            sda_status_for_response = sda_status;
            goto out;
        }

        // Dispatch function callback
        demo_callback_restart();

    }
    else {
        tr_error("Unsupported callback function name (%.*s)", (int)func_callback_name_size, func_callback_name);
        sda_status_for_response = SDA_STATUS_INVALID_REQUEST;
        goto out;
    }

    sda_status = sda_response_data_set(handle, g_app_user_response_buff, sizeof(g_app_user_response_buff));
    if (sda_status != SDA_STATUS_SUCCESS) {
        tr_error("sda_response_data_set failed (%u)", sda_status);
        sda_status_for_response = sda_status;
        goto out;
    }

    // flow succeeded
    tr_info("(%.*s) execution succeeded", func_callback_name_size, func_callback_name);
out: 

    if ((sda_status_for_response != SDA_STATUS_SUCCESS) && (sda_status_for_response != SDA_STATUS_NO_MORE_SCOPES)) {
        // Notify some fault happen (only if not 'access denied')
        display_faulty_message("Bad Request");
    }

    return sda_status_for_response;

}
/** Processes a request fetched from the communication line medium and
* returns the corresponding response message to be fed into the communication
* line medium back.
*
* @param request[in] - A request blob
* @param request_size[in] - A request blob size in bytes
* @param response[out] - The response blob
* @param response_max_size[in] - The response buffer max size in bytes
* @param response_actual_size[out] - The response blob size in bytes that was
*        effectively written (should be less or equal to the response_max_size buffer)
*
* @return "true" in case of success "false" otherwise.
*/
static bool process_request_fetch_response(
    const uint8_t *request,
    uint32_t request_size,
    uint8_t *response,
    size_t response_max_size,
    size_t *response_actual_size)
{

    sda_status_e sda_status = SDA_STATUS_SUCCESS;

    //Call to sda_operation_process to process current message, the response message will be returned as output.
    sda_status = sda_operation_process(request, request_size, *application_callback, NULL, response, response_max_size, response_actual_size);
    if (sda_status != SDA_STATUS_SUCCESS) {
        tr_error("Secure-Device-Access operation process failed (%u)", sda_status);
    }

    if (*response_actual_size != 0) {
        return true;
    }
    else {
        return false;
    }

}


// Get endpoint name and store it in g_endpoint_name for latter use
static bool get_endpoint_name()
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
    char* endpoint_name = NULL;
    size_t endpoint_buffer_size;
    size_t endpoint_name_size;


    // Get endpoint name data size
    kcm_status = kcm_item_get_data_size((const uint8_t*)g_fcc_endpoint_parameter_name, strlen(g_fcc_endpoint_parameter_name), KCM_CONFIG_ITEM, &endpoint_name_size);
    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("kcm_item_get_data_size failed (%u)", kcm_status);
        return false;
    }

    endpoint_buffer_size = endpoint_name_size + 1; /* for '\0' */

    endpoint_name = (char*)malloc(endpoint_buffer_size);
    if (endpoint_name == NULL) {
        return false;
    }

    memset(endpoint_name, 0, endpoint_buffer_size);

    kcm_status = kcm_item_get_data((const uint8_t*)g_fcc_endpoint_parameter_name, strlen(g_fcc_endpoint_parameter_name), KCM_CONFIG_ITEM, (uint8_t*)endpoint_name, endpoint_name_size, &endpoint_name_size);
    if (kcm_status != KCM_STATUS_SUCCESS) {
        free(endpoint_name);
        tr_error("kcm_item_get_data failed (%u)", kcm_status);
        return false;
    }

    // save endpoint_name for later use
    g_endpoint_name = endpoint_name;

    tr_cmdline("Endpoint name: %s", endpoint_name);

    return true;
}

static bool factory_setup(void)
{
#if MBED_CONF_APP_DEVELOPER_MODE == 1
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
#endif
    fcc_status_e fcc_status = FCC_STATUS_SUCCESS;
    bool status = true;

    // In both of this cases we call fcc_verify_device_configured_4mbed_cloud() to check if all data was provisioned correctly.

    // Initializes FCC to be able to call FCC APIs
    // TBD: SDA should be able to run without FCC init
    fcc_status = fcc_init();
    if (fcc_status != FCC_STATUS_SUCCESS) {
        status = false;
        tr_error("Failed to initialize Factory-Configurator-Client (%u)", fcc_status);
        goto out;
    }

#if MBED_CONF_APP_DEVELOPER_MODE == 1
    // Storage delete
    fcc_status = fcc_storage_delete();
    if (fcc_status != FCC_STATUS_SUCCESS) {
        tr_error("Storage format failed (%u)", fcc_status);
        status = false;
        goto out;
    }
    // Call developer flow
    tr_cmdline("Start developer flow");
    fcc_status = fcc_developer_flow();
    if (fcc_status != FCC_STATUS_SUCCESS) {
        tr_error("fcc_developer_flow failed (%u)", fcc_status);
        status = false;
        goto out;
    }

    // Store trust anchor
    // Note: Until TA will be part of the developer flow.
    tr_info("Store trust anchor");
    kcm_status = kcm_item_store((const uint8_t*)MBED_CLOUD_TRUST_ANCHOR_PK_NAME, strlen(MBED_CLOUD_TRUST_ANCHOR_PK_NAME), KCM_PUBLIC_KEY_ITEM, true, MBED_CLOUD_TRUST_ANCHOR_PK, MBED_CLOUD_TRUST_ANCHOR_PK_SIZE, NULL);
    if (kcm_status != KCM_STATUS_SUCCESS) {
        tr_error("kcm_item_store failed (%u)", kcm_status);
        status = false;
        goto out;
    }
#endif

    fcc_status = fcc_verify_device_configured_4mbed_cloud();
    if (fcc_status != FCC_STATUS_SUCCESS) {
        status = false;
        goto out;
    }

    //Get endpoint name
    status = get_endpoint_name();
    if (status != true) {
        tr_error("get_endpoint_name failed");
    }

out:
    // Finalize FFC
    fcc_status = fcc_finalize();
    if (status == false) {
        return false;
    } else {
        if (fcc_status != FCC_STATUS_SUCCESS) {
            tr_error("Failed finalizing Factory-Configurator-Client");
            return false;
        }
    }

    return status;
}

/**
* Main demo task
*/
static void demo_main()
{
    bool success;
    sda_status_e sda_status = SDA_STATUS_SUCCESS;
    ftcd_comm_status_e ftcd_status;
    FtcdCommBase *comm = NULL;
    uint8_t *request = NULL;
    uint32_t request_size = 0;
    uint8_t response[SDA_RESPONSE_HEADER_SIZE + sizeof(g_app_user_response_buff)];
    size_t response_max_size = sizeof(response);
    size_t response_actual_size;

    mcc_platform_sw_build_info();

    tr_cmdline("Secure-Device-Access initialization");

    // Initialize storage
    success = mcc_platform_storage_init() == 0;
    if (success != true) {
        tr_error("Failed initializing mcc platform storage\n");
        return;
    }

    // Avoid standard output buffering
    setvbuf(stdout, (char *)NULL, _IONBF, 0);

    success = factory_setup();
    if (success != true) {
        tr_error("Demo setup failed");
        goto out;
    }

    // Create communication interface object
    comm = sda_create_comm_interface();
    if (comm == NULL) {
        tr_error("Failed creating communication object");
        display_faulty_message("Init. failed");
        goto out;
    }

    //init sda_comm object
    success = comm->init();
    if (success != true) {
        tr_error("Failed instantiating communication object");
        display_faulty_message("Init. failed");
        goto out;
    }

    sda_status = sda_init();
    if (sda_status != SDA_STATUS_SUCCESS) {
        tr_error("Failed initializing Secure-Device-Access");
        display_faulty_message("Init. failed");
        goto out;
    }

    if (pal_osGetTime() != 0) {
        // Note - please keep after sda_init() to assure clock is disabled.
        // This is workaround for special case where MCCE is run before SDAE.
        // In such case, the device time will be set upon connection to LWM2M.
        // If the device has no RTC (real time clock), the time on the device won't be correct after reboot
        // And it will lead to failure in validation of access token time.
        tr_warn("For demo propose only, setting time to 0");
        pal_osSetTime(0);
    }

    // demo setup
    demo_setup();

    tr_cmdline("Secure-Device-Access demo start");

    do { // loop forever

        ftcd_status = comm->wait_for_message(&request, &request_size);
        if (ftcd_status != FTCD_COMM_STATUS_SUCCESS) {
            tr_error("Failed receiving Secure-Device-Access message (%u)", ftcd_status);
            display_faulty_message("Bad Request");
            goto out;
        }

        // clear response message buffer
        memset(response, 0, sizeof(response));
        response_actual_size = 0;

        success = process_request_fetch_response(request, request_size, response, response_max_size, &response_actual_size);
        if (!success) {
            tr_error("Failed processing request message");
            free(request);
            goto out;
        }

        ftcd_status = comm->send_response(response, response_actual_size);
        if (ftcd_status != FTCD_COMM_STATUS_SUCCESS) {
            tr_error("Failed sending Secure-Device-Access response message (%u)", ftcd_status);
            display_faulty_message("Failed to respond");
            free(request);
            goto out;
        }

        free(request);

        request = NULL;
        request_size = 0;

#if defined(MBED_HEAP_STATS_ENABLED) || defined(MBED_STACK_STATS_ENABLED)
        print_mbed_stats();
#endif
    } while (true);


    // reaching here means we had a failure,
    // evacuate resources and return callee
out:

    // Marks that this task has failed
    g_demo_main_status = EXIT_FAILURE;

    // Free the endpoint name string buffer
    if (g_endpoint_name) {
        free(g_endpoint_name);
        g_endpoint_name = NULL;
    }

    // Finalize SDA
    sda_status = sda_finalize();
    if (sda_status != SDA_STATUS_SUCCESS) {
        tr_error("Failed finalizing Secure-Device-Access");
    }

    // Disconnect communication module
    if (comm != NULL) {
        comm->finish();
        delete comm;
        sda_destroy_comm_interface();
    }

    // Flush standard output leftovers
    fflush(stdout);
}

/**
* Example main (entry point)
*/
int main(int argc, char * argv[])
{
    bool success = false;

    // careful, mbed-trace initialization may happen at this point if and only if we 
    // do NOT use mutex by passing "true" at the second param for this functions.
    // In case mutex is used, this function MUST be moved *after* pal_init()
    success = mbed_trace_helper_init(TRACE_ACTIVE_LEVEL_ALL | TRACE_MODE_COLOR, false);
    if (!success) {
        // Nothing much can be done here, trace module should be initialized before file system
        // and if failed - no tr_* print is eligible.
        return EXIT_FAILURE;
    }

    success = (mcc_platform_init() == 0);
    if (success) {
        success = mcc_platform_run_program(&demo_main);
    }

    mbed_trace_helper_finish();

    return success ? g_demo_main_status : EXIT_FAILURE;
}
