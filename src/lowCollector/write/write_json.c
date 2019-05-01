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
                          Useful
                          function
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
  /* Variables declaration */
  json_object *res;
  json_object *res_plugin;
  char *plugin_instance_label;
  char *type_label;
  json_object *type_instance;
  json_object *type;
  json_object *plugin_instance;
  json_object *type_witout_instance;

  /* Variables allocation */
  res = json_object_new_object();
  res_plugin = json_object_new_object();
  plugin_instance = json_object_new_object();
  type_witout_instance = json_object_new_object();

  /* Ensure the metrics list is not NULL */
  if(!metrics_list)
    return json_object_new_string("Metrics list NULL.");

  /* Ensure the metrics list is not empty */
  if(!metrics_list->size)
    return json_object_new_string("Metrics list empty.");

  /* For each one of the metrics stored in the list */
  for(int i = 0 ; i != metrics_list->size ; i++)
  {
    /* If the current metrics has a plugin instance */
    if(strncmp(metrics_list->metrics[i].plugin_instance, "", strlen(metrics_list->metrics[i].plugin_instance)))
    {
      /* Json allocation due to json_object_object_add which destroy the json sources */
      type_instance = json_object_new_object();
      type = json_object_new_object();

      /* Let's store the plugin instance in a string */
      plugin_instance_label = malloc(strlen(metrics_list->metrics[i].plugin_instance)*sizeof(char));
      strcpy(plugin_instance_label, metrics_list->metrics[i].plugin_instance);

      /* While the plugin instance do not change */
      while(!strncmp(plugin_instance_label, metrics_list->metrics[i].plugin_instance, max_size(strlen(plugin_instance_label), strlen(metrics_list->metrics[i].plugin_instance))))
      {
        /* If the metrics do not have a type instance */
        if(!strncmp(metrics_list->metrics[i].type_instance, "", strlen(metrics_list->metrics[i].type_instance)))
          json_object_object_add(type_instance, metrics_list->metrics[i].plugin, json_object_new_double((double) metrics_list->metrics[i].values->gauge));
        
        /* Pack the metrics value with it type instance associated */
        else
          json_object_object_add(type_instance, metrics_list->metrics[i].type_instance, json_object_new_double((double) metrics_list->metrics[i].values->gauge));

        i++;
      }

      /* Because of the previous while loop, we went one incremmentation too far */
      i--;

      /* Then we associate it to its type */
      json_object_object_add(type, metrics_list->metrics[i].type, type_instance);

      /* Then we associate the json with its plugin instance */
      json_object_object_add(plugin_instance, metrics_list->metrics[i].plugin_instance, type);
    }

    /* If we reach that point, it means the metrics do not have a plugin instance, then we gathered all of these type of metrics in a json */
    else
    {
      /* Json allocation due to json_object_object_add which destroy the json sources */
      type_instance = json_object_new_object();
      type = json_object_new_object();

      /* Let's store the type label in a string */
      type_label = malloc(strlen(metrics_list->metrics[i].type)*sizeof(char));
      strcpy(type_label, metrics_list->metrics[i].type);

      /* While the type do not change */
      while(!strncmp(type_label, metrics_list->metrics[i].type, max_size(strlen(type_label), strlen(metrics_list->metrics[i].type))))
      {
        printf("name : %s, value : %f\n", metrics_list->metrics[i].type_instance, metrics_list->metrics[i].values->gauge);
        /* If the metrics do not have a type instance */
        if(!strncmp(metrics_list->metrics[i].type_instance, "", strlen(metrics_list->metrics[i].type_instance)))
          json_object_object_add(type_instance, metrics_list->metrics[i].plugin, json_object_new_double((double) metrics_list->metrics[i].values->gauge));
        
        /* Pack the metrics value with it type instance associated */
        else
          json_object_object_add(type_instance, metrics_list->metrics[i].type_instance, json_object_new_double((double) metrics_list->metrics[i].values->gauge));
        
        i++;
      }

      /* Because of the previous while loop, we went one incremmentation too far */
      i--;

      /* Then we associate it to its type */
      json_object_object_add(type_witout_instance, metrics_list->metrics[i].type, type_instance);
    }
  }

  /* Add without any key the two type of json */
  wrap_json_object_add(plugin_instance, type_witout_instance);

  /* Add the name of the plugin as a key to these variables */
  json_object_object_add(res_plugin, metrics_list->metrics->plugin, plugin_instance);

  json_object_object_add(res, metrics_list->metrics->host, res_plugin);
  return res;
}
