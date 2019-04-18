#ifndef API_H
#define API_H

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            DEFINE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#define _GNU_SOURCE

#define AFB_BINDING_VERSION 3

#define PLUGIN_PATH "../build/src/collectd/./plugin.so"

#define CPU_PATH "../build/src/collectd/./cpu.so"

#define MEM_PATH "../build/src/collectd/./mem.so"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            INCLUDES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
#include <dlfcn.h>
#include <json-c/json.h>
#include </opt/AGL/include/wrap-json.h>
#include </opt/AGL/include/afb/afb-binding.h>

#include "../collectd/collectd_glue.h"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            API FUNCTIONS PROTOTYPE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            INITIALIZATION CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

int api_plugin_init(plugin_t *plugin_list, json_object *arg);
int api_plugin_init_cpu(plugin_t *plugin_list);

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            CONFIGURATION CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
int api_plugin_config(plugin_t *plugin_list, json_object *arg);
int api_plugin_config_cpu(plugin_t *plugin_list, json_object *args);
int api_cpu_config_mean(plugin_t *plugin_list, int plugin_index);
int api_cpu_config_mean_cpu(plugin_t *plugin_list, int plugin_index);
int api_cpu_config_mean_state(plugin_t *plugin_list, int plugin_index);
int api_cpu_config_percent_state_cpu(plugin_t *plugin_list, int plugin_index);
int api_cpu_config_jiffies_state_cpu(plugin_t *plugin_list, int plugin_index);
int api_cpu_config_number_cpu(plugin_t *plugin_list, int plugin_index);

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            READ CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
json_object *api_plugin_read(plugin_t *plugin_list, json_object *arg);
json_object *api_cpu_read(plugin_t *plugin_list);

#endif
