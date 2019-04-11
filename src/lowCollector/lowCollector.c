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
#include <json-c/json.h>
#include </opt/AGL/include/wrap-json.h>

#define AFB_BINDING_VERSION 3
#include </opt/AGL/include/afb/afb-binding.h>
#include <dlfcn.h>

#include "../collectd/plugin.h"

#define PLUGIN_PATH "../build/src/collectd/./plugin.so"

plugin_t **Plugin;
metrics_t **Metrics;

static void config(afb_req_t request)
{
    /* Retrieve the api for debug/error message */
    afb_api_t api;
    api = afb_req_get_api(request);

    /* Initialize handle to make a link with the dynamic cpu library */
    void *handle_cpu = NULL;
    if( (handle_cpu = dlopen(CPU_PATH, RTLD_NOW | RTLD_GLOBAL)) == NULL)
    {
        AFB_API_ERROR(api, "dlerror dlopen cpu %s", dlerror());
        afb_req_fail(request, NULL, "dlerror dlopen cpu");
        return;
    }

    /* We want to get the module register function relative to the cpu library */
    module_register_t module_register;
    if( (module_register = (module_register_t) dlsym(handle_cpu, "module_register")) == NULL)
    {
        AFB_API_ERROR(api, "dlerror module register : %s", dlerror());
        afb_req_fail(request, NULL, "dlerror module register");
        return;
    }

    AFB_API_NOTICE(api, "CPU initialization");

    /* Call the module_register function to fill the plugin variable with the cpu callbacks */
    (*module_register)();

    if((*Plugin)->init == NULL)
    {
        afb_req_fail(request, NULL, "Initialization callback NULL");
        return;
    }

    if((*Plugin)->config == NULL)
    {
        afb_req_fail(request, NULL, "Configuration callback NULL");
        return;
    }

    AFB_API_NOTICE(api, "CPU initialization finished");
    afb_req_success(request, NULL, "Plugin configured");
}

static void simpleRead(afb_req_t request)
{
    afb_api_t api;
    api = afb_req_get_api(request);
    if((*Plugin)->read == NULL)
    {
        afb_req_fail(request, NULL, "Read callbacks NULL");
        return;
    }
    (*(*Plugin)->init)();

    (*(*Plugin)->config)("ReportByCpu", "true");
    (*(*Plugin)->config)("ReportByState", "true");
    (*(*Plugin)->config)("ValuesPercentage", "true");
    /* (*(*Plugin)->config)("ReportNumCpu", "true"); */
    /* (*(*Plugin)->config)("ReportGuestState", "true");
    (*(*Plugin)->config)("ReportGuestState", "true"); */

    (*(*Plugin)->read)(NULL);

    if(*Metrics != NULL)
    {
        AFB_API_NOTICE(api, "Metrics loaded");
        printf("Number of metrics : %ld\n", (*Metrics)->size);
        /* for(int i = 0 ; i != (*Metrics)->size ; i++)
        {
            printf("Value counter : %ld\n", (*Metrics)->metrics[i].values->counter);
            printf("Value gauge : %f\n", (*Metrics)->metrics[i].values->gauge);
            printf("Value absolute : %ld\n", (*Metrics)->metrics[i].values->absolute);
            printf("Value derive : %ld\n", (*Metrics)->metrics[i].values->derive);
            printf("size : %ld\n", (*Metrics)->metrics[i].values_len);
            printf("time : %ld\n", (*Metrics)->metrics[i].time);
            printf("Interval : %ld\n", (*Metrics)->metrics[i].interval);
            printf("Host : %s\n", (*Metrics)->metrics[i].host);
            printf("Plugin : %s\n", (*Metrics)->metrics[i].plugin);
            printf("Plugin instance : %s\n", (*Metrics)->metrics[i].plugin_instance);
            printf("Type : %s\n", (*Metrics)->metrics[i].type);
            printf("Type instance : %s\n\n", (*Metrics)->metrics[i].type_instance);
        } */

        afb_req_success(request, NULL, "Plugin succesfully read");
    return;
    }
    else
    {
        AFB_API_NOTICE(api, "Metrics failed to load");
        afb_req_fail(request, NULL, "Fails to read the plugin");
        return;
    }
}

// Binder initialization function
static int initialization(afb_api_t api)
{
    void *handle_plugin;
    plugin_init_t plugin_init;
    /* metrics_init_t metrics_init; */
    AFB_API_NOTICE(api, "Initialization");

    /* Load the plugin library */
    handle_plugin = dlopen(PLUGIN_PATH, RTLD_NOW | RTLD_GLOBAL);
    if( handle_plugin == NULL)
    {
        AFB_API_ERROR(api, "dlerror dlopen plugin %s", dlerror());
        return -1;
    }

    /* Load the plugin variable initialization */
    plugin_init = (plugin_init_t)dlsym(handle_plugin, "plugin_init");
    if( plugin_init == NULL)
    {
        AFB_API_ERROR(api, "dlerror init plugin symbol : %s", dlerror());
        return -1;
    }

    /* If the initialization fails */
    if( (*plugin_init)() )
    {
        AFB_API_ERROR(api, "dlerror plugin initialization fail");
        return -1;
    }

    /* Load the plugin variable */
    Plugin = (plugin_t**) dlsym(handle_plugin, "Plugin_collectd");
    if( Plugin == NULL)
    {
        AFB_API_ERROR(api, "dlerror Plugin : %s", dlerror());
        return -1;
    }

    /* Load the metrics variable */
    Metrics = (metrics_t**) dlsym(handle_plugin, "Metrics_collectd");
    if( Metrics == NULL)
    {
        AFB_API_ERROR(api, "dlerror Plugin : %s", dlerror());
        return -1;
    }

    AFB_API_NOTICE(api, "Initialization finished");
	return 0;
}

// Binder preinitialization
int preinit(afb_api_t api)
{
    AFB_API_NOTICE(api, "Preinitialization");

   // Set the verb of this API
   if(afb_api_add_verb(api, "config", "Initialize configure and start the cpu metrics collect", config, NULL, NULL, AFB_SESSION_NONE, 0)
   || afb_api_add_verb(api, "simpleRead", "Read only one time the plugin previously configured", simpleRead, NULL, NULL, AFB_SESSION_NONE, 0)){
        AFB_API_ERROR(api, "Verb initialization for API controller failed");
   }

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
