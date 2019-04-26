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

#include "write_json.h"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                          max_size
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

size_t max_size(size_t a, size_t b)
{
    if(a >= b)
        return a;
    return b;
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                          WRITE JSON CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *write_json(metrics_list_t *metrics_list)
{
  /* Variables definition */
  json_object *res;
  json_object *res_value_instance;
  json_object *res_value_without_instance;
  json_object *res_type_instance;
  json_object *res_type_without_instance;
  json_object *res_plugin_without_instance;
  json_object *res_plugin_instance;
  json_object *res_plugin;
  char *instance_label;

  /* Variables allocation */
  res = json_object_new_object();
  res_value_without_instance = json_object_new_object();
  res_type_instance = json_object_new_object();
  res_type_without_instance = json_object_new_object();
  res_plugin_without_instance = json_object_new_object();
  res_plugin_instance = json_object_new_object();
  res_plugin = json_object_new_object();

  /* Ensure the metrics list is not NULL */
  if(!metrics_list)
    return json_object_new_string("Metrics list NULL");

  /* Ensure the metrics list is not empty */
  if(!metrics_list->size)
    return json_object_new_string("Metrics list empty");

  /* For each one of the metrics stored in the list */
  for(int i = 0 ; i != metrics_list->size ; i++)
  {
    printf("plugin : %s\nplugin_instance : %s\ntype : %s\ntype instance : %s\nvalue : %lf\n\n",
        metrics_list->metrics[i].plugin, metrics_list->metrics[i].plugin_instance, metrics_list->metrics[i].type, metrics_list->metrics[i].type_instance, metrics_list->metrics[i].values->gauge);
    /* If the plugin instance is empty */
    if(!strncmp(metrics_list->metrics->plugin_instance, "", strlen(metrics_list->metrics->plugin_instance)))
    {
      printf("BONZOUR\n");
      json_object_object_add(res_value_without_instance, metrics_list->metrics[i].type_instance, json_object_new_double(metrics_list->metrics[i].values->gauge));
    }

    /* If the plugin instance is not empty */
    else
    {
      printf("HEHEHE\n");
      /* Label allocation */
      instance_label = (char *)malloc(strlen(metrics_list->metrics[i].plugin_instance)*sizeof(char));

      /* Json allocation */
      res_value_instance = json_object_new_object();
      res_type_instance = json_object_new_object();

      /* We store the plugin instance in a string */
      strcpy(instance_label, metrics_list->metrics[i].plugin_instance);

      /* While the plugin instance do not change, we pack the metrics regarding the plugin instance */
      while(!strncmp(instance_label, metrics_list->metrics[i].plugin_instance, max_size(strlen(instance_label), strlen(metrics_list->metrics[i].plugin_instance))))
      {
        json_object_object_add(res_value_instance, metrics_list->metrics[i].type_instance, json_object_new_double(metrics_list->metrics[i].values->gauge));
        i++;
      }

      i--;
      /* Pack the type of the previous plugin instance */
      json_object_object_add(res_type_instance, metrics_list->metrics[i].type, res_value_instance);

      /* Pack the metrics with it plugin instance */
      json_object_object_add(res_plugin_instance, instance_label, res_type_instance);
    }
  }

  /* Pack the metrics which are not concerned by a plugin instance */
  json_object_object_add(res_type_without_instance, metrics_list->metrics->type, res_value_without_instance);
  json_object_object_add(res_plugin_without_instance, metrics_list->metrics->plugin, res_type_without_instance);
  wrap_json_object_add(res_plugin_instance, res_plugin_without_instance);
  json_object_object_add(res_plugin, metrics_list->metrics->plugin, res_plugin_instance);
  json_object_object_add(res, metrics_list->metrics->host, res_plugin);


  /* If the plugin do not have an instance (memory, processes...) */
  /* if(!strncmp(metrics_list->metrics->plugin_instance, "", strlen(metrics_list->metrics->plugin_instance)))
  { */
    /* gathered all the metrics values associated to their field */
    /* for(int i = 0 ; i != metrics_list->size ; i++)
    {
      json_object_object_add(res_value, metrics_list->metrics[i].type_instance, json_object_new_double(metrics_list->metrics[i].values->gauge));
    }
    json_object_object_add(res_type, metrics_list->metrics->type, res_value);
    json_object_object_add(res_plugin, metrics_list->metrics->plugin, res_type);
    json_object_object_add(res, metrics_list->metrics->host, res_plugin);
  } */

  /* If the plugin has an instance, then we do the same work but for each instance of the plugin */
  /* else
  {
    char *instance_label;
    instance_label = (char *)malloc(sizeof(char));
    for(int i = 0 ; i != metrics_list->size ; i++)
    {
      instance_label = metrics_list->metrics[i].plugin_instance;
      while(!strncmp(instance_label, metrics_list->metrics[i].plugin_instance, max_size(strlen(instance_label), strlen(metrics_list->metrics[i].plugin_instance))))
      {
        json_object_object_add(res_value, metrics_list->metrics[i].type_instance, json_object_new_double(metrics_list->metrics[i].values->gauge));
        i++;
      }
      i--;
      json_object_object_add(res_type, metrics_list->metrics->type, res_value);
      json_object_object_add(res_plugin_instance, metrics_list->metrics[i].plugin_instance, res_type);
      res_value = json_object_new_object();
      res_type = json_object_new_object();
    }
    json_object_object_add(res_plugin, metrics_list->metrics->plugin, res_plugin_instance);
    json_object_object_add(res, metrics_list->metrics->host, res_plugin);

  }
 */
  return res;
}
