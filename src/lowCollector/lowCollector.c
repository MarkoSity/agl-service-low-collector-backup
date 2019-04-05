/*
 * Copyright (C) 2016-2018 "IoT.bzh"
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _GNU_SOURCE
#include </opt/AGL/include/wrap-json.h>
#include <json-c/json.h>

#define AFB_BINDING_VERSION 3
#include </opt/AGL/include/afb/afb-binding.h>
#include <dlfcn.h>

#include "plugin.h"

static void config(afb_req_t request)
{
    /* Retrieve the api for debug/error message */
    afb_api_t api;
    api = afb_req_get_api(request);

    AFB_API_NOTICE(api, "CPU initialization");

    /* Initialize handle to make a link with the dynamic cpu library */
    void* handle_cpu = NULL;
    if ((handle_cpu = dlopen("../collectd_plugin/./cpu.so", RTLD_NOW | RTLD_GLOBAL)) == NULL) {
        AFB_API_ERROR(api, "dlopen dlerror : %s", dlerror());
        return;
    }

    /* We want to get the module register function relative to the cpu library */
    module_register_t module_register;
    if ((module_register = dlsym(handle_cpu, "module_register")) == NULL) {
        AFB_API_ERROR(api, "dlsym dlerror : %s", dlerror());
        return;
    }

    /* Call the module_register function to fill the plugin variable with the cpu callbacks */
    (*module_register)();

    AFB_API_NOTICE(api, "CPU initialization finished");
    afb_req_success(request, NULL,"Config");
}

static void read_once(afb_req_t request)
{
    int value;
    afb_api_t api;
    api = afb_req_get_api(request);

    (*Plugin->init)();
    (*Plugin->config)("ReportByCpu", "true");
    value = (*Plugin->read)(NULL);
    AFB_API_NOTICE(api, "Read value : %d", value);
}

/* static void start(afb_req_t request)
{
    int value;

    (*Plugin->init)();
    (*Plugin->config)();
    pthread_create(func_read);
}
bool running = true;
static void func_read(void)
{
    running = true;
    while(running){
        value = (*Plugin->read)();
        AFB_API_NOTICE(api, "Read value : %d", value);
    }
}
static void stop(afb_req_t request)
{
    running = false;
} */

// Binder initialization function
static int initialization(afb_api_t api)
{
    AFB_API_NOTICE(api, "Initialization");
    plugin_init();
    AFB_API_NOTICE(api, "Initialization finished");
    return 0;
}

// Binder preinitialization
int preinit(afb_api_t api)
{
    AFB_API_NOTICE(api, "Preinitialization");

    // Set the verb of this API
    if (afb_api_add_verb(api, "config", "Initialize configure and start the cpu metrics collect", config, NULL, NULL, AFB_SESSION_NONE, 0)) {
        AFB_API_ERROR(api, "Verb initialization for API controller failed");
    }

    if (afb_api_add_verb(api, "read_once", "Initialize configure and start the cpu metrics collect", read_once, NULL, NULL, AFB_SESSION_NONE, 0)) {
        AFB_API_ERROR(api, "Verb initialization for API controller failed");
    }

    /* if (afb_api_add_verb(api, "stop", "Initialize configure and start the cpu metrics collect", stop, NULL, NULL, AFB_SESSION_NONE, 0)) {
        AFB_API_ERROR(api, "Verb initialization for API controller failed");
    } */

    // Define the initialization fonction
    afb_api_on_init(api, initialization);

    // Now the api is set and ready to work
    afb_api_seal(api);

    AFB_API_NOTICE(api, "Preinitialization finished");

    return 0;
}

const afb_binding_t afbBindingExport = {
    .api = "collector",
    .specification = NULL,
    .preinit = preinit,
    .provide_class = NULL,
    .require_class = NULL,
    .require_api = NULL,
    .noconcurrency = 0
};
