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

/* SETTINGS RESET */
cpu_settings_reset_t Cpu_settings_reset;
mem_settings_reset_t Mem_settings_reset;

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            AFB INITIALIZATION
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

static void afb_init(afb_req_t req)
{
    /* Variables definition */
    json_object *res;
    json_object *res_plugin;
    json_object *arg;
    json_object *args;
    json_object *current_plugin;
    json_type args_type;
    int plugin_number;
    char *plugin_label;
    int loading_count;

    /* Variables allocation */
    res = json_object_new_object();
    res_plugin = json_object_new_object();
    arg = json_object_new_object();
    args = json_object_new_object();
    current_plugin = json_object_new_object();

    /* Variables initialization */
    arg = afb_req_json(req);
    args_type = json_object_get_type(args);
    loading_count = 0;

    /* Invalid key */
	if(!json_object_object_get_ex(arg, "plugin", &args))
    {
        AFB_REQ_ERROR(req, "Invalid request in %s.", json_object_get_string(arg));
		afb_req_fail_f(req, "ERROR", "Invalid request in %s.", json_object_get_string(arg));
		return;
	}

    /* Retrieve the type of the arguments and store the number of plugin desired  */
    args_type = json_object_get_type(args);
    switch(args_type)
    {
        case json_type_string:
        {
            plugin_number = 1;
			break;
        }

        case json_type_array:
        {
            plugin_number = json_object_array_length(args);
			break;
        }

        default:
        {
            AFB_REQ_ERROR(req, "Invalid plugin type in %s.", json_object_get_string(args));
            afb_req_fail_f(req, "ERROR", "Invalid plugin type in %s.", json_object_get_string(args));
            return;
        }
    }

    /* For each one of the previous plugin */
    for(int i = 0 ; i != plugin_number ; i++)
    {
        /* Retrieve the current plugin */
        if(args_type == json_type_array)
        {
            current_plugin = json_object_array_get_idx(args, i);
        }

        else
        {
            current_plugin = args;
        }

        /* If the type ain't the one we expected we noticed it but we continue the process */
        if(!json_object_is_type(current_plugin, json_type_string))
        {
			AFB_REQ_ERROR(req, "Current plugin is not correct ('%s').", json_object_get_string(current_plugin));
			continue;
		}

        /* Retrieve the string in the current j-son object */
        plugin_label = (char *) json_object_get_string(current_plugin);


        /* If the current plugin is known */
        if(!strncmp(plugin_label, "cpu", (*Max_size)((size_t) 3, strlen(plugin_label)))
        || !strncmp(plugin_label, "mem", (*Max_size)((size_t) 3, strlen(plugin_label))))
        {
            /* Add to the plugin variable list the cpu, and if its already stored, we handle it */
            if((*Plugin_add)(plugin_label))
            {
                AFB_REQ_ERROR(req, "Plugin %s is already stored.", plugin_label);
                continue;
            }

            /* Notify that we stored a new plugin */
            else
            {
                wrap_json_pack(&res_plugin,
                           "{ss}",
                           "plugin", plugin_label);
                json_object_object_add(res, "Loaded", res_plugin);
                loading_count ++;
                continue;
            }
        }

        /* If we reach that point, it means that the plugin desired ain't known */
        AFB_REQ_WARNING(req, "Plugin %s is not known.", plugin_label);
    }

    /* If none of the previous plugins were known */
    if(!loading_count)
    {
        AFB_REQ_ERROR(req, "None of the plugin(s) specified had been stored.");
        afb_req_fail(req, NULL, "None of the plugin(s) specified had been stored.");
        return;
    }

    /* Notify that the initialization is a success */
    AFB_API_NOTICE(afbBindingV3root, "Verbosity macro at level notice invoked at init invocation");
    afb_req_success(req, res, "Plugin initialization succeed.");
    return;
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            AFB CONFIGURATION
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

static void afb_config(afb_req_t req)
{
    /* Variables definition */
    afb_api_t api;
    void *handle;
    module_register_t module_register;
    json_object *res;
    json_object *res_config;
    json_object *arg;
    json_object *args;
    json_type args_type;
    char *config_label;
    size_t plugin_index;
    bool plugin_found;
    
    /* Variables allocation */
    res = json_object_new_object();
    res_config = json_object_new_object();
    args = json_object_new_object();
    arg = json_object_new_object();

    /* Variables initialization */
    api = afb_req_get_api(req);
    arg = afb_req_json(req);
    plugin_index = 0;
    plugin_found = false;

    /* Ensure the plugin list ain't NULL */
    if(!(*Plugin))
    {
        AFB_API_ERROR(api, "Plugins list is NULL.");
        afb_req_fail(req, NULL, "Plugins list is NULL.");
        return;
    }

    /* Ensure the plugin list ain't empty */
    if(!(*Plugin)->size)
    {
        AFB_REQ_ERROR(req, "Plugin list is empty.");
        afb_req_fail(req, NULL, "Plugins list is empty.");
        return;
    }

    /* MEMORY PLUGIN CASE */
    if (json_object_object_get_ex(arg, "mem", &args))
    {
        /* Retrieve the type of the configuration arguments */
        args_type = json_object_get_type(args);

        /* Let's check if the memory plugin has been initialized */
        for(int i = 0 ; i != (*Plugin)->size ; i++)
        {
            if(!strncmp( (*Plugin)->plugin_callback[i].name, "mem", (*Max_size)((size_t) 3, strlen((*Plugin)->plugin_callback[i].name))))
            {
                /* Store the cpu plugin index */
                plugin_index = i;
                plugin_found = true;
                break;
            }
        }

        /* If the plugin has not been found */
        if(!plugin_found)
        {
            AFB_API_ERROR(api, "No plugin memory initialized.");
            afb_req_fail(req, NULL, "No plugin memory initialized.");
            return;
        }

        /* Open the shared memory plugin library */
        handle = dlopen(MEM_PATH, RTLD_NOW | RTLD_GLOBAL);

        /* If it fails */
        if(!handle)
        {
            AFB_API_ERROR(api, "dlerror dlopen mem %s.", dlerror());
            afb_req_fail(req, NULL, "dlerror dlopen mem.");
            return;
        }

        /* Retrieve the memory module register function */
        module_register = (module_register_t) dlsym(handle, "module_register");

        /* If it fails */
        if(!module_register)
        {
            AFB_API_ERROR(api, "dlerror module register : %s.", dlerror());
            afb_req_fail(req, NULL, "dlerror module register.");
            return;
        }

        /* Execute the module register and handle whether it fails */
        if((*module_register)(plugin_index))
        {
            AFB_API_ERROR(api, "Failed to load callbacks.");
            afb_req_fail(req, NULL, "Failed to load plugin callbacks.");
            return;
        }

        /* Initialize the memory plugin and handle whether it fails */
        if((*(*Plugin)->plugin_callback[plugin_index].init)())
        {
            AFB_API_ERROR(api, "Failed to initialize the memory plugin.");
            afb_req_fail(req, NULL, "Failed to initialize the memory plugin.");
            return;
        }

    }

    /* CPU PLUGIN CASE */
    if (json_object_object_get_ex(arg, "cpu", &args))
    {
        /* Retrieve the type of the configuration arguments */
        args_type = json_object_get_type(args);

        /* Let's check if the cpu plugin has been initialized */
        for(int i = 0 ; i != (*Plugin)->size ; i++)
        {
            if(!strncmp( (*Plugin)->plugin_callback[i].name, "cpu", (*Max_size)((size_t) 3, strlen((*Plugin)->plugin_callback[i].name))))
            {
                /* Store the cpu plugin index */
                plugin_index = i;
                plugin_found = true;
                break;
            }
        }

        /* If the plugin has not been found */
        if(!plugin_found)
        {
            AFB_API_ERROR(api, "No plugin cpu initialized.");
            afb_req_fail(req, NULL, "No plugin cpu initialized.");
            return;
        }

        /* Open the shared cpu plugin library */
        handle = dlopen(CPU_PATH, RTLD_NOW | RTLD_GLOBAL);

        /* If it fails */
        if(!handle)
        {
            AFB_API_ERROR(api, "dlerror dlopen cpu %s.", dlerror());
            afb_req_fail(req, NULL, "dlerror dlopen cpu.");
            return;
        }

        /* Retrieve the cpu module register function */
        module_register = (module_register_t) dlsym(handle, "module_register");

        /* If it fails */
        if(!module_register)
        {
            AFB_API_ERROR(api, "dlerror module register : %s.", dlerror());
            afb_req_fail(req, NULL, "dlerror module register.");
            return;
        }

        /* Execute the module register and handle whether it fails */
        if((*module_register)(plugin_index))
        {
            AFB_API_ERROR(api, "Failed to load callbacks.");
            afb_req_fail(req, NULL, "Failed to load plugin callbacks.");
            return;
        }

        /* Initialize the cpu plugin and handle whether it fails */
        if((*(*Plugin)->plugin_callback[plugin_index].init)())
        {
            AFB_API_ERROR(api, "Failed to initialize the cpu plugin.");
            afb_req_fail(req, NULL, "Failed to initialize the cpu plugin.");
            return;
        }

        /* If the argument has not the good type */
        if(args_type != json_type_string)
        {
            AFB_API_ERROR(api, "Invalid config type in %s.", json_object_get_string(args));
            afb_req_fail_f(req, "ERROR", "Invalid config type in %s.", json_object_get_string(args));
            return;
        }

        /* Retrieve the string contained by the j-son object */
        config_label = (char *) json_object_get_string(args);

        /* MEAN PER CPU CONFIGURATION CASE */
        /* Do the mean of each cpus */
        if(!strncmp(config_label, "mean_cpu", (*Max_size)((size_t) 8, strlen(config_label))))
        {
            /* Reset the cpu plugin settings in order to apply the new one we want */
            (*Cpu_settings_reset)();

            /* Handle whether the configuration failed */
            if((*(*Plugin)->plugin_callback[plugin_index].config)("ReportByCpu", "true")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ReportByState", "false")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ReportNumCpu", "false")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ValuesPercentage", "false"))
            {
                AFB_REQ_ERROR(req, "Failed to apply the following configuration : %s.", config_label);
                afb_req_fail(req, NULL, "Failed to apply the configuration.");
                return;
            }

            /* Notify that the configuration succeed  */
            else
            {
                wrap_json_pack(&res_config,
                               "{ss}",
                               "Configuration", config_label);
                json_object_object_add(res, "success", res_config);
            }
        }

        /* MEAN PER STATE CONFIGURATION CASE */
        /* Do the mean of each state of each cpu */
        else if(!strncmp(config_label, "mean_state", (*Max_size)((size_t) 10, strlen(config_label))))
        {
            /* Reset the cpu plugin settings in order to apply the new one we want */
            (*Cpu_settings_reset)();

            /* Handle whether the configuration failed */
            if((*(*Plugin)->plugin_callback[plugin_index].config)("ReportByCpu", "false")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ReportByState", "true")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ReportNumCpu", "false")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ValuesPercentage", "false"))
            {
                AFB_REQ_ERROR(req, "Failed to apply the following configuration : %s.", config_label);
                afb_req_fail(req, NULL, "Failed to apply the configuration.");
                return;
            }

            /* Notify that the configuration succeed  */
            else
            {
                wrap_json_pack(&res_config,
                               "{ss}",
                               "Configuration", config_label);
                json_object_object_add(res, "success", res_config);
            }
        }

        /* MEAN_CPU CONFIGURATION CASE  */
        else if(!strncmp(config_label, "mean", (*Max_size)((size_t) 4, strlen(config_label))))
        {
            /* Reset the cpu plugin settings in order to apply the new one we want */
            (*Cpu_settings_reset)();

            /* Handle whether the configuration failed */
            if((*(*Plugin)->plugin_callback[plugin_index].config)("ReportByCpu", "false")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ReportByState", "false")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ReportNumCpu", "false")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ValuesPercentage", "false"))
            {
                AFB_REQ_ERROR(req, "Failed to apply the following configuration : %s.", config_label);
                afb_req_fail(req, NULL, "Failed to apply the configuration.");
                return;
            }

            /* Notify that the configuration succeed  */
            else
            {
                wrap_json_pack(&res_config,
                               "{ss}",
                               "Configuration", config_label);
                json_object_object_add(res, "success", res_config);
            }
        }

        /* PERCENT PER STATE PER CPU CONFIGURATION CASE */
        /* Do the mean of each cpus */
        else if(!strncmp(config_label, "percent_state_cpu", (*Max_size)((size_t) 17, strlen(config_label))))
        {
            /* Reset the cpu plugin settings in order to apply the new one we want */
            (*Cpu_settings_reset)();

            /* Handle whether the configuration failed */
            if((*(*Plugin)->plugin_callback[plugin_index].config)("ReportByCpu", "true")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ReportByState", "true")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ReportNumCpu", "false")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ValuesPercentage", "true"))
            {
                AFB_REQ_ERROR(req, "Failed to apply the following configuration : %s.", config_label);
                afb_req_fail(req, NULL, "Failed to apply the configuration.");
                return;
            }

            /* Notify that the configuration succeed  */
            else
            {
                wrap_json_pack(&res_config,
                               "{ss}",
                               "Configuration", config_label);
                json_object_object_add(res, "success", res_config);
            }
        }

        /* JIFFIES PER STATE PER CPU CONFIGURATION CASE */
        /* Do the mean of each cpus */
        else if(!strncmp(config_label, "jiffies_state_cpu", (*Max_size)((size_t) 17, strlen(config_label))))
        {
            /* Reset the cpu plugin settings in order to apply the new one we want */
            (*Cpu_settings_reset)();

            /* Handle whether the configuration failed */
            if((*(*Plugin)->plugin_callback[plugin_index].config)("ReportByCpu", "true")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ReportByState", "true")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ReportNumCpu", "false")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ValuesPercentage", "false"))
            {
                AFB_REQ_ERROR(req, "Failed to apply the following configuration : %s.", config_label);
                afb_req_fail(req, NULL, "Failed to apply the configuration.");
                return;
            }

            /* Notify that the configuration succeed  */
            else
            {
                wrap_json_pack(&res_config,
                               "{ss}",
                               "Configuration", config_label);
                json_object_object_add(res, "success", res_config);
            }
        }

        /* NUMBER CONFIGURATION CASE */
        else if (!strncmp(config_label, "number", (*Max_size)((size_t) 5, strlen(config_label))))
        {
            /* Reset the cpu plugin settings in order to apply the new one we want */
            (*Cpu_settings_reset)();

            /* Handle whether the configuration failed */
            if((*(*Plugin)->plugin_callback[plugin_index].config)("ReportByCpu", "false")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ReportByState", "false")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ReportNumCpu", "true")
            || (*(*Plugin)->plugin_callback[plugin_index].config)("ValuesPercentage", "false"))
            {
                AFB_REQ_ERROR(req, "Failed to apply the following configuration : %s.", config_label);
                afb_req_fail(req, NULL, "Failed to apply the configuration.");
                return;
            }

            /* Notify that the configuration succeed  */
            else
            {
                wrap_json_pack(&res_config,
                               "{ss}",
                               "Configuration", config_label);
                json_object_object_add(res, "success", res_config);
            }
        }

        /* If we reach that point it means that the configuration is not valid */
        else
        {
            AFB_REQ_ERROR(req, "The configuration %s is not valid.", config_label);
            afb_req_fail(req, NULL, "The configuration specified is not valid.");
            return;
        }

    /* Notify that the configuration is a success */
    AFB_API_NOTICE(afbBindingV3root, "Verbosity macro at level notice invoked at config cpu invocation");
    afb_req_success(req, res, "Plugin configuration succeed.");
    return;
    }

    /* The plugin ain't known so any configuration has been applied */
    AFB_REQ_ERROR(req, "The specified plugin ain't valid.");
    afb_req_fail(req, NULL, "The specified plugin ain't valid.");
    return;
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            AFB READ
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
static void afb_read(afb_req_t req)
{
    /* Variables definition */
    json_object *res;
    json_object *res_plugin;
    json_object *arg;
    json_object *args;
    json_type args_type;
    char *plugin_label;
    size_t plugin_index;
    bool plugin_found;
    
    /* Variables allocation */
    res = json_object_new_object();
    res_plugin = json_object_new_object();
    args = json_object_new_object();
    arg = json_object_new_object();

    /* Variables initialization */
    arg = afb_req_json(req);
    plugin_found = false;

    /* Ensure the plugin list ain't NULL */
    if(!(*Plugin))
    {
        AFB_REQ_ERROR(req, "Plugin list is NULL.");
        afb_req_fail(req, NULL, "Plugins list is NULL.");
        return;
    }

    /* Ensure the plugin list ain't empty */
    if(!(*Plugin)->size)
    {
        AFB_REQ_ERROR(req, "Plugin list is empty.");
        afb_req_fail(req, NULL, "Plugins list is empty.");
        return;
    }

    /* Invalid key */
    if(!json_object_object_get_ex(arg, "plugin", &args))
    {
        AFB_REQ_ERROR(req, "Invalid request in %s.", json_object_get_string(arg));
		afb_req_fail_f(req, "ERROR", "Invalid request in %s.", json_object_get_string(arg));
		return;
    }

    /* Retrieve the argument type */
    args_type = json_object_get_type(args);
    if(args_type != json_type_string)
    {
        AFB_REQ_ERROR(req, "Invalid arguments in %s.", json_object_get_string(args));
		afb_req_fail_f(req, "ERROR", "Invalid arguments in %s.", json_object_get_string(args));
    }

    plugin_label = (char *) json_object_get_string(args);

    /* PLUGIN CPU CASE */
    if(!strncmp(plugin_label, "cpu", (*Max_size)((size_t) 3, strlen(plugin_label))))
    {
        /* Ensure a plugin named cpu is stored and retrieve its index */
        for(int i = 0 ; i != (*Plugin)->size ; i++)
        {
            if(!strncmp((*Plugin)->plugin_callback[i].name, "cpu", (*Max_size)((size_t) 3, strlen((*Plugin)->plugin_callback[i].name))))
            {
                plugin_index = i;
                plugin_found = true;
            }
        }

        /* If the cpu has not been found */
        if(!plugin_found)
        {
            AFB_REQ_ERROR(req, "The cpu plugin ain't initialized.");
            afb_req_fail(req, NULL, "The cpu plugin ain't initialized.");
            return;
        }

        /* Ensure the cpu plugin has been configured */
        if(!(*Plugin)->plugin_callback[plugin_index].read)
        {
            AFB_REQ_ERROR(req, "The cpu plugin has not been configured.");
            afb_req_fail(req, NULL, "The cpu plugin has not been configured.");
            return;
        }

        /* Call the cpu callbacks read */
        if((*(*Plugin)->plugin_callback[plugin_index].read)(NULL))
        {
            AFB_REQ_ERROR(req, "Failed to execute cpu read callback.");
            afb_req_fail(req, NULL, "Failed to execute cpu read callback.");
            return;
        }

        /* If the metrics list retrieved is empty */
        if(!*Metrics)
        {
            AFB_REQ_WARNING(req, "Metrics list is empty.");
            afb_req_fail(req, NULL, "Metrics list is empty.");
            return;
        }

        /* For each metrics contained in the metrics list */
        for(int i = 0 ; i != (*Metrics)->size ; i++)
        {
            /* The metrics plugin has to be cpu */
            if(!strncmp((*Metrics)->metrics[i].plugin, "cpu", (*Max_size)((size_t) 3, strlen((*Metrics)->metrics[i].plugin))))
            {
                /* If the plugin configuration regards the number of cpu */
                if(!strncmp((*Metrics)->metrics[i].type, "count", (*Max_size)((size_t) 3, strlen((*Metrics)->metrics[i].plugin))))
                {
                    /* Pack the metrics in a j-son */
                    wrap_json_pack(&res_plugin,
                                   "{sf}",
                                   "n°cpu(s)", (*Metrics)->metrics[i].values->gauge);
                    json_object_object_add(res, (*Metrics)->metrics[i].type, res_plugin);
                }

                /* If the plugin configuration regards the percentage of each state of each cpu */
                if(!strncmp((*Metrics)->metrics[i].type, "percent", (*Max_size)((size_t) 3, strlen((*Metrics)->metrics[i].plugin))))
                {
                    /* Pack the metrics in a j-son */
                    wrap_json_pack(&res_plugin,
                                   "{sf}",
                                   (*Metrics)->metrics[i].type_instance, (*Metrics)->metrics[i].values->gauge);
                    json_object_object_add(res, strcat(strcat((*Metrics)->metrics[i].type_instance, " "), (*Metrics)->metrics[i].plugin_instance), res_plugin);
                }

                /* If the plugin configuration regards each state of each cpu in jiffies */
                if(!strncmp((*Metrics)->metrics[i].type, "cpu", (*Max_size)((size_t) 3, strlen((*Metrics)->metrics[i].plugin))))
                {
                    /* Pack the metrics in a j-son */
                    wrap_json_pack(&res_plugin,
                                   "{sI}",
                                   (*Metrics)->metrics[i].type_instance, (*Metrics)->metrics[i].values->derive);
                    json_object_object_add(res, strcat(strcat((*Metrics)->metrics[i].type_instance, " "), (*Metrics)->metrics[i].plugin_instance), res_plugin);
                }
            }
        }

        /* Reset the metrics list  and notify the read is a success */
        (*Metrics_deinit)();
        AFB_API_NOTICE(afbBindingV3root, "Verbosity macro at level notice invoked at read cpu invocation");
        afb_req_success(req, res, "Plugin cpu succesfully read");
        return;
    }

    /* TEST TO KNOW WHAT ARE THE METRIC FIELDS OF A NEW PLUGIN */
    else
    {
        if(*Metrics)
        {
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
            afb_req_success(req, NULL, "Plugin succesfully read");
            (*Metrics_deinit)();
    }
    else
    {
        AFB_API_NOTICE(afbBindingV3root, "Metrics failed to load");
        afb_req_fail(req, NULL, "Fails to read the plugin");
    }
    return;
    }
}

static void afb_reset(afb_req_t req)
{
    /* Variables definition */
    afb_api_t api;
    json_object *res;
    json_object *res_reset;
    json_object *arg;
    json_object *args;
    json_object *current_plugin;
    json_type args_type;
    int reset_number;
    char *plugin_label;
    int reset_count;
    size_t plugin_index;
    bool plugin_found;
    
    /* Variables allocation */
    res = json_object_new_object();
    res_reset = json_object_new_object();
    args = json_object_new_object();
    arg = json_object_new_object();
    current_plugin = json_object_new_object();

    /* Variables initialization */
    api = afb_req_get_api(req);
    arg = afb_req_json(req);
    plugin_index = 0;
    reset_count = 0;

    if(!(*Plugin))
    {
        AFB_API_ERROR(api, "Plugin list is already empty.");
        afb_req_fail(req, NULL, "Plugin list is already empty.");
        return;
    }

    /* Invalid key */
	if(!json_object_object_get_ex(arg, "plugin", &args)){
        AFB_REQ_ERROR(req, "Invalid request in %s.", json_object_get_string(arg));
		afb_req_fail_f(req, "ERROR", "Invalid request in %s.", json_object_get_string(arg));
		return;
	}

    /* Retrieve the type of the arguments and store the number of plugin desired  */
    args_type = json_object_get_type(args);
    switch(args_type)
    {
        case json_type_string:
        {
            reset_number = 1;
			break;
        }

        case json_type_array:
        {
            reset_number = json_object_array_length(args);
			break;
        }

        default:
        {
            AFB_REQ_ERROR(req, "Invalid plugin type in %s.", json_object_get_string(args));
            afb_req_fail_f(req, "ERROR", "Invalid plugin type in %s.", json_object_get_string(args));
            return;
        }
    }

    /* For each one of the previous plugin */
    for(int i = 0 ; i != reset_number ; i++)
    {
        /* Set the boolean which mean the current plugin has been found */
        plugin_found = false;

        /* Retrieve the current plugin */
        if(args_type == json_type_array)
        {
            current_plugin = json_object_array_get_idx(args, i);
        }

        else
        {
            current_plugin = args;
        }

        /* If the type ain't the one we expected we noticed it but we continue the process */
        if(!json_object_is_type(current_plugin, json_type_string))
        {
			AFB_REQ_ERROR(req, "Current plugin is not correct ('%s').", json_object_get_string(current_plugin));
			continue;
		}

        /* Retrieve the string in the current j-son object */
        plugin_label = (char *) json_object_get_string(current_plugin);

        /* If the current plugin is the cpu one */
        if(!strncmp(plugin_label, "cpu", (*Max_size)((size_t) 3, strlen(plugin_label)))
        || !strncmp(plugin_label, "mem", (*Max_size)((size_t) 3, strlen(plugin_label))))
        {
            /* let's check if there is a pugin named cpu stored */
            for(int i = 0 ; i != (*Plugin)->size ; i++)
            {
                if(!strncmp(plugin_label, (*Plugin)->plugin_callback[i].name, (*Max_size)(strlen((*Plugin)->plugin_callback[i].name), strlen(plugin_label))))
                {
                    plugin_index = i;
                    plugin_found = true;
                }
            }

            /* If the desired plugin is not found in the list we noticed it but we continue the process*/
            if(!plugin_found)
            {
                AFB_REQ_ERROR(req, "Plugin %s is not initialized yet.", plugin_label);
                continue;
            }
            
            /* Delete the plugin and update the plugin list */
            if((*Plugin_deinit)(plugin_index))
            {
                AFB_REQ_ERROR(req, "Failed to remove the plugin %s.", plugin_label);
                continue;
            }

            /* Notify that we delete a plugin */
            else
            {
                wrap_json_pack(&res_reset,
                           "{ss}",
                           "plugin", plugin_label);
                json_object_object_add(res, "Deleted", res_reset);
                reset_count ++;
                continue;
            }
        }

        /* If we reach that point, it means that the plugin desired ain't known */
        AFB_REQ_WARNING(req, "Plugin %s is not known.", plugin_label);
    }

    /* If none of the previous plugins were known */
    if(!reset_count)
    {
        AFB_REQ_ERROR(req, "None of the plugin(s) specified had been deleted.");
        afb_req_fail(req, NULL, "None of the plugin(s) specified had been deleted.");
        return;
    }

    /* Notify that the initialization is a success */
    AFB_API_NOTICE(afbBindingV3root, "Verbosity macro at level notice invoked at reset invocation");
    afb_req_success(req, res, "Plugin reset succeed.");
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

    Cpu_settings_reset = (cpu_settings_reset_t)dlsym(handle_plugin, "cpu_settings_reset");
    if(!Cpu_settings_reset)
    {
        AFB_API_ERROR(api, "dlerror cpu settings reset symbol : %s", dlerror());
        return -1;
    }

    Mem_settings_reset = (cpu_settings_reset_t)dlsym(handle_plugin, "mem_settings_reset");
    if(!Mem_settings_reset)
    {
        AFB_API_ERROR(api, "dlerror mem settings reset symbol : %s", dlerror());
        return -1;
    }

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
        AFB_API_ERROR(api, "dlerror Metrics : %s", dlerror());
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
   || afb_api_add_verb(api, "read", "Read the metrics plugin previously configured", afb_read, NULL, NULL, AFB_SESSION_NONE, 0)
   || afb_api_add_verb(api, "reset", "Delete plugins stored in the list", afb_reset, NULL, NULL, AFB_SESSION_NONE, 0))
   {
        AFB_API_ERROR(api, "Verb initialization for API collector failed");
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