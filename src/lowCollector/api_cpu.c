/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            INCLUDES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#include "api_cpu.h"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            DEFINE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#define CPU_PATH "../build/src/collectd/./cpu.so"

#define PLUGIN_PATH "../build/src/collectd/./collectd_glue.so"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            INITIALIZATION CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* CPU PLUGIN LIST HANDLING */
json_object *api_plugin_init_cpu()
{
  /* Variable definition */
  void *handle_cpu;
  void *handle_collectd;
  plugin_t **Plugin;
  module_register_t module_register;
  index_plugin_label_t Index_plugin_label;

  /* Open the cpu library */
  handle_cpu = dlopen(CPU_PATH, RTLD_NOW || RTLD_GLOBAL);
  if(!handle_cpu)
    return json_object_new_string(dlerror());

  /* Open the collectd glue library */
  handle_collectd = dlopen(PLUGIN_PATH, RTLD_NOW || RTLD_GLOBAL);
  if(!handle_collectd)
  {
    /* Close the library we opened and return the error */
    dlclose(handle_cpu);
    return json_object_new_string(dlerror());
  }

  /* Retrieve the global variable plugin list from the collectd glue library */
  Plugin = dlsym(handle_collectd, "Plugin_collectd");
  if(!Plugin)
  {
    /* Close the library we opened and return the error */
    dlclose(handle_cpu);
    dlclose(handle_collectd);
    return json_object_new_string(dlerror());
  }

  Index_plugin_label = dlsym(handle_collectd, "index_plugin_label");
  if(!Index_plugin_label)
  {
    /* Close the library we opened and return the error*/
    dlclose(handle_cpu);
    dlclose(handle_collectd);
    return json_object_new_string(dlerror());
  }

  /* Load the module register symbol */
  module_register = dlsym(handle_cpu, "module_register");
  if(!module_register)
  {
    /* Close the library we opened and return the error*/
    dlclose(handle_cpu);
    dlclose(handle_collectd);
    return json_object_new_string(dlerror());
  }

  /* First let's check if a plugin with the cpu name already exists */
    if((*Index_plugin_label)(*Plugin, "cpu") != -1)
    {
      /* Close the library we opened and return the error*/
      dlclose(handle_cpu);
      dlclose(handle_collectd);
      return json_object_new_string("Plugin already stored");
    }

  /* Call the module register function to create the plugin and store its callbacks */
  (module_register)();

  /* Close the library we opened and return the success*/
  dlclose(handle_cpu);
  dlclose(handle_collectd);
  return json_object_new_string("loaded");
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            CONFIGURATION CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *api_plugin_config_cpu(json_object *args)
{
  int plugin_index;
  json_type args_type;
  char *config_label;
  void *handle_collectd;
  max_size_t Max_size;
  index_plugin_label_t Index_plugin_label;
  plugin_t **Plugin;

  config_label = (char*)malloc(sizeof(char));

  handle_collectd = dlopen(PLUGIN_PATH, RTLD_NOW | RTLD_GLOBAL);
  if(!handle_collectd)
    return json_object_new_string(dlerror());

  Max_size = dlsym(handle_collectd, "max_size");
  if(!Max_size)
    return json_object_new_string(dlerror());

  Index_plugin_label = dlsym(handle_collectd, "index_plugin_label");
  if(!Index_plugin_label)
    return json_object_new_string(dlerror());

  Plugin = dlsym(handle_collectd, "Plugin_collectd");
  if(!Plugin)
    return json_object_new_string(dlerror());

  /* First, let's ensure the list has a cpu plugin initialize */
  plugin_index = (*Index_plugin_label)(*Plugin, "cpu");

  if(plugin_index == -1)
    return json_object_new_string("Plugin not initialized");

  /* Retrieve the type of the configuration and ensure it's a good one */
  args_type = json_object_get_type(args);
  if(args_type != json_type_string)
    return json_object_new_string("Unknown type for arguments");

  /* Retrieve the configuration in a string */
  config_label = (char*)json_object_get_string(args);

  printf("%s : Plugin address %p : \n", __func__, *Plugin);
  printf("%s : Plugin callback address %p : \n", __func__, (*Plugin)->plugin_callback);
  printf("%s : Plugin init callback address %p : \n", __func__, (*Plugin)->plugin_callback[plugin_index].init);
  printf("%s : Plugin config callback address %p : \n", __func__, (*Plugin)->plugin_callback[plugin_index].config);
  printf("%s : Plugin complex config callback address %p : \n", __func__, (*Plugin)->plugin_callback[plugin_index].complex_config);
  printf("%s : Plugin read callback address %p : \n", __func__, (*Plugin)->plugin_callback[plugin_index].read);

  /* Initialize the cpu plugin */
  if((*Plugin)->plugin_callback[plugin_index].init())
    return json_object_new_string("Fail to initialize cpu plugin");

  /* Apply the configuration */
  if(!strncmp(config_label, "mean", (*Max_size)((size_t) 8, strlen(config_label))))
  {
    /* Close the opened library */
    dlclose(handle_collectd);
    return api_cpu_config_mean(plugin_index);
  }

  else if(!strncmp(config_label, "mean_cpu", (*Max_size)((size_t) 8, strlen(config_label))))
  {
    /* Close the opened library */
    dlclose(handle_collectd);
    return api_cpu_config_mean_cpu(plugin_index);
  }

  else if(!strncmp(config_label, "mean_state", (*Max_size)((size_t) 8, strlen(config_label))))
  {
    /* Close the opened library */
    dlclose(handle_collectd);
    return api_cpu_config_mean_state(plugin_index);
  }

  else if(!strncmp(config_label, "percent_state_cpu", (*Max_size)((size_t) 8, strlen(config_label))))
  {
    /* Close the opened library */
    dlclose(handle_collectd);
    return api_cpu_config_percent_state_cpu(plugin_index);
  }

  else if(!strncmp(config_label, "jiffies_state_cpu", (*Max_size)((size_t) 8, strlen(config_label))))
  {
    /* Close the opened library */
    dlclose(handle_collectd);
    return api_cpu_config_mean_cpu(plugin_index);
  }

  else if(!strncmp(config_label, "number", (*Max_size)((size_t) 8, strlen(config_label))))
  {
    /* Close the opened library */
    dlclose(handle_collectd);
    return api_cpu_config_number_cpu(plugin_index);
  }

  else
  {
    /* Close the opened library */
    dlclose(handle_collectd);
    return json_object_new_string("Unknown configuration");
  }
}

json_object *api_cpu_config_mean(int plugin_index)
{
  /* Variable definition */
  void *handle_collectd;
  plugin_t **Plugin;

  /* Open the collectd glue library */
  handle_collectd = dlopen(PLUGIN_PATH, RTLD_NOW | RTLD_GLOBAL);
  if(!handle_collectd)
    return json_object_new_string(dlerror());

  /* Retrieve the global variable plugin list */
  Plugin = dlsym(handle_collectd, "Plugin_collectd");
  if(!Plugin)
    return json_object_new_string(dlerror());

  if((*Plugin)->plugin_callback[plugin_index].config("ReportByCpu", "false")
  || (*Plugin)->plugin_callback[plugin_index].config("ReportByState", "false")
  || (*Plugin)->plugin_callback[plugin_index].config("ReportNumCpu", "false")
  || (*Plugin)->plugin_callback[plugin_index].config("ValuesPercentage", "false"))
    return json_object_new_string("Fail to apply 'mean' configuration");

  /* Launch the cpu read callack in order to initialise the collection */
  (*Plugin)->plugin_callback[plugin_index].read(NULL);

  /* Close the collectd glue library */
  dlclose(handle_collectd);
  return json_object_new_string("'mean' configuration successfully apply");
}

json_object *api_cpu_config_mean_cpu(int plugin_index)
{
  /* Variable definition */
  void *handle_collectd;
  plugin_t **Plugin;

  /* Open the collectd glue library */
  handle_collectd = dlopen(PLUGIN_PATH, RTLD_NOW | RTLD_GLOBAL);
  if(!handle_collectd)
    return json_object_new_string(dlerror());

  /* Retrieve the global variable plugin list */
  Plugin = dlsym(handle_collectd, "Plugin_collectd");
  if(!Plugin)
    return json_object_new_string(dlerror());

  if((*Plugin)->plugin_callback[plugin_index].config("ReportByCpu", "true")
  || (*Plugin)->plugin_callback[plugin_index].config("ReportByState", "false")
  || (*Plugin)->plugin_callback[plugin_index].config("ReportNumCpu", "false")
  || (*Plugin)->plugin_callback[plugin_index].config("ValuesPercentage", "false"))
    return json_object_new_string("Fail to apply 'mean cpu' configuration");

  /* Launch the cpu read callack in order to initialise the collection */
  (*Plugin)->plugin_callback[plugin_index].read(NULL);

  /* Close the collectd glue library */
  dlclose(handle_collectd);
  return json_object_new_string("'mean cpu' configuration successfully apply");
}

json_object *api_cpu_config_mean_state(int plugin_index)
{
  /* Variable definition */
  void *handle_collectd;
  plugin_t **Plugin;

  /* Open the collectd glue library */
  handle_collectd = dlopen(PLUGIN_PATH, RTLD_NOW | RTLD_GLOBAL);
  if(!handle_collectd)
    return json_object_new_string(dlerror());

  /* Retrieve the global variable plugin list */
  Plugin = dlsym(handle_collectd, "Plugin_collectd");
  if(!Plugin)
    return json_object_new_string(dlerror());

  if((*Plugin)->plugin_callback[plugin_index].config("ReportByCpu", "false")
  || (*Plugin)->plugin_callback[plugin_index].config("ReportByState", "true")
  || (*Plugin)->plugin_callback[plugin_index].config("ReportNumCpu", "false")
  || (*Plugin)->plugin_callback[plugin_index].config("ValuesPercentage", "false"))
    return json_object_new_string("Fail to apply 'mean state' configuration");

  /* Launch the cpu read callack in order to initialise the collection */
  (*Plugin)->plugin_callback[plugin_index].read(NULL);

  /* Close the collectd glue library */
  dlclose(handle_collectd);
  return json_object_new_string("'mean state' configuration successfully apply");
}

json_object *api_cpu_config_percent_state_cpu(int plugin_index)
{
  /* Variable definition */
  void *handle_collectd;
  plugin_t **Plugin;

  /* Open the collectd glue library */
  handle_collectd = dlopen(PLUGIN_PATH, RTLD_NOW | RTLD_GLOBAL);
  if(!handle_collectd)
    return json_object_new_string(dlerror());

  /* Retrieve the global variable plugin list */
  Plugin = dlsym(handle_collectd, "Plugin_collectd");
  if(!Plugin)
    return json_object_new_string(dlerror());

  if((*Plugin)->plugin_callback[plugin_index].config("ReportByCpu", "true")
  || (*Plugin)->plugin_callback[plugin_index].config("ReportByState", "true")
  || (*Plugin)->plugin_callback[plugin_index].config("ReportNumCpu", "false")
  || (*Plugin)->plugin_callback[plugin_index].config("ValuesPercentage", "true"))
    return json_object_new_string("Fail to apply 'percent state cpu' configuration");

  /* Close the collectd glue library */
  dlclose(handle_collectd);

  return json_object_new_string("'percent state cpu' configuration successfully apply");
}

json_object *api_cpu_config_jiffies_state_cpu(int plugin_index)
{
  /* Variable definition */
  void *handle_collectd;
  plugin_t **Plugin;

  /* Open the collectd glue library */
  handle_collectd = dlopen(PLUGIN_PATH, RTLD_NOW | RTLD_GLOBAL);
  if(!handle_collectd)
    return json_object_new_string(dlerror());

  /* Retrieve the global variable plugin list */
  Plugin = dlsym(handle_collectd, "Plugin_collectd");
  if(!Plugin)
    return json_object_new_string(dlerror());

  if((*Plugin)->plugin_callback[plugin_index].config("ReportByCpu", "true")
  || (*Plugin)->plugin_callback[plugin_index].config("ReportByState", "true")
  || (*Plugin)->plugin_callback[plugin_index].config("ReportNumCpu", "false")
  || (*Plugin)->plugin_callback[plugin_index].config("ValuesPercentage", "false"))
    return json_object_new_string("Fail to apply 'jiffies state cpu' configuration");

  /* Close the collectd glue library */
  dlclose(handle_collectd);

  /* Launch the cpu read callack in order to initialise the collection */
  (*Plugin)->plugin_callback[plugin_index].read(NULL);
  return json_object_new_string("'jiffies state cpu' configuration successfully apply");
}

json_object *api_cpu_config_number_cpu(int plugin_index)
{
  /* Variable definition */
  void *handle_collectd;
  plugin_t **Plugin;

  /* Open the collectd glue library */
  handle_collectd = dlopen(PLUGIN_PATH, RTLD_NOW | RTLD_GLOBAL);
  if(!handle_collectd)
    return json_object_new_string(dlerror());

  /* Retrieve the global variable plugin list */
  Plugin = dlsym(handle_collectd, "Plugin_collectd");
  if(!Plugin)
    return json_object_new_string(dlerror());

  if((*Plugin)->plugin_callback[plugin_index].config("ReportByCpu", "false")
  || (*Plugin)->plugin_callback[plugin_index].config("ReportByState", "false")
  || (*Plugin)->plugin_callback[plugin_index].config("ReportNumCpu", "true")
  || (*Plugin)->plugin_callback[plugin_index].config("ValuesPercentage", "false"))
    return json_object_new_string("Fail to apply 'number' configuration");

  /* Close the collectd glue library */
  dlclose(handle_collectd);

  /* Launch the cpu read callack in order to initialise the collection */
  (*Plugin)->plugin_callback[plugin_index].read(NULL);
  return json_object_new_string("'number' configuration successfully apply");
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            READ CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *api_cpu_read(plugin_t *plugin_list)
{
  int plugin_index;
  void *handle;
  metrics_t **Metrics_list;
  metrics_deinit_t Metrics_deinit;
  max_size_t Max_size;
  index_plugin_label_t Index_plugin_label;
  json_object *res_cpu;
  json_object *res;

  res_cpu = json_object_new_object();
  res = json_object_new_object();

  handle = dlopen(PLUGIN_PATH, RTLD_NOW | RTLD_GLOBAL);
  if(!handle)
    return NULL;

  Metrics_list = dlsym(handle, "Metrics_collectd");
  if(!Metrics_list)
    return NULL;

  Metrics_deinit = dlsym(handle, "metrics_deinit");
  if(!Metrics_deinit)
    return NULL;

  Max_size = dlsym(handle, "max_size");
  if(!Max_size)
    return NULL;

  Index_plugin_label = dlsym(handle, "index_plugin_label");
  if(!Index_plugin_label)
    return NULL;

  /* Ensure a plugin named cpu is stored and retrieve its index */
  plugin_index = (*Index_plugin_label)(plugin_list, "cpu");
  if(plugin_index == -1)
    return NULL;

  /* Call the cpu callbacks read */
  if(plugin_list->plugin_callback[plugin_index].read(NULL))
    return NULL;

  for(int i = 0 ; i != (*Metrics_list)->size ; i++)
  {
    /* The metrics plugin has to be cpu */
    if(!strncmp((*Metrics_list)->metrics[i].plugin, "cpu", (*Max_size)((size_t) 3, strlen((*Metrics_list)->metrics[i].plugin))))
    {
      /* If the plugin configuration regards the number of cpu */
      if(!strncmp((*Metrics_list)->metrics[i].type, "count", (*Max_size)((size_t) 3, strlen((*Metrics_list)->metrics[i].plugin))))
      {
        /* Pack the metrics in a j-son */
        wrap_json_pack(&res_cpu,
                       "{sf}",
                       "n°cpu(s)", (*Metrics_list)->metrics[i].values->gauge);
        json_object_object_add(res, (*Metrics_list)->metrics[i].type, res_cpu);
      }

      /* If the plugin configuration regards the percentage of each state of each cpu */
      if(!strncmp((*Metrics_list)->metrics[i].type, "percent", (*Max_size)((size_t) 3, strlen((*Metrics_list)->metrics[i].plugin))))
      {
        /* Pack the metrics in a j-son */
        wrap_json_pack(&res_cpu,
                      "{sf}",
                       (*Metrics_list)->metrics[i].type_instance, (*Metrics_list)->metrics[i].values->gauge);
        json_object_object_add(res, strcat(strcat((*Metrics_list)->metrics[i].type_instance, " "), (*Metrics_list)->metrics[i].plugin_instance), res_cpu);
      }

      /* If the plugin configuration regards each state of each cpu in jiffies */
      if(!strncmp((*Metrics_list)->metrics[i].type, "cpu", (*Max_size)((size_t) 3, strlen((*Metrics_list)->metrics[i].plugin))))
      {
        /* Pack the metrics in a j-son */
        wrap_json_pack(&res_cpu,
                       "{sI}",
                       (*Metrics_list)->metrics[i].type_instance, (*Metrics_list)->metrics[i].values->derive);
        json_object_object_add(res, strcat(strcat((*Metrics_list)->metrics[i].type_instance, " "), (*Metrics_list)->metrics[i].plugin_instance), res_cpu);
      }
    }
  }

  /* Reset the metrics list  and notify the read is a success */
  (*Metrics_deinit)();

  /* Close the plugin library */
  dlclose(handle);

  return res;
}
