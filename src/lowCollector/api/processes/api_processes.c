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
#include "api_processes.h"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            DEFINE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#define PROCESSES_PATH "../build/src/collectd/./processes.so"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            INITIALIZATION CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *api_processes_init(userdata_t *userdata)
{
  /* Variable definition */
  plugin_t **Plugin;
  module_register_t module_register;
  index_plugin_label_t Index_plugin_label;

  /* Open the processes library only if it ain't already open*/
  if(!userdata->handle_processes)
  {
    userdata->handle_processes = dlopen(PROCESSES_PATH, RTLD_NOW || RTLD_GLOBAL);
    if(!userdata->handle_processes)
      return json_object_new_string(dlerror());
  }

  /* Retrieve the global variable plugin list from the collectd glue library */
  Plugin = (plugin_t **)dlsym(userdata->handle_collectd, "Plugin_collectd");
  if(!Plugin)
    return json_object_new_string(dlerror());

  /* Retrieve the index plugin label function */
  Index_plugin_label = (index_plugin_label_t)dlsym(userdata->handle_collectd, "index_plugin_label");
  if(!Index_plugin_label)
    return json_object_new_string(dlerror());

  /* Load the module register symbol */
  module_register = (module_register_t)dlsym(userdata->handle_processes, "module_register");
  if(!module_register)
    return json_object_new_string(dlerror());

  /* First let's check if a plugin with the memory name already exists */
    if((*Index_plugin_label)(*Plugin, "processes") != -1)
      return json_object_new_string("Plugin already stored");

  /* Call the module register function to create the plugin and store its callbacks */
  (module_register)();

  return json_object_new_string("Loaded.");
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            CONFIGURATION CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *api_processes_config(userdata_t *userdata, json_object *args)
{
  /* Variable definition */
  int plugin_index;
  json_type args_type;
  char *config_label;
  max_size_t Max_size;
  index_plugin_label_t Index_plugin_label;
  plugin_t **Plugin;

  /* Variable allocation */
  config_label = (char*)malloc(sizeof(char));

  /* Ensure the processes library is open */
  if(!userdata->handle_processes)
    return json_object_new_string("The processes plugin has not been initialized");

  /* Retrieve the max_size function */
  Max_size = dlsym(userdata->handle_collectd, "max_size");
  if(!Max_size)
    return json_object_new_string(dlerror());

  /* Retrieve the index plugin label function */
  Index_plugin_label = dlsym(userdata->handle_collectd, "index_plugin_label");
  if(!Index_plugin_label)
    return json_object_new_string(dlerror());

  /* Retrieve the plugin list variable */
  Plugin = (plugin_t**) dlsym(userdata->handle_collectd, "Plugin_collectd");
  if(!Plugin)
    return json_object_new_string(dlerror());

  /* Ensure the plugin list ain't NULL */
  if(!(*Plugin))
    return json_object_new_string("Plugin list is null.");

  /* First, let's ensure the list has a processes plugin initialize */
  plugin_index = (*Index_plugin_label)(*Plugin, "processes");
  if(plugin_index == -1)
    return json_object_new_string("Plugin not stored.");

  /* Retrieve the type of the configuration and ensure it's a good one */
  args_type = json_object_get_type(args);
  if(args_type != json_type_string)
    return json_object_new_string("Fail to recognize arguments type (string).");

  /* Retrieve the configuration in a string */
  config_label = (char*)json_object_get_string(args);

  /* Launch the memory init callack */
  if((*Plugin)->plugin_callback[plugin_index].init())
    return json_object_new_string("Fail to initialize the processes plugin.");

  /* Context configuration case */
  if(!strncmp(config_label, "context", (*Max_size)((size_t) 7, strlen(config_label))))
    return api_processes_config_context(userdata, plugin_index);

  /* File configuration case */
  else if(!strncmp(config_label, "file", (*Max_size)((size_t) 4, strlen(config_label))))
    return api_processes_config_file(userdata, plugin_index);

  /* Memory configuration case */
  else if(!strncmp(config_label, "memory", (*Max_size)((size_t) 8, strlen(config_label))))
    return api_processes_config_memory(userdata, plugin_index);

  return json_object_new_string("Unknown configuration.");
}

json_object *api_processes_config_context(userdata_t *userdata, int plugin_index)
{
  /* Variables definition */
  plugin_t **Plugin;
  oconfig_item_t *config;

  /* Variable allocation */
  config = (oconfig_item_t *)malloc(sizeof(oconfig_item_t));
  config->children = (oconfig_item_t*)malloc(4*sizeof(oconfig_item_t));
  config->children[0].values = (oconfig_value_t *)malloc(sizeof(oconfig_value_t));
  config->children[1].values = (oconfig_value_t *)malloc(sizeof(oconfig_value_t));
  config->children[2].values = (oconfig_value_t *)malloc(sizeof(oconfig_value_t));

  /* Set the configuration */
  config->children_num = 3;

  config->children[0].key = "CollectContextSwitch";
  config->children[0].values_num = 1;
  config->children[0].values->type = OCONFIG_TYPE_BOOLEAN;
  config->children[0].values->value.boolean = true;

  config->children[1].key = "CollectFileDescriptor";
  config->children[1].values_num = 1;
  config->children[1].values->type = OCONFIG_TYPE_BOOLEAN;
  config->children[1].values->value.boolean = false;

  config->children[2].key = "CollectMemoryMaps";
  config->children[2].values_num = 1;
  config->children[2].values->type = OCONFIG_TYPE_BOOLEAN;
  config->children[2].values->value.boolean = false;

  /* Retrieve the global variable plugin list */
  Plugin = (plugin_t **)dlsym(userdata->handle_collectd, "Plugin_collectd");
  if(!Plugin)
    return json_object_new_string(dlerror());

  /* Processes configuration */
  if((*Plugin)->plugin_callback->complex_config(config))
    return json_object_new_string("Fail to apply 'context'.");

  return json_object_new_string("'context' apply.");
}

json_object *api_processes_config_file(userdata_t *userdata, int plugin_index)
{
  /* Variables definition */
  plugin_t **Plugin;
  oconfig_item_t *config;

  /* Variable allocation */
  config = (oconfig_item_t *)malloc(sizeof(oconfig_item_t));
  config->children = (oconfig_item_t*)malloc(4*sizeof(oconfig_item_t));
  config->children[0].values = (oconfig_value_t *)malloc(sizeof(oconfig_value_t));
  config->children[1].values = (oconfig_value_t *)malloc(sizeof(oconfig_value_t));
  config->children[2].values = (oconfig_value_t *)malloc(sizeof(oconfig_value_t));

  /* Set the configuration */
  config->children_num = 3;

  config->children[0].key = "CollectContextSwitch";
  config->children[0].values_num = 1;
  config->children[0].values->type = OCONFIG_TYPE_BOOLEAN;
  config->children[0].values->value.boolean = false;

  config->children[1].key = "CollectFileDescriptor";
  config->children[1].values_num = 1;
  config->children[1].values->type = OCONFIG_TYPE_BOOLEAN;
  config->children[1].values->value.boolean = true;

  config->children[2].key = "CollectMemoryMaps";
  config->children[2].values_num = 1;
  config->children[2].values->type = OCONFIG_TYPE_BOOLEAN;
  config->children[2].values->value.boolean = false;

  /* Retrieve the global variable plugin list */
  Plugin = (plugin_t **)dlsym(userdata->handle_collectd, "Plugin_collectd");
  if(!Plugin)
    return json_object_new_string(dlerror());

  /* Processes configuration */
  if((*Plugin)->plugin_callback->complex_config(config))
    return json_object_new_string("Fail to apply 'file'.");

  return json_object_new_string("'file' apply.");
}

json_object *api_processes_config_memory(userdata_t *userdata, int plugin_index)
{
  /* Variables definition */
  plugin_t **Plugin;
  oconfig_item_t *config;

  /* Variable allocation */
  config = (oconfig_item_t *)malloc(sizeof(oconfig_item_t));
  config->children = (oconfig_item_t*)malloc(3*sizeof(oconfig_item_t));
  config->children[0].values = (oconfig_value_t *)malloc(sizeof(oconfig_value_t));
  config->children[1].values = (oconfig_value_t *)malloc(sizeof(oconfig_value_t));
  config->children[2].values = (oconfig_value_t *)malloc(sizeof(oconfig_value_t));

  /* Set the configuration */
  config->children_num = 3;

  config->children[0].key = "CollectContextSwitch";
  config->children[0].values_num = 1;
  config->children[0].values->type = OCONFIG_TYPE_BOOLEAN;
  config->children[0].values->value.boolean = false;

  config->children[1].key = "CollectFileDescriptor";
  config->children[1].values_num = 1;
  config->children[1].values->type = OCONFIG_TYPE_BOOLEAN;
  config->children[1].values->value.boolean = false;

  config->children[2].key = "CollectMemoryMaps";
  config->children[2].values_num = 1;
  config->children[2].values->type = OCONFIG_TYPE_BOOLEAN;
  config->children[2].values->value.boolean = true;


  /* Retrieve the global variable plugin list */
  Plugin = (plugin_t **)dlsym(userdata->handle_collectd, "Plugin_collectd");
  if(!Plugin)
    return json_object_new_string(dlerror());

  /* Processes configuration */
  if((*Plugin)->plugin_callback->complex_config(config))
    return json_object_new_string("Fail to apply 'memory'.");

  return json_object_new_string("'memory' apply.");
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            READ CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *api_processes_read(userdata_t *userdata)
{
  /* Variables definition */
  int plugin_index;
  metrics_t **Metrics;
  plugin_t **Plugin;
  metrics_deinit_t Metrics_deinit;
  max_size_t Max_size;
  index_plugin_label_t Index_plugin_label;
  json_object *res;

  /* Ensure the processes library is opened */
  if(!userdata->handle_processes)
    return json_object_new_string("The processes plugin has not been initialized.");

  /* Variable allocation */
  res = json_object_new_object();

  /* Retrieve the global variable plugin list */
  Plugin = (plugin_t **)dlsym(userdata->handle_collectd, "Plugin_collectd");
  if(!Plugin)
    return json_object_new_string(dlerror());

  /* Retrieve the global variable metrics list */
  Metrics = (metrics_t **)dlsym(userdata->handle_collectd, "Metrics_collectd");
  if(!Metrics)
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

  /* Ensure a plugin named processes is stored and retrieve its index */
  plugin_index = (*Index_plugin_label)(*Plugin, "processes");
  if(plugin_index == -1)
    return json_object_new_string("The processes plugin is not registered.");

  /* Call the memory callbacks read */
  if((*Plugin)->plugin_callback[plugin_index].read(NULL))
    return json_object_new_string("Fail to execute the processes read callback.");

  res = write_json((*Metrics));

  /* If the metrics has been filled with values, we reset it */
  if((*Metrics)->metrics)
    (*Metrics_deinit)();

  return res;
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            RESET CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *api_processes_reset(userdata_t *userdata)
{
  /* Variables definition */
  plugin_t **Plugin;
  plugin_deinit_t Plugin_deinit;
  int plugin_index;
  index_plugin_label_t Index_plugin_label;

  /* Retrieve the global plugin list variable */
  Plugin = (plugin_t **)dlsym(userdata->handle_collectd, "Plugin_collectd");
  if(!Plugin)
    return json_object_new_string(dlerror());

  /* Retrieve the plugin deinit function */
  Plugin_deinit = (plugin_deinit_t)dlsym(userdata->handle_collectd, "plugin_deinit");
  if(!Plugin)
    return json_object_new_string(dlerror());

  /* Retrieve the index plugin label function */
  Index_plugin_label = (index_plugin_label_t)dlsym(userdata->handle_collectd, "index_plugin_label");
  if(!Index_plugin_label)
    return json_object_new_string(dlerror());

  /* Ensure the processes library is opened */
  if(!userdata->handle_processes)
    return json_object_new_string("The processes plugin is not registered.");

  /* Ensure the processes plugin is registered in the plugin list */
  if((*Index_plugin_label)(*Plugin, "processes") == -1)
    return json_object_new_string("The processes plugin is not loaded.");

  /* Retrieve the index of the processes plugin */
  plugin_index = (*Index_plugin_label)(*Plugin, "processes");

  /* Delete the processes plugin from the list */
  if((*Plugin_deinit)(plugin_index))
    return json_object_new_string("Fail to remove the processes plugin.");

  /* Close the processes library */
  dlclose(userdata->handle_processes);
  userdata->handle_processes = NULL;
  return json_object_new_string("Remove.");
}