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
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, "Plugin_list");
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Retrieve the index plugin label function */
  Index_plugin_label = (index_plugin_label_t)dlsym(userdata->handle_collectd, "index_plugin_label");
  if(!Index_plugin_label)
    return json_object_new_string(dlerror());

  /* Load the module register symbol */
  module_register = (module_register_t)dlsym(userdata->handle_memory, "module_register");
  if(!module_register)
    return json_object_new_string(dlerror());

  /* First let's check if a plugin with the memory name already exists */
    if((*Index_plugin_label)(*plugin_list, "memory") != -1)
      return json_object_new_string("Plugin already stored");

  /* Call the module register function to create the plugin and store its callbacks */
  (module_register)();

  return json_object_new_string("Loaded.");
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
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, "Plugin_list");
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Memory configuration */
  if((*plugin_list)->plugin[plugin_index].complex_config(config))
    return json_object_new_string("Fail to apply 'absolute'.");

  return json_object_new_string("'absolute' apply.");
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
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, "Plugin_list");
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Memory configuration */
  if((*plugin_list)->plugin[plugin_index].complex_config(config))
    return json_object_new_string("Fail to apply 'percent'.");

  return json_object_new_string("'percent' apply.");
}

json_object *api_memory_config(userdata_t *userdata, json_object *args)
{
  /* Variable definition */
  int plugin_index;
  json_type args_type;
  char *config_label;
  max_size_t Max_size;
  index_plugin_label_t Index_plugin_label;
  plugin_list_t **plugin_list;

  /* Variable allocation */
  config_label = (char*)malloc(sizeof(char));

  /* Ensure the memory library is open */
  if(!userdata->handle_memory)
    return json_object_new_string("The memory plugin has not been initialized");

  /* Retrieve the max_size function */
  Max_size = dlsym(userdata->handle_collectd, "max_size");
  if(!Max_size)
    return json_object_new_string(dlerror());

  /* Retrieve the index plugin label function */
  Index_plugin_label = dlsym(userdata->handle_collectd, "index_plugin_label");
  if(!Index_plugin_label)
    return json_object_new_string(dlerror());

  /* Retrieve the plugin list variable */
  plugin_list = (plugin_list_t **) dlsym(userdata->handle_collectd, "Plugin_list");
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Ensure the plugin list ain't NULL */
  if(!(*plugin_list))
    return json_object_new_string("Plugin list is null.");

  /* First, let's ensure the list has a memory plugin initialize */
  plugin_index = (*Index_plugin_label)(*plugin_list, "memory");
  if(plugin_index == -1)
    return json_object_new_string("Plugin not stored.");

  /* Retrieve the type of the configuration and ensure it's a good one */
  args_type = json_object_get_type(args);
  if(args_type != json_type_string)
    return json_object_new_string("Fail to recognize arguments type (string).");

  /* Retrieve the configuration in a string */
  config_label = (char*)json_object_get_string(args);

  /* Launch the memory init callack */
  if((*plugin_list)->plugin[plugin_index].init())
    return json_object_new_string("Fail to initialize the memory plugin.");

  /* Absolute configuration case */
  if(!strncmp(config_label, "absolute", (*Max_size)((size_t) 8, strlen(config_label))))
    return api_mem_config_absolute(userdata, plugin_index);

  /* Percentage configuration case */
  else if(!strncmp(config_label, "percent", (*Max_size)((size_t) 7, strlen(config_label))))
    return api_mem_config_percent(userdata, plugin_index);

  return json_object_new_string("Unknown configuration.");
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
    return json_object_new_string("The memory plugin has not been initialized.");

  /* Variable allocation */
  res = json_object_new_object();

  /* Retrieve the global variable plugin list */
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, "Plugin_list");
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Retrieve the global variable metrics list */
  metrics_list = (metrics_list_t **)dlsym(userdata->handle_collectd, "Metrics_list");
  if(!metrics_list)
    return json_object_new_string(dlerror());

  /* Retrieve the metrics deinit function */
  Metrics_deinit = (metrics_deinit_t)dlsym(userdata->handle_collectd, "metrics_deinit");
  if(!Metrics_deinit)
    return json_object_new_string(dlerror());

  /* Retrieve the max size function */
  Max_size = (max_size_t)dlsym(userdata->handle_collectd, "max_size");
  if(!Max_size)
    return json_object_new_string(dlerror());

  /* Retrieve the index plugin label function */
  Index_plugin_label = (index_plugin_label_t)dlsym(userdata->handle_collectd, "index_plugin_label");
  if(!Index_plugin_label)
    return json_object_new_string(dlerror());

  /* Ensure a plugin named memory is stored and retrieve its index */
  plugin_index = (*Index_plugin_label)(*plugin_list, "memory");
  if(plugin_index == -1)
    return json_object_new_string("The memory plugin is not registered.");

  /* Call the memory callbacks read */
  if((*plugin_list)->plugin[plugin_index].read(NULL))
    return json_object_new_string("Fail to execute the memory read callback.");

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
  plugin_list = (plugin_list_t **)dlsym(userdata->handle_collectd, "Plugin_list");
  if(!plugin_list)
    return json_object_new_string(dlerror());

  /* Retrieve the plugin deinit function */
  Plugin_deinit = (plugin_deinit_t)dlsym(userdata->handle_collectd, "plugin_deinit");
  if(!Plugin_deinit)
    return json_object_new_string(dlerror());

  /* Retrieve the index plugin label function */
  Index_plugin_label = (index_plugin_label_t)dlsym(userdata->handle_collectd, "index_plugin_label");
  if(!Index_plugin_label)
    return json_object_new_string(dlerror());

  /* Ensure the memory library is opened */
  if(!userdata->handle_memory)
    return json_object_new_string("The memory plugin is not registered.");

  /* Ensure the memory plugin is registered in the plugin list */
  if((*Index_plugin_label)(*plugin_list, "memory") == -1)
    return json_object_new_string("The memory plugin is not loaded.");

  /* Retrieve the index of the memory plugin */
  plugin_index = (*Index_plugin_label)(*plugin_list, "memory");

  /* Delete the memory plugin from the list */
  if((*Plugin_deinit)(plugin_index))
    return json_object_new_string("Fail to remove the memory plugin.");

  /* Close the memory library */
  dlclose(userdata->handle_memory);
  userdata->handle_memory = NULL;
  return json_object_new_string("Remove.");
}
