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
#include "api_memory.h"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            DEFINE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#define MEMORY_PATH "../build/src/collectd/./memory.so"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            INITIALIZATION CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *api_memory_init(userdata_t *userdata)
{
  /* Variable definition */
  plugin_list_t **plugin_list;
  module_register_t module_register;
  index_plugin_label_t Index_plugin_label;

  /* Open the memory library only if it ain't already open*/
  if(!userdata->handle_memory)
  {
    userdata->handle_memory = dlopen(MEMORY_PATH, RTLD_NOW || RTLD_GLOBAL);
    if(!userdata->handle_memory)
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
  module_register = (module_register_t)dlsym(userdata->handle_memory, MODULE_REGISTER_CHAR);
  if(!module_register)
    return json_object_new_string(dlerror());

  /* First let's check if a plugin with the memory name already exists */
    if((*Index_plugin_label)(*plugin_list, MEMORY_CHAR) != -1)
      return json_object_new_string(ERR_PLUGIN_STORED_CHAR);

  /* Call the module register function to create the plugin and store its callbacks */
  (module_register)();

  return json_object_new_string(SUCCESS_INIT_CHAR);
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            CONFIGURATION CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *api_mem_config_absolute(userdata_t *userdata, int plugin_index)
{
  /* Variables definition */
  plugin_list_t **plugin_list;
  oconfig_item_t *config;

    /* Variable allocation */
  config = (oconfig_item_t *)malloc(sizeof(oconfig_item_t));
  config->children = (oconfig_item_t*)malloc(2*sizeof(oconfig_item_t));
  config->children[0].values = (oconfig_value_t *)malloc(sizeof(oconfig_value_t));
  config->children[1].values = (oconfig_value_t *)malloc(sizeof(oconfig_value_t));

  /* Set the configuration */
  config->children_num = 2;

  config->children[0].key = "ValuesAbsolute";
  config->children[0].values_num = 1;
  config->children[0].values->type = OCONFIG_TYPE_BOOLEAN;
  config->children[0].values->value.boolean = true;

  config->children[1].key = "ValuesPercentage";
  config->children[1].values_num = 1;
  config->children[1].values->type = OCONFIG_TYPE_BOOLEAN;
  config->children[1].values->value.boolean = false;


  /* Retrieve the global variable plugin list */
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, PLUGIN_LIST_CHAR);
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Memory configuration */
  if((*plugin_list)->plugin[plugin_index].complex_config(config))
    return json_object_new_string(ERR_CONFIG_CHAR);

  return json_object_new_string(SUCCESS_CONFIG_CHAR);
}

json_object *api_mem_config_percent(userdata_t *userdata, int plugin_index)
{
  /* Variables definition */
  plugin_list_t **plugin_list;
  oconfig_item_t *config;

  /* Variable allocation */
  config = (oconfig_item_t *)malloc(sizeof(oconfig_item_t));
  config->children = (oconfig_item_t*)malloc(2*sizeof(oconfig_item_t));
  config->children[0].values = (oconfig_value_t *)malloc(sizeof(oconfig_value_t));
  config->children[1].values = (oconfig_value_t *)malloc(sizeof(oconfig_value_t));

  /* Set the configuration */
  config->children_num = 2;

  config->children[0].key = "ValuesAbsolute";
  config->children[0].values_num = 1;
  config->children[0].values->type = OCONFIG_TYPE_BOOLEAN;
  config->children[0].values->value.boolean = false;

  config->children[1].key = "ValuesPercentage";
  config->children[1].values_num = 1;
  config->children[1].values->type = OCONFIG_TYPE_BOOLEAN;
  config->children[1].values->value.boolean = true;

  /* Retrieve the global variable plugin list */
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, PLUGIN_LIST_CHAR);
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Memory configuration */
  if((*plugin_list)->plugin[plugin_index].complex_config(config))
    return json_object_new_string(ERR_CONFIG_CHAR);

  return json_object_new_string(SUCCESS_CONFIG_CHAR);
}

json_object *api_mem_config_all(userdata_t *userdata, int plugin_index)
{
  /* Variables definition */
  plugin_list_t **plugin_list;
  oconfig_item_t *config;

  /* Variable allocation */
  config = (oconfig_item_t *)malloc(sizeof(oconfig_item_t));
  config->children = (oconfig_item_t*)malloc(2*sizeof(oconfig_item_t));
  config->children[0].values = (oconfig_value_t *)malloc(sizeof(oconfig_value_t));
  config->children[1].values = (oconfig_value_t *)malloc(sizeof(oconfig_value_t));

  /* Set the configuration */
  config->children_num = 2;

  config->children[0].key = "ValuesAbsolute";
  config->children[0].values_num = 1;
  config->children[0].values->type = OCONFIG_TYPE_BOOLEAN;
  config->children[0].values->value.boolean = true;

  config->children[1].key = "ValuesPercentage";
  config->children[1].values_num = 1;
  config->children[1].values->type = OCONFIG_TYPE_BOOLEAN;
  config->children[1].values->value.boolean = true;

  /* Retrieve the global variable plugin list */
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, PLUGIN_LIST_CHAR);
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Memory configuration */
  if((*plugin_list)->plugin[plugin_index].complex_config(config))
    return json_object_new_string(ERR_CONFIG_CHAR);

  return json_object_new_string(SUCCESS_CONFIG_CHAR);
}

