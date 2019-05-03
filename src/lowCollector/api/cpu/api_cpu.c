/*
 * Copyright (C) 2016-2019 "IoT.bzh"
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

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            INCLUDES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#include <dlfcn.h>
#include "api_cpu.h"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            DEFINE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#define CPU_PATH "../build/src/collectd/./cpu.so"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            INITIALIZATION CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *api_cpu_init(userdata_t *userdata)
{
  /* Variable definition */
  plugin_list_t **plugin_list;
  module_register_t module_register;
  index_plugin_label_t Index_plugin_label;

  /* Open the cpu library only if it ain't already open*/
  if(!userdata->handle_cpu)
  {
    userdata->handle_cpu = dlopen(CPU_PATH, RTLD_NOW || RTLD_GLOBAL);
    if(!userdata->handle_cpu)
      return json_object_new_string(dlerror());
  }

  /* Retrieve the global variable plugin list from the collectd glue library */
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, PLUGIN_LIST_CHAR);
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Retrieve the index plugin label function */
  Index_plugin_label = (index_plugin_label_t)dlsym(userdata->handle_collectd, INDEX_PLUGIN_LABEL_CHAR);
  if(!Index_plugin_label)
    return json_object_new_string(dlerror());

  /* Load the module register symbol */
  module_register = (module_register_t)dlsym(userdata->handle_cpu, MODULE_REGISTER_CHAR);
  if(!module_register)
    return json_object_new_string(dlerror());

  /* First let's check if a plugin with the cpu name already exists */
    if((*Index_plugin_label)(*plugin_list, CPU_CHAR) != -1)
      return json_object_new_string(ERR_PLUGIN_IS_STORED_CHAR);

  /* Call the module register function to create the plugin and store its callbacks */
  (module_register)();

  return json_object_new_string(SUCCESS_INIT_CHAR);
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            CONFIGURATION CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *api_cpu_config_mean(userdata_t *userdata, int plugin_index)
{
  /* Variables definition */
  plugin_list_t **plugin_list;
  metrics_list_t **metrics_list;
  metrics_deinit_t Metrics_deinit;

  /* Retrieve the global variable plugin list */
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, PLUGIN_LIST_CHAR);
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Retrieve the global variable metrics list */
  metrics_list = (metrics_list_t **)dlsym(userdata->handle_collectd, METRICS_LIST_CHAR);
  if(!metrics_list)
    return json_object_new_string(dlerror());

  /* Retrieve the metrics deinit function */
  Metrics_deinit = (metrics_deinit_t)dlsym(userdata->handle_collectd, METRICS_DEINIT_CHAR);
  if(!Metrics_deinit)
    return json_object_new_string(dlerror());

  if((*plugin_list)->plugin[plugin_index].config("ReportByCpu", "false")
  || (*plugin_list)->plugin[plugin_index].config("ReportByState", "false")
  || (*plugin_list)->plugin[plugin_index].config("ReportNumCpu", "false")
  || (*plugin_list)->plugin[plugin_index].config("ValuesPercentage", "false"))
    return json_object_new_string(ERR_CONFIG_CHAR);

  /* Launch the cpu read callack in order to initialise the collection */
  (*plugin_list)->plugin[plugin_index].read(NULL);

  /* If the metrics list is fill with metrics, we reset it */
  if((*metrics_list) && (*metrics_list)->size)
    (*Metrics_deinit)();

  return json_object_new_string(SUCCESS_CONFIG_CHAR);
}

