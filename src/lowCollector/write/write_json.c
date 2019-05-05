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
  /* Json */
  json_object *res;
  json_object *value_without_instance;
  json_object *value_with_instance;
  json_object *value;
  json_object *type;
  json_object *plugin_with_instance;
  json_object *plugin_without_instance;
  json_object *plugin_tmp;
  json_object *plugin;

  /* String label */
  char *host_label;
  char *plugin_label;
  char *plugin_instance_label;
  char *type_label;
  char *type_instance_label;

  int index = 0;

  /* Variable allocation */
  res = json_object_new_object();
  value_without_instance = json_object_new_object();
  value_with_instance = json_object_new_object();
  value = json_object_new_object();
  type = json_object_new_object();
  plugin_with_instance = json_object_new_object();
  plugin_without_instance = json_object_new_object();
  plugin_tmp = json_object_new_object();
  plugin = json_object_new_object();

  /* Ensure the metrics list is not NULL */
  if(!metrics_list)
    return json_object_new_string("Metrics list NULL.");

  /* Ensure the metrics list is not empty */
  if(!metrics_list->size)
    return json_object_new_string("Metrics list empty.");

  /* While index is not out of bond */
  while(index != metrics_list->size)
  {
    /* Retrieve and store the metrics host label */
    host_label = (char *)malloc(strlen(metrics_list->metrics[index].host) + 1);
    strcpy(host_label, metrics_list->metrics[index].host);

    /* While the metrics host do not change */
    while(!strncmp(metrics_list->metrics[index].host, host_label,
         max_size(strlen(metrics_list->metrics[index].host), strlen(host_label))))
    {
      /* Retrieve and store the metrics plugin */
      plugin_label = (char *)malloc(strlen(metrics_list->metrics[index].plugin) + 1);
      strcpy(plugin_label, metrics_list->metrics[index].plugin);

      /* While the metrics plugin do not change */
      while(!strncmp(metrics_list->metrics[index].host, host_label,
           max_size(strlen(metrics_list->metrics[index].host), strlen(host_label))) &&
           !strncmp(metrics_list->metrics[index].plugin, plugin_label,
           max_size(strlen(metrics_list->metrics[index].plugin), strlen(plugin_label))))
      {
        /* Retrieve and store the metrics plugin instance label */
        plugin_instance_label = (char *)malloc(strlen(metrics_list->metrics[index].plugin_instance) + 1);
        strcpy(plugin_instance_label, metrics_list->metrics[index].plugin_instance);

        /* While the metrics plugin instance do not change */
        while(!strncmp(metrics_list->metrics[index].host, host_label,
              max_size(strlen(metrics_list->metrics[index].host), strlen(host_label))) &&
              !strncmp(metrics_list->metrics[index].plugin, plugin_label,
              max_size(strlen(metrics_list->metrics[index].plugin), strlen(plugin_label))) &&
              !strncmp(metrics_list->metrics[index].plugin_instance, plugin_instance_label,
              max_size(strlen(metrics_list->metrics[index].plugin_instance), strlen(plugin_instance_label))))
        {
          /* Retrieve and store the metrics type */
          type_label = (char *)malloc(strlen(metrics_list->metrics[index].type) + 1);
          strcpy(type_label, metrics_list->metrics[index].type);

          /* While the metrics type do not change */
          while(!strncmp(metrics_list->metrics[index].host, host_label,
                max_size(strlen(metrics_list->metrics[index].host), strlen(host_label))) &&
                !strncmp(metrics_list->metrics[index].plugin, plugin_label,
                max_size(strlen(metrics_list->metrics[index].plugin), strlen(plugin_label))) &&
                !strncmp(metrics_list->metrics[index].plugin_instance, plugin_instance_label,
                max_size(strlen(metrics_list->metrics[index].plugin_instance), strlen(plugin_instance_label))) &&
                !strncmp(metrics_list->metrics[index].type, type_label,
                max_size(strlen(metrics_list->metrics[index].type), strlen(type_label))))
          {
            /* Retrieve and store the metrics type instance*/
            type_instance_label = (char *)malloc(strlen(metrics_list->metrics[index].type_instance) + 1);
            strcpy(type_instance_label, metrics_list->metrics[index].type_instance);

            /* While the type instance do not change */
            while(!strncmp(metrics_list->metrics[index].host, host_label,
                  max_size(strlen(metrics_list->metrics[index].host), strlen(host_label))) &&
                  !strncmp(metrics_list->metrics[index].plugin, plugin_label,
                  max_size(strlen(metrics_list->metrics[index].plugin), strlen(plugin_label))) &&
                  !strncmp(metrics_list->metrics[index].plugin_instance, plugin_instance_label,
                  max_size(strlen(metrics_list->metrics[index].plugin_instance), strlen(plugin_instance_label))) &&
                  !strncmp(metrics_list->metrics[index].type, type_label,
                  max_size(strlen(metrics_list->metrics[index].type), strlen(type_label))) &&
                  !strncmp(metrics_list->metrics[index].type_instance, type_instance_label,
                  max_size(strlen(metrics_list->metrics[index].type_instance), strlen(type_instance_label))))
            {
              /* If the type instance if empty, we put the value in its associated json */
              if(!strncmp(type_instance_label, "", strlen(type_instance_label)))
                wrap_json_object_add(value_without_instance,
                                    json_object_new_double(metrics_list->metrics[index].values->gauge));
              
               /* Else we add the value with it associated type instance */
              else
                json_object_object_add(value_with_instance, type_instance_label,
                                      json_object_new_double(metrics_list->metrics[index].values->gauge));

              /* We have just stored an other metrics value so we increment the index */
              index++;
            }

            /* Gathered the two type of value previously fill */
            wrap_json_object_add(value, value_without_instance);
            wrap_json_object_add(value, value_with_instance);

            /* Type instance label desallocation */
            sfree(type_instance_label);
          }

          /* Associate the values with their type */
          json_object_object_add(type, type_label, value);

          /* Json allocation because of the previous function which
          delete the json pointer */
          value = json_object_new_object();

          /* Type label desallocation */
          sfree(type_label);
        }

        /* If the current metrics do not have a plugin instance */
        if(!strncmp(plugin_instance_label, "", strlen(plugin_instance_label)))
          wrap_json_object_add(plugin_without_instance, type);

        /* Else the metrics has a plugin instance */
        else
          json_object_object_add(plugin_with_instance, plugin_instance_label, type);

        /* Json allocation because of the previous function which
        delete the json pointer */
        type = json_object_new_object();

        /* Plugin instance label desallocation */
        sfree(plugin_instance_label);
      }

      /* Gather the two previous json a temporary json plugin */
      wrap_json_object_add(plugin_tmp, plugin_with_instance);
      wrap_json_object_add(plugin_tmp, plugin_without_instance);

      /* Associate the previous json with their associated key */
      json_object_object_add(plugin, plugin_label, plugin_tmp);

      /* Json allocation because of the previous function which
      delete the json pointer */
      plugin_tmp = json_object_new_object();

      /* Plugin label desallocation */
      sfree(plugin_label);
    }
    
    /* Associate the whole metrics value with their associated host */
    json_object_object_add(res, host_label, plugin);

    /* Json allocation because of the previous function which
    delete the json pointer */
    plugin = json_object_new_object();

    /* Host label desallocation */
    sfree(host_label);
  }

  return res;
}
