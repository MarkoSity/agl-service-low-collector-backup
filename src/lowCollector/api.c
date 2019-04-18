/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            INCLUDES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#include "api.h"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            API FUNCTIONS CODE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                        INITIALIZATION CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* PLUGIN LIST HANDLING */
int api_plugin_init(plugin_t *plugin_list, json_object *arg)
{
  json_object *args;
  json_object *current_plugin;
  json_type args_type;
  int plugin_number;
  char *plugin_label;
  int loading_count;

  args = json_object_new_object();
  current_plugin = json_object_new_object();

  loading_count = 0;

  if(!json_object_object_get_ex(arg, "plugin", &args))
    return -1;

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
      return -1;
  }

  /* For each one of the previous plugin */
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
    if(!strncmp(plugin_label, "cpu", max_size((size_t) 3, strlen(plugin_label))))
    {
      if(!api_plugin_init_cpu(plugin_list))
      {
        loading_count ++;
        continue;
      }
    }
  }

  /* If none of the previous plugins were known */
  if(!loading_count)
    return -1;

  return 0;
}

/* CPU PLUGIN LIST HANDLING */
int api_plugin_init_cpu(plugin_t *plugin_list)
{
    void *handle;
    module_register_t module_register;

    /* First let's check if a plugin with the cpu name already exists */
    if(index_plugin_label(plugin_list, "cpu") != -1)
        return -1;

    /* Open the cpu library */
    handle = dlopen(CPU_PATH, RTLD_NOW || RTLD_GLOBAL);

    /* If it fails */
    if(!handle)
        return -1;

    /* Load the module register symbol */
    module_register = dlsym(handle, "module_register");

    /* If it fails */
    if(!module_register)
        return -1;

    /* Call the module register function to create the plugin and store its callbacks */
    (module_register)();

    return 0;
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            CONFIGURATION CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

int api_plugin_config(plugin_t *plugin_list, json_object *arg)
{
  json_object *args;
  args = json_object_new_object();

  if(json_object_object_get_ex(arg, "cpu", &args))
    return api_plugin_config_cpu(plugin_list, args);

  return -1;
}

int api_plugin_config_cpu(plugin_t *plugin_list, json_object *args)
{
  int plugin_index;
  json_type args_type;
  char *config_label;

  config_label = (char*)malloc(sizeof(char));

  /* First, let's ensure the list has a cpu plugin initialize */
  plugin_index = index_plugin_label(plugin_list, "cpu");

  if(plugin_index == -1)
    return -1;

  /* Retrieve the type of the configuration and ensure it's a good one */
  args_type = json_object_get_type(args);
  if(args_type != json_type_string)
    return -1;

  /* Retrieve the configuration in a string */
  config_label = (char*)json_object_get_string(args);

  /* Initialize the cpu plugin */
  if((plugin_list)->plugin_callback[plugin_index].init())
    return -1;

  /* Apply the configuration */
  if(!strncmp(config_label, "mean", max_size((size_t) 8, strlen(config_label))))
    return api_cpu_config_mean(plugin_list, plugin_index);

  else if(!strncmp(config_label, "mean_cpu", max_size((size_t) 8, strlen(config_label))))
    return api_cpu_config_mean_cpu(plugin_list, plugin_index);

  else if(!strncmp(config_label, "mean_state", max_size((size_t) 8, strlen(config_label))))
    return api_cpu_config_mean_state(plugin_list, plugin_index);

  else if(!strncmp(config_label, "percent_state_cpu", max_size((size_t) 8, strlen(config_label))))
    return api_cpu_config_percent_state_cpu(plugin_list, plugin_index);

  else if(!strncmp(config_label, "jiffies_state_cpu", max_size((size_t) 8, strlen(config_label))))
    return api_cpu_config_mean_cpu(plugin_list, plugin_index);

  else if(!strncmp(config_label, "number", max_size((size_t) 8, strlen(config_label))))
    return api_cpu_config_number_cpu(plugin_list, plugin_index);

  else
    return -1;


}

int api_cpu_config_mean(plugin_t *plugin_list, int plugin_index)
{
  if(plugin_list->plugin_callback[plugin_index].config("ReportByCpu", "false")
  || plugin_list->plugin_callback[plugin_index].config("ReportByState", "false")
  || plugin_list->plugin_callback[plugin_index].config("ReportNumCpu", "false")
  || plugin_list->plugin_callback[plugin_index].config("ValuesPercentage", "false"))
    return -1;

  /* Launch the cpu read callack in order to initialise the collection */
  plugin_list->plugin_callback[plugin_index].read(NULL);
  return 0;
}

int api_cpu_config_mean_cpu(plugin_t *plugin_list, int plugin_index)
{
  if(plugin_list->plugin_callback[plugin_index].config("ReportByCpu", "true")
  || plugin_list->plugin_callback[plugin_index].config("ReportByState", "false")
  || plugin_list->plugin_callback[plugin_index].config("ReportNumCpu", "false")
  || plugin_list->plugin_callback[plugin_index].config("ValuesPercentage", "false"))
    return -1;

  /* Launch the cpu read callack in order to initialise the collection */
  plugin_list->plugin_callback[plugin_index].read(NULL);
  return 0;
}

int api_cpu_config_mean_state(plugin_t *plugin_list, int plugin_index)
{
  if(plugin_list->plugin_callback[plugin_index].config("ReportByCpu", "false")
  || plugin_list->plugin_callback[plugin_index].config("ReportByState", "true")
  || plugin_list->plugin_callback[plugin_index].config("ReportNumCpu", "false")
  || plugin_list->plugin_callback[plugin_index].config("ValuesPercentage", "false"))
    return -1;

  /* Launch the cpu read callack in order to initialise the collection */
  plugin_list->plugin_callback[plugin_index].read(NULL);
  return 0;
}

int api_cpu_config_percent_state_cpu(plugin_t *plugin_list, int plugin_index)
{
  if(plugin_list->plugin_callback[plugin_index].config("ReportByCpu", "true")
  || plugin_list->plugin_callback[plugin_index].config("ReportByState", "true")
  || plugin_list->plugin_callback[plugin_index].config("ReportNumCpu", "false")
  || plugin_list->plugin_callback[plugin_index].config("ValuesPercentage", "true"))
    return -1;

  return 0;
}

int api_cpu_config_jiffies_state_cpu(plugin_t *plugin_list, int plugin_index)
{
  if(plugin_list->plugin_callback[plugin_index].config("ReportByCpu", "true")
  || plugin_list->plugin_callback[plugin_index].config("ReportByState", "true")
  || plugin_list->plugin_callback[plugin_index].config("ReportNumCpu", "false")
  || plugin_list->plugin_callback[plugin_index].config("ValuesPercentage", "false"))
    return -1;
  /* Launch the cpu read callack in order to initialise the collection */
  plugin_list->plugin_callback[plugin_index].read(NULL);
  return 0;
}

int api_cpu_config_number_cpu(plugin_t *plugin_list, int plugin_index)
{
  if(plugin_list->plugin_callback[plugin_index].config("ReportByCpu", "false")
  || plugin_list->plugin_callback[plugin_index].config("ReportByState", "false")
  || plugin_list->plugin_callback[plugin_index].config("ReportNumCpu", "true")
  || plugin_list->plugin_callback[plugin_index].config("ValuesPercentage", "false"))
    return -1;

  /* Launch the cpu read callack in order to initialise the collection */
  plugin_list->plugin_callback[plugin_index].read(NULL);
  return 0;
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            READ CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
json_object *api_plugin_read(plugin_t *plugin_list, json_object *arg)
{
  json_object *args;
  json_type args_type;
  char *plugin_label;

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

  if(!strncmp(plugin_label, "cpu", max_size((size_t) 3, strlen(plugin_label))))
    return api_cpu_read(plugin_list);

  return NULL;
}

json_object *api_cpu_read(plugin_t *plugin_list)
{
  int plugin_index;
  void *handle;
  metrics_t **metrics_list;
  json_object *res_cpu;
  json_object *res;

  res_cpu = json_object_new_object();
  res = json_object_new_object();

  handle = dlopen(PLUGIN_PATH, RTLD_NOW | RTLD_GLOBAL);
  if(!handle)
    return NULL;

  metrics_list = dlsym(handle, "Metrics_collectd");
  if(!metrics_list)
    return NULL;

  /* Ensure a plugin named cpu is stored and retrieve its index */
  plugin_index = index_plugin_label(plugin_list, "cpu");
  if(plugin_index == -1)
    return NULL;

  /* Call the cpu callbacks read */
  if(plugin_list->plugin_callback[plugin_index].read(NULL))
    return NULL;

  if(!metrics_list)
  {
    printf("METRICS LIST NULL\n");
    return NULL;
  }

  printf("metrics_list address : %p\n", metrics_list);

  for(int i = 0 ; i != (*metrics_list)->size ; i++)
  {
    /* The metrics plugin has to be cpu */
    if(!strncmp((*metrics_list)->metrics[i].plugin, "cpu", max_size((size_t) 3, strlen((*metrics_list)->metrics[i].plugin))))
    {
      /* If the plugin configuration regards the number of cpu */
      if(!strncmp((*metrics_list)->metrics[i].type, "count", max_size((size_t) 3, strlen((*metrics_list)->metrics[i].plugin))))
      {
        /* Pack the metrics in a j-son */
        wrap_json_pack(&res_cpu,
                       "{sf}",
                       "n°cpu(s)", (*metrics_list)->metrics[i].values->gauge);
        json_object_object_add(res, (*metrics_list)->metrics[i].type, res_cpu);
      }

      /* If the plugin configuration regards the percentage of each state of each cpu */
      if(!strncmp((*metrics_list)->metrics[i].type, "percent", max_size((size_t) 3, strlen((*metrics_list)->metrics[i].plugin))))
      {
        /* Pack the metrics in a j-son */
        wrap_json_pack(&res_cpu,
                      "{sf}",
                       (*metrics_list)->metrics[i].type_instance, (*metrics_list)->metrics[i].values->gauge);
        json_object_object_add(res, strcat(strcat((*metrics_list)->metrics[i].type_instance, " "), (*metrics_list)->metrics[i].plugin_instance), res_cpu);
      }

      /* If the plugin configuration regards each state of each cpu in jiffies */
      if(!strncmp((*metrics_list)->metrics[i].type, "cpu", max_size((size_t) 3, strlen((*metrics_list)->metrics[i].plugin))))
      {
        /* Pack the metrics in a j-son */
        wrap_json_pack(&res_cpu,
                       "{sI}",
                       (*metrics_list)->metrics[i].type_instance, (*metrics_list)->metrics[i].values->derive);
        json_object_object_add(res, strcat(strcat((*metrics_list)->metrics[i].type_instance, " "), (*metrics_list)->metrics[i].plugin_instance), res_cpu);
      }
    }
  }

  /* Close the plugin library */
  dlclose(handle);

  /* Reset the metrics list  and notify the read is a success */
  metrics_deinit();
  return res;
}