json_object *api_cpu_config_mean_cpu(userdata_t *userdata, int plugin_index)
{
  /* Variables definition */
  plugin_list_t **plugin_list;
  metrics_list_t **metrics_list;
  metrics_deinit_t Metrics_deinit;

  /* Retrieve the global variable plugin list */
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, PLUGIN_LIST_CHAR);
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Retrieve the global variable metrics list */
  metrics_list = (metrics_list_t **)dlsym(userdata->handle_collectd, METRICS_LIST_CHAR);
  if(!metrics_list)
    return json_object_new_string(dlerror());

  /* Retrieve the metrics deinit function */
  Metrics_deinit = (metrics_deinit_t)dlsym(userdata->handle_collectd, METRICS_DEINIT_CHAR);
  if(!Metrics_deinit)
    return json_object_new_string(dlerror());

  if((*plugin_list)->plugin[plugin_index].config("ReportByCpu", "true")
  || (*plugin_list)->plugin[plugin_index].config("ReportByState", "false")
  || (*plugin_list)->plugin[plugin_index].config("ReportNumCpu", "false")
  || (*plugin_list)->plugin[plugin_index].config("ValuesPercentage", "false"))
    return json_object_new_string(ERR_CONFIG_CHAR);

  /* Launch the cpu read callack in order to initialise the collection */
  (*plugin_list)->plugin[plugin_index].read(NULL);

  /* If the metrics list is fill with metrics, we reset it */
  if((*metrics_list) && (*metrics_list)->size)
    (*Metrics_deinit)();

  return json_object_new_string(SUCCESS_CONFIG_CHAR);
}

json_object *api_cpu_config_mean_state(userdata_t *userdata, int plugin_index)
{
  /* Variables definition */
  plugin_list_t **plugin_list;
  metrics_list_t **metrics_list;
  metrics_deinit_t Metrics_deinit;

  /* Retrieve the global variable plugin list */
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, PLUGIN_LIST_CHAR);
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Retrieve the global variable metrics list */
  metrics_list = (metrics_list_t **)dlsym(userdata->handle_collectd, METRICS_LIST_CHAR);
  if(!metrics_list)
    return json_object_new_string(dlerror());

  /* Retrieve the metrics deinit function */
  Metrics_deinit = (metrics_deinit_t)dlsym(userdata->handle_collectd, METRICS_DEINIT_CHAR);
  if(!Metrics_deinit)
    return json_object_new_string(dlerror());

  if((*plugin_list)->plugin[plugin_index].config("ReportByCpu", "false")
  || (*plugin_list)->plugin[plugin_index].config("ReportByState", "true")
  || (*plugin_list)->plugin[plugin_index].config("ReportNumCpu", "false")
  || (*plugin_list)->plugin[plugin_index].config("ValuesPercentage", "false"))
    return json_object_new_string(ERR_CONFIG_CHAR);

  /* Launch the cpu read callack in order to initialise the collection */
  (*plugin_list)->plugin[plugin_index].read(NULL);

  /* If the metrics list is fill with metrics, we reset it */
  if((*metrics_list) && (*metrics_list)->size)
    (*Metrics_deinit)();

  return json_object_new_string(SUCCESS_CONFIG_CHAR);
}

json_object *api_cpu_config_percent_state_cpu(userdata_t *userdata, int plugin_index)
{
  /* Variables definition */
  plugin_list_t **plugin_list;
  metrics_list_t **metrics_list;
  metrics_deinit_t Metrics_deinit;

  /* Retrieve the global variable plugin list */
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, PLUGIN_LIST_CHAR);
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Retrieve the global variable metrics list */
  metrics_list = (metrics_list_t **)dlsym(userdata->handle_collectd, METRICS_LIST_CHAR);
  if(!metrics_list)
    return json_object_new_string(dlerror());

  /* Retrieve the metrics deinit function */
  Metrics_deinit = (metrics_deinit_t)dlsym(userdata->handle_collectd, METRICS_DEINIT_CHAR);
  if(!Metrics_deinit)
    return json_object_new_string(dlerror());

  if((*plugin_list)->plugin[plugin_index].config("ReportByCpu", "true")
  || (*plugin_list)->plugin[plugin_index].config("ReportByState", "true")
  || (*plugin_list)->plugin[plugin_index].config("ReportNumCpu", "false")
  || (*plugin_list)->plugin[plugin_index].config("ValuesPercentage", "true"))
    return json_object_new_string(ERR_CONFIG_CHAR);

  /* Launch the cpu read callack in order to initialise the collection */
  (*plugin_list)->plugin[plugin_index].read(NULL);

  /* If the metrics list is fill with metrics, we reset it */
  if((*metrics_list) && (*metrics_list)->size)
    (*Metrics_deinit)();

  return json_object_new_string(SUCCESS_CONFIG_CHAR);
}

