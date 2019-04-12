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

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            Global variables
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* PLUGIN */
plugin_t **Plugin;
plugin_init_t Plugin_init;
plugin_add_t Plugin_add;
plugin_deinit_t Plugin_deinit;

/* METRICS */
metrics_t **Metrics;
metrics_init_t Metrics_init;
metrics_add_t Metrics_add;
metrics_deinit_t Metrics_deinit;

/* SIZE */
max_size_t Max_size;

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            AFB INITIALIZATION
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

static void afb_init(afb_req_t req)
{
    json_object *res;
    json_object *res_plugin;
    json_object *arg;
    json_object *args;
    json_object *current_plugin;
    json_type args_type;
    int plugin_number;
    char *plugin_label;
    int loading_count;

    res = json_object_new_object();
    res_plugin = json_object_new_object();
    arg = json_object_new_object();
    args = json_object_new_object();
    current_plugin = json_object_new_object();


    arg = afb_req_json(req);
    args_type = json_object_get_type(args);
    loading_count = 0;

    // Invalid request
	if (!json_object_object_get_ex(arg, "plugin", &args)){
		afb_req_fail_f(req, "ERROR", "Invalid request in %s.", json_object_get_string(arg));
		return;
	}

    args_type = json_object_get_type(args);
    switch(args_type)
    {
        case json_type_string:
			plugin_number = 1;
			break;

        case json_type_array:
			plugin_number = json_object_array_length(args);
            printf("Plugin number : %d", plugin_number);
			break;

        default:
            afb_req_fail_f(req, "ERROR", "Invalid type in %s.", json_object_get_string(args));
            return;
    }

    for(int i = 0 ; i != plugin_number ; i++)
    {
        if (args_type == json_type_array){
            current_plugin = json_object_array_get_idx(args, i);
        }

        else{
            current_plugin = args;
        }

        if (! json_object_is_type(current_plugin, json_type_string)) {
			AFB_REQ_ERROR(req, "current plugin is not correct ('%s')", json_object_get_string(current_plugin));
			continue;
		}

        plugin_label = (char *) json_object_get_string(current_plugin);

        if (!strncmp(plugin_label, "cpu", (*Max_size)((size_t) 3, strlen(plugin_label))))
        {
            if((*Plugin_add)(plugin_label))
            {
                AFB_REQ_ERROR(req, "Plugin %s is already stored", plugin_label);
                continue;
            }
            else
            {
                wrap_json_pack(&res_plugin,
                           "{ss}",
                           "plugin", "cpu");
            json_object_object_add(res, "Loaded", res_plugin);
            loading_count ++;
            continue;
            }
        }

        AFB_REQ_WARNING(req, "Plugin %s is not known", plugin_label);
    }

    if(!loading_count)
    {
        afb_req_fail(req, NULL, "None of the plugin(s) specified had been stored.");
        return;
    }
    afb_req_success(req, res, "Plugin initialization succeed");
    return;
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            AFB CONFIGURATION
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

static void afb_config(afb_req_t request)
{
    /* Retrieve the api for debug/error message */
    afb_api_t api;
    api = afb_req_get_api(request);

    /* Initialize handle to make a link with the dynamic cpu library */
    void *handle_cpu = NULL;
    handle_cpu = dlopen(CPU_PATH, RTLD_NOW | RTLD_GLOBAL);
    if(!handle_cpu)
    {
        AFB_API_ERROR(api, "dlerror dlopen cpu %s", dlerror());
        afb_req_fail(request, NULL, "dlerror dlopen cpu");
        return;
    }

    /* We want to get the module register function relative to the cpu library */
    module_register_t module_register;
    module_register = (module_register_t) dlsym(handle_cpu, "module_register");
    if(!module_register)
    {
        AFB_API_ERROR(api, "dlerror module register : %s", dlerror());
        afb_req_fail(request, NULL, "dlerror module register");
        return;
    }

    AFB_API_NOTICE(api, "CPU initialization");

    /* Call the module_register function to fill the plugin variable with the cpu callbacks */
    (*module_register)();

    if(!(*Plugin)->plugin_callback->init)
    {
        afb_req_fail(request, NULL, "Initialization callback NULL");
        return;
    }

    if(!(*Plugin)->plugin_callback->config)
    {
        afb_req_fail(request, NULL, "Configuration callback NULL");
        return;
    }

    AFB_API_NOTICE(api, "CPU initialization finished");
    afb_req_success(request, NULL, "Plugin configured");
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            AFB READ ONCE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
static void afb_read_once(afb_req_t request)
{
    afb_api_t api;
    api = afb_req_get_api(request);
    if(!(*Plugin)->plugin_callback->read)
    {
        afb_req_fail(request, NULL, "Read callbacks NULL");
        return;
    }
    (*(*Plugin)->plugin_callback->init)();

    (*(*Plugin)->plugin_callback->config)("ReportByCpu", "true");
    (*(*Plugin)->plugin_callback->config)("ReportByState", "true");
    (*(*Plugin)->plugin_callback->config)("ValuesPercentage", "true");
    /* (*(*Plugin)->plugin_callback->config)("ReportNumCpu", "true"); */
    /* (*(*Plugin)->plugin_callback->config)("ReportGuestState", "true"); */
    /* (*(*Plugin)->plugin_callback->config)("ReportGuestState", "true"); */

    (*(*Plugin)->plugin_callback->read)(NULL);

    if(*Metrics)
    {
        AFB_API_NOTICE(api, "Metrics loaded");
        printf("Number of metrics : %ld\n", (*Metrics)->size);
        for(int i = 0 ; i != (*Metrics)->size ; i++)
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
        }

        afb_req_success(request, NULL, "Plugin succesfully read");
        (*Metrics_deinit)();
    }
    else
    {
        AFB_API_NOTICE(api, "Metrics failed to load");
        afb_req_fail(request, NULL, "Fails to read the plugin");
    }
    return;
}

// Binder initialization function
static int initialization(afb_api_t api)
{
    void *handle_plugin;
    AFB_API_NOTICE(api, "Initialization");

    /* Load the plugin library */
    handle_plugin = dlopen(PLUGIN_PATH, RTLD_NOW | RTLD_GLOBAL);
    if(!handle_plugin)
    {
        AFB_API_ERROR(api, "dlerror dlopen plugin %s", dlerror());
        return -1;
    }

    /* Link our global functions to the one in the plugin library */
    Plugin_init = (plugin_init_t)dlsym(handle_plugin, "plugin_init");
    if(!Plugin_init)
    {
        AFB_API_ERROR(api, "dlerror init plugin symbol : %s", dlerror());
        return -1;
    }

    Plugin_add = (plugin_add_t)dlsym(handle_plugin, "plugin_add");
    if(!Plugin_add)
    {
        AFB_API_ERROR(api, "dlerror add plugin symbol : %s", dlerror());
        return -1;
    }

    Plugin_deinit = (plugin_deinit_t)dlsym(handle_plugin, "plugin_deinit");
    if(!Plugin_deinit)
    {
        AFB_API_ERROR(api, "dlerror deinit plugin symbol : %s", dlerror());
        return -1;
    }

    Metrics_init = (metrics_init_t)dlsym(handle_plugin, "metrics_init");
    if(!Metrics_init)
    {
        AFB_API_ERROR(api, "dlerror init metrics symbol : %s", dlerror());
        return -1;
    }

    Metrics_add = (metrics_add_t)dlsym(handle_plugin, "metrics_add");
    if(!Metrics_init)
    {
        AFB_API_ERROR(api, "dlerror add metrics symbol : %s", dlerror());
        return -1;
    }

    Metrics_deinit = (metrics_deinit_t)dlsym(handle_plugin, "metrics_deinit");
    if(!Metrics_deinit)
    {
        AFB_API_ERROR(api, "dlerror deinit metrics symbol : %s", dlerror());
        return -1;
    }

    Max_size = (max_size_t)dlsym(handle_plugin, "max_size");
    if(!Max_size)
    {
        AFB_API_ERROR(api, "dlerror max size symbol : %s", dlerror());
        return -1;
    }

    /* If the initialization fails */
    /* if( (*Plugin_init)() )
    {
        AFB_API_ERROR(api, "dlerror plugin initialization fail");
        return -1;
    } */

    /* Load the plugin variable */
    Plugin = (plugin_t**) dlsym(handle_plugin, "Plugin_collectd");
    if(!Plugin)
    {
        AFB_API_ERROR(api, "dlerror Plugin : %s", dlerror());
        return -1;
    }

    /* Load the metrics variable */
    Metrics = (metrics_t**) dlsym(handle_plugin, "Metrics_collectd");
    if(!Metrics)
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
   if(afb_api_add_verb(api, "init", "Initialize the global plugin variable with the plugin we want", afb_init, NULL, NULL, AFB_SESSION_NONE, 0)
   || afb_api_add_verb(api, "config", "Store the callbacks of the desired plugin", afb_config, NULL, NULL, AFB_SESSION_NONE, 0)
   || afb_api_add_verb(api, "readOnce", "Read the metrics plugin previously configured", afb_read_once, NULL, NULL, AFB_SESSION_NONE, 0)){
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