json_object *api_memory_config(userdata_t *userdata, json_object *args)
{
  /* Variable definition */
  int plugin_index;
  json_type args_type;
  max_size_t Max_size;
  index_plugin_label_t Index_plugin_label;
  plugin_list_t **plugin_list;

  /* Ensure the memory library is open */
  if(!userdata->handle_memory)
    return json_object_new_string(ERR_LIB_CHAR);

  /* Retrieve the max_size function */
  Max_size = dlsym(userdata->handle_collectd, MAX_SIZE_CHAR);
  if(!Max_size)
    return json_object_new_string(dlerror());

  /* Retrieve the index plugin label function */
  Index_plugin_label = dlsym(userdata->handle_collectd, INDEX_PLUGIN_LABEL_CHAR);
  if(!Index_plugin_label)
    return json_object_new_string(dlerror());

  /* Retrieve the plugin list variable */
  plugin_list = (plugin_list_t **) dlsym(userdata->handle_collectd, PLUGIN_LIST_CHAR);
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Ensure the plugin list ain't NULL */
  if(!(*plugin_list))
    return json_object_new_string(ERR_PLUGIN_NULL_CHAR);

  /* First, let's ensure the list has a memory plugin initialize */
  plugin_index = (*Index_plugin_label)(*plugin_list, MEMORY_CHAR);
  if(plugin_index == -1)
    return json_object_new_string(ERR_PLUGIN_STORED_CHAR);

  /* Retrieve the type of the configuration and ensure it's a good one */
  args_type = json_object_get_type(args);
  if(args_type != json_type_string)
    return json_object_new_string(ERR_ARG_CHAR);

  /* Launch the memory init callack */
  if((*plugin_list)->plugin[plugin_index].init())
    return json_object_new_string(ERR_INIT_CHAR);

  /* Absolute configuration case */
  if(!strncmp(json_object_get_string(args), MEMORY_ABSOLUTE_CHAR, (*Max_size)(strlen(MEMORY_ABSOLUTE_CHAR), strlen(json_object_get_string(args)))))
   return api_mem_config_absolute(userdata, plugin_index);

  /* Percentage configuration case */
  else if(!strncmp(json_object_get_string(args), MEMORY_PERCENT_CHAR, (*Max_size)(strlen(MEMORY_PERCENT_CHAR), strlen(json_object_get_string(args)))))
    return api_mem_config_percent(userdata, plugin_index);

  /* All configuration case */
  else if(!strncmp(json_object_get_string(args), MEMORY_ALL_CHAR, (*Max_size)(strlen(MEMORY_ALL_CHAR), strlen(json_object_get_string(args)))))
    return api_mem_config_all(userdata, plugin_index);

  /* Uknown configuration */
  else
    return json_object_new_string(ERR_CONFIG_UNKNOWN_CHAR);
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            READ CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *api_memory_read(userdata_t *userdata)
{
  /* Variables definition */
  int plugin_index;
  metrics_list_t **metrics_list;
  plugin_list_t **plugin_list;
  metrics_deinit_t Metrics_deinit;
  max_size_t Max_size;
  index_plugin_label_t Index_plugin_label;
  json_object *res;

  /* Ensure the memory library is opened */
  if(!userdata->handle_memory)
    return json_object_new_string(ERR_LIB_CHAR);

  /* Variable allocation */
  res = json_object_new_object();

  /* Retrieve the global variable plugin list */
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, PLUGIN_LIST_CHAR);
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Ensure the plugin list ain't NULL */
  if(!*plugin_list)
    return json_object_new_string(ERR_PLUGIN_NULL_CHAR);

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

  /* Ensure a plugin named memory is stored and retrieve its index */
  plugin_index = (*Index_plugin_label)(*plugin_list, MEMORY_CHAR);
  if(plugin_index == -1)
    return json_object_new_string(ERR_PLUGIN_STORED_CHAR);

  /* Call the memory callbacks read */
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

json_object *api_memory_reset(userdata_t *userdata)
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

  /* Ensure the memory library is opened */
  if(!userdata->handle_memory)
    return json_object_new_string(ERR_LIB_CHAR);

  /* Ensure the plugin list ain't NULL */
  if(!*plugin_list)
    return json_object_new_string(ERR_PLUGIN_NULL_CHAR);

  /* Ensure the memory plugin is registered in the plugin list */
  if((*Index_plugin_label)(*plugin_list, MEMORY_CHAR) == -1)
    return json_object_new_string(ERR_PLUGIN_STORED_CHAR);

  /* Retrieve the index of the memory plugin */
  plugin_index = (*Index_plugin_label)(*plugin_list, MEMORY_CHAR);

  /* Delete the memory plugin from the list */
  if((*Plugin_deinit)(plugin_index))
    return json_object_new_string(ERR_RESET_CHAR);

  /* Close the memory library */
  dlclose(userdata->handle_memory);
  userdata->handle_memory = NULL;
  return json_object_new_string(SUCCESS_RESET_CHAR);
}