json_object *api_cpu_config_jiffies_state_cpu(userdata_t *userdata, int plugin_index)
{
  /* Variables definition */
  plugin_list_t **plugin_list;
  metrics_list_t **metrics_list;
  metrics_deinit_t Metrics_deinit;

  /* Retrieve the global variable plugin list */
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, PLUGIN_LIST_CHAR);
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Retrieve the global variable metrics list */
  metrics_list = (metrics_list_t **)dlsym(userdata->handle_collectd, METRICS_LIST_CHAR);
  if(!metrics_list)
    return json_object_new_string(dlerror());

  /* Retrieve the metrics deinit function */
  Metrics_deinit = (metrics_deinit_t)dlsym(userdata->handle_collectd, METRICS_DEINIT_CHAR);
  if(!Metrics_deinit)
    return json_object_new_string(dlerror());

  if((*plugin_list)->plugin[plugin_index].config("ReportByCpu", "true")
  || (*plugin_list)->plugin[plugin_index].config("ReportByState", "true")
  || (*plugin_list)->plugin[plugin_index].config("ReportNumCpu", "false")
  || (*plugin_list)->plugin[plugin_index].config("ValuesPercentage", "false"))
    return json_object_new_string(ERR_CONFIG_CHAR);

  /* Launch the cpu read callack in order to initialise the collection */
  (*plugin_list)->plugin[plugin_index].read(NULL);

  /* If the metrics list is fill with metrics, we reset it */
  if((*metrics_list) && (*metrics_list)->size)
    (*Metrics_deinit)();

  return json_object_new_string(SUCCESS_CONFIG_CHAR);
}

json_object *api_cpu_config_number_cpu(userdata_t *userdata, int plugin_index)
{
  /* Variables definition */
  plugin_list_t **plugin_list;
  metrics_list_t **metrics_list;
  metrics_deinit_t Metrics_deinit;

  /* Retrieve the global variable plugin list */
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, PLUGIN_LIST_CHAR);
  if(!plugin_list)
    return json_object_new_string(dlerror());

  metrics_list = (metrics_list_t **)dlsym(userdata->handle_collectd, METRICS_LIST_CHAR);
  if(!metrics_list)
    return json_object_new_string(dlerror());

  Metrics_deinit = (metrics_deinit_t)dlsym(userdata->handle_collectd, METRICS_DEINIT_CHAR);
  if(!Metrics_deinit)
    return json_object_new_string(dlerror());

  if((*plugin_list)->plugin[plugin_index].config("ReportByCpu", "false")
  || (*plugin_list)->plugin[plugin_index].config("ReportByState", "false")
  || (*plugin_list)->plugin[plugin_index].config("ReportNumCpu", "true")
  || (*plugin_list)->plugin[plugin_index].config("ValuesPercentage", "false"))
    return json_object_new_string(ERR_CONFIG_CHAR);

  /* Launch the cpu read callack in order to initialise the collection */
  (*plugin_list)->plugin[plugin_index].read(NULL);

  /* If the metrics list is fill with metrics, we reset it */
  if((*metrics_list) && (*metrics_list)->size)
    (*Metrics_deinit)();

  return json_object_new_string(SUCCESS_CONFIG_CHAR);
}

