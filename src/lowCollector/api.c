/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            INCLUDES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#include "api.h"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            DEFINE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#define PLUGIN_PATH "../build/src/collectd/./collectd_glue.so"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            API FUNCTIONS CODE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                        INITIALIZATION CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *api_plugin_init(json_object *arg)
{
  /* Variables definition */
  json_object *res;
  json_object *res_plugin;
  json_object *args;
  json_object *current_plugin;
  json_type args_type;
  int plugin_number;
  char *plugin_label;
  int loading_count;
  void *handle;
  max_size_t Max_size;

  /* Allocate the answer of the function */
  res = json_object_new_object();
  res_plugin = json_object_new_object();

  /* Open the collectd_glue library */
  handle = dlopen(PLUGIN_PATH, RTLD_NOW | RTLD_GLOBAL);
  if(!handle)
    return json_object_new_string(dlerror());

  /* Retrieve the max_size function from the collectd_glue library */
  Max_size = dlsym(handle, "max_size");
  if(!Max_size)
    return json_object_new_string(dlerror());

  /* Retrieve the j-son arguments */
  args = json_object_new_object();
  if(!json_object_object_get_ex(arg, "plugin", &args))
    return json_object_new_string(dlerror());

  /* Retrieve the type of the j-son arguments */
  args_type = json_object_get_type(args);

  /* Here we allow multiple plugin initialization */
  current_plugin = json_object_new_object();

  /* Retrieve the number of plugin specified in the j-son arguments */
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
      return json_object_new_string("Fail to recognize arguments type (string or string array)");
  }

  /* Define the loading counter which is usefull to know whether at least one plugin has been initialized */
  loading_count = 0;

  /* For each one of the plugin notified in the j-son arguments */
  for(int i = 0 ; i != plugin_number ; i++)
  {
    /* Retrieve the current plugin */
    if(args_type == json_type_array)
      current_plugin = json_object_array_get_idx(args, i);

    else
      current_plugin = args;

    /* If the type ain't the one we expected we noticed it but we continue the process */
    if(!json_object_is_type(current_plugin, json_type_string))
			continue;

    /* Retrieve the string in the current j-son object */
    plugin_label = (char*)json_object_get_string(current_plugin);

    /* Initialize the plugin only if it's a known one and has not been initialized yet */
    if(!strncmp(plugin_label, "cpu", (*Max_size)((size_t) 3, strlen(plugin_label))))
    {
      res_plugin = api_plugin_init_cpu();
      if(res_plugin)
      {
        json_object_object_add(res, plugin_label, res_plugin);
        loading_count ++;
        continue;
      }
    }
  }

  /* If none of the previous plugins were known */
  if(!loading_count)
    return json_object_new_string("Plugins already stored or unknown");

  return res;
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            CONFIGURATION CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *api_plugin_config(json_object *arg)
{
  /* Variables definition */
  json_object *args;
  void *handle_collectd;
  plugin_t **Plugin;

  /* Variable allocation */
  args = json_object_new_object();

  /* Open the collectd glue library */
  handle_collectd = dlopen(PLUGIN_PATH, RTLD_NOW | RTLD_GLOBAL);
  if(!handle_collectd)
    return json_object_new_string(dlerror());

  /* Retrieve the global variable plugin list form the collectd glue library */
  Plugin = dlsym(handle_collectd, "Plugin_collectd");
  if(!Plugin)
    return json_object_new_string(dlerror());

  /* Ensure the plugin list ain't NULL */
  if(!(*Plugin))
    return json_object_new_string("Plugin list is null.");

  /* Ensure the plugin list ain't empty */
  if(!(*Plugin))
    return json_object_new_string("Plugin list is empty.");

  /* Close the library we opened */
  dlclose(handle_collectd);

  /* CPU configuration case */
  if(json_object_object_get_ex(arg, "cpu", &args))
    return api_plugin_config_cpu(args);

  return json_object_new_string("Unknown plugin.");

}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            READ CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
json_object *api_plugin_read(plugin_t *plugin_list, json_object *arg)
{
  json_object *args;
  json_type args_type;
  char *plugin_label;
  void *handle;
  max_size_t Max_size;

  handle = dlopen(PLUGIN_PATH, RTLD_NOW | RTLD_GLOBAL);
  if(!handle)
    return NULL;

  Max_size = dlsym(handle, "max_size");
  if(!Max_size)
    return NULL;

  args = json_object_new_object();
  plugin_label = (char*)malloc(sizeof(char));

  /* Invalid key */
  if(!json_object_object_get_ex(arg, "plugin", &args))
		return NULL;

  /* Retrieve the argument type */
  args_type = json_object_get_type(args);
  if(args_type != json_type_string)
    return NULL;

  plugin_label = (char*)json_object_get_string(args);

  if(!strncmp(plugin_label, "cpu", (*Max_size)((size_t) 3, strlen(plugin_label))))
    return api_cpu_read(plugin_list);

  return NULL;
}