json_object *api_cpu_config(userdata_t *userdata, json_object *args)
{
  /* Variable definition */
  int plugin_index;
  json_type args_type;
  max_size_t Max_size;
  index_plugin_label_t Index_plugin_label;
  plugin_list_t **plugin_list;

  /* Ensure the cpu library is open */
  if(!userdata->handle_cpu)
    return json_object_new_string(ERR_LIB_CHAR);

  /* Retrieve the max_size function */
  Max_size = (max_size_t)dlsym(userdata->handle_collectd, MAX_SIZE_CHAR);
  if(!Max_size)
    return json_object_new_string(dlerror());

  /* Retrieve the index plugin label function */
  Index_plugin_label = (index_plugin_label_t)dlsym(userdata->handle_collectd, INDEX_PLUGIN_LABEL_CHAR);
  if(!Index_plugin_label)
    return json_object_new_string(dlerror());

  /* Retrieve the plugin list variable */
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, PLUGIN_LIST_CHAR);
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Ensure the plugin list ain't NULL */
  if(!(*plugin_list))
    return json_object_new_string(ERR_PLUGIN_NULL_CHAR);

  /* First, let's ensure the list has a cpu plugin initialize */
  plugin_index = (*Index_plugin_label)(*plugin_list, CPU_CHAR);
  if(plugin_index == -1)
    return json_object_new_string(ERR_PLUGIN_STORED_CHAR);

  /* Retrieve the type of the configuration and ensure it's a good one */
  args_type = json_object_get_type(args);
  if(args_type != json_type_string)
    return json_object_new_string(ERR_ARG_CHAR);

  /* Initialize the cpu plugin */
   if((*plugin_list)->plugin[plugin_index].init())
     return json_object_new_string(ERR_INIT_CHAR);

  /* Mean configuration case */
  if(!strncmp(json_object_get_string(args), CPU_MEAN_CHAR, (*Max_size)(strlen(CPU_MEAN_CHAR), strlen(json_object_get_string(args)))))
    return api_cpu_config_mean(userdata, plugin_index);

  /* Mean cpu configuration case */
  else if(!strncmp(json_object_get_string(args), CPU_MEAN_CPU_CHAR, (*Max_size)(strlen(CPU_MEAN_CPU_CHAR), strlen(json_object_get_string(args)))))
    return api_cpu_config_mean_cpu(userdata, plugin_index);

  /* Mean state configuration case */
  else if(!strncmp(json_object_get_string(args), CPU_MEAN_STATE_CHAR, (*Max_size)(strlen(CPU_MEAN_STATE_CHAR), strlen(json_object_get_string(args)))))
    return api_cpu_config_mean_state(userdata, plugin_index);

  /* Percent state cpu configuration case */
  else if(!strncmp(json_object_get_string(args), CPU_PERCENT_STATE_CPU_CHAR, (*Max_size)(strlen(CPU_PERCENT_STATE_CPU_CHAR), strlen(json_object_get_string(args)))))
    return api_cpu_config_percent_state_cpu(userdata, plugin_index);

  /* Jiffies state cpu configuration case */
  else if(!strncmp(json_object_get_string(args), CPU_JIFFIES_STATE_CPU_CHAR, (*Max_size)(strlen(CPU_JIFFIES_STATE_CPU_CHAR), strlen(json_object_get_string(args)))))
    return api_cpu_config_jiffies_state_cpu(userdata, plugin_index);

  /* Number configuration case */
  else if(!strncmp(json_object_get_string(args), CPU_NUMBER_CHAR, (*Max_size)(strlen(CPU_NUMBER_CHAR), strlen(json_object_get_string(args)))))
    return api_cpu_config_number_cpu(userdata, plugin_index);

  /* Unknown configuration */
  else
    return json_object_new_string(ERR_CONFIG_UNKNOWN_CHAR);
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            READ CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *api_cpu_read(userdata_t *userdata)
{
  /* Variables definition */
  int plugin_index;
  metrics_list_t **metrics_list;
  plugin_list_t **plugin_list;
  metrics_deinit_t Metrics_deinit;
  max_size_t Max_size;
  index_plugin_label_t Index_plugin_label;
  json_object *res;

  /* Ensure the cpu library is opened */
  if(!userdata->handle_cpu)
    return json_object_new_string(ERR_LIB_CHAR);

  /* Variable allocation */
  res = json_object_new_object();

  /* Retrieve the global variable plugin list */
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, PLUGIN_LIST_CHAR);
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Retrieve the global variable metrics list */
  metrics_list = (metrics_list_t **)dlsym(userdata->handle_collectd, METRICS_LIST_CHAR);
  if(!metrics_list)
    return json_object_new_string(dlerror());

  /* Retrieve the metrics deinit function */
  Metrics_deinit = (metrics_deinit_t)dlsym(userdata->handle_collectd, METRICS_DEINIT_CHAR);
  if(!Metrics_deinit)
    return json_object_new_string(dlerror());

  /* Retrieve the max size function */
  Max_size = (max_size_t)dlsym(userdata->handle_collectd, MAX_SIZE_CHAR);
  if(!Max_size)
    return json_object_new_string(dlerror());

  /* Retrieve the index plugin label function */
  Index_plugin_label = (index_plugin_label_t)dlsym(userdata->handle_collectd, INDEX_PLUGIN_LABEL_CHAR);
  if(!Index_plugin_label)
    return json_object_new_string(dlerror());

  /* Ensure the plugin list ain't NULL */
  if(!*plugin_list)
    return json_object_new_string(ERR_PLUGIN_NULL_CHAR);

  /* Ensure a plugin named cpu is stored and retrieve its index */
  plugin_index = (*Index_plugin_label)(*plugin_list, CPU_CHAR);
  if(plugin_index == -1)
    return json_object_new_string(ERR_PLUGIN_STORED_CHAR);

  /* Call the cpu callbacks read */
  if((*plugin_list)->plugin[plugin_index].read(NULL))
    return json_object_new_string(ERR_READ_CHAR);

  res = write_json((*metrics_list));

  /* If the metrics has been filled with values, we reset it */
  if((*metrics_list)->metrics)
    (*Metrics_deinit)();

  return res;
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            RESET CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *api_cpu_reset(userdata_t *userdata)
{
  /* Variables definition */
  plugin_list_t **plugin_list;
  plugin_deinit_t Plugin_deinit;
  int plugin_index;
  index_plugin_label_t Index_plugin_label;

  /* Retrieve the global plugin list variable */
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, PLUGIN_LIST_CHAR);
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Retrieve the plugin deinit function */
  Plugin_deinit = (plugin_deinit_t)dlsym(userdata->handle_collectd, PLUGIN_DEINIT_CHAR);
  if(!Plugin_deinit)
    return json_object_new_string(dlerror());

  /* Retrieve the index plugin label function */
  Index_plugin_label = (index_plugin_label_t)dlsym(userdata->handle_collectd, INDEX_PLUGIN_LABEL_CHAR);
  if(!Index_plugin_label)
    return json_object_new_string(dlerror());

  /* Ensure the cpu library is opened */
  if(!userdata->handle_cpu)
    return json_object_new_string(ERR_LIB_CHAR);

  /* Ensure the plugin list ain't NULL */
  if(!*plugin_list)
    return json_object_new_string(ERR_PLUGIN_NULL_CHAR);

  /* Ensure the cpu plugin is registered in the plugin list */
  if((*Index_plugin_label)(*plugin_list, CPU_CHAR) == -1)
    return json_object_new_string(ERR_PLUGIN_STORED_CHAR);

  /* Retrieve the index of the cpu plugin */
  plugin_index = (*Index_plugin_label)(*plugin_list, CPU_CHAR);

  /* Delete the cpu plugin from the list */
  if((*Plugin_deinit)(plugin_index))
    return json_object_new_string(ERR_RESET_CHAR);

  /* Close the cpu library */
  dlclose(userdata->handle_cpu);
  userdata->handle_cpu = NULL;
  return json_object_new_string(SUCCESS_RESET_CHAR);
}
