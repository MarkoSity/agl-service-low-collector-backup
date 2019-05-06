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
#include "../userdata.h"

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

json_object *json_agregation(json_object *obj1, json_object *obj2)
{
  json_object *temp1;
  json_object *temp2;

  temp1 = wrap_json_clone(obj1);
  temp2 = wrap_json_clone(obj2);

  wrap_json_object_add(temp1, temp2);

  return temp1;
}

void json_free(json_object *obj)
{
  int idx;

  idx = json_object_put(obj);
  
  while(idx != 1)
    idx = json_object_put(obj);
}

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                          WRITE JSON CALLBACK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

json_object *write_json(metrics_list_t *metrics_list)
{
  /* Variables declaration */
  /* Json */
  json_object *res;
  json_object *value;
  json_object *type;
  json_object *type_instance;
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
  printf("malloc res\n");
  res = json_object_new_object();
  if(!res)
    return json_object_new_string(ERR_ALLOC_CHAR);

  /* Ensure the metrics list is not NULL */
  if(!metrics_list)
    return json_object_new_string(ERR_METRICS_NULL_CHAR);

  /* Ensure the metrics list is not empty */
  if(!metrics_list->size)
    return json_object_new_string(ERR_METRICS_EMPTY_CHAR);

  /* While index is not out of bond */
  while(index != metrics_list->size)
  {
    /* Retrieve and store the metrics host label */
    printf("malloc host label\n");
    host_label = (char *)malloc(strlen(metrics_list->metrics[index].host) + 1);
    if(!host_label)
      return json_object_new_string(ERR_ALLOC_CHAR);

    strcpy(host_label, metrics_list->metrics[index].host);
    printf("host : %s\n", host_label);

    /* Json allocation */
    printf("malloc plugin json\n");
    plugin = json_object_new_object();
    if(!plugin)
      return json_object_new_string(ERR_ALLOC_CHAR);

    /* While the metrics host do not change */
    while(!strncmp(metrics_list->metrics[index].host, host_label,
         max_size(strlen(metrics_list->metrics[index].host), strlen(host_label))))
    {
      /* Retrieve and store the metrics plugin */
      printf("malloc plugin label\n");
      plugin_label = (char *)malloc(strlen(metrics_list->metrics[index].plugin) + 1);
      if(!plugin_label)
        return json_object_new_string(ERR_ALLOC_CHAR);

      strcpy(plugin_label, metrics_list->metrics[index].plugin);
      printf("plugin : %s\n", plugin_label);

      /* Json allocation */
      printf("malloc plugin without instance json\n");
      plugin_without_instance = json_object_new_object();
      if(!plugin_without_instance)
        return json_object_new_string(ERR_ALLOC_CHAR);

      printf("malloc plugin with instance json\n");
      plugin_with_instance = json_object_new_object();
      if(!plugin_with_instance)
        return json_object_new_string(ERR_ALLOC_CHAR);

      printf("malloc plugin tmp json\n");
      plugin_tmp = json_object_new_object();
      if(!plugin_tmp)
        return json_object_new_string(ERR_ALLOC_CHAR);

      /* While the metrics plugin do not change */
      while(!strncmp(metrics_list->metrics[index].host, host_label,
           max_size(strlen(metrics_list->metrics[index].host), strlen(host_label))) &&
           !strncmp(metrics_list->metrics[index].plugin, plugin_label,
           max_size(strlen(metrics_list->metrics[index].plugin), strlen(plugin_label))))
      {
        /* Retrieve and store the metrics plugin instance label */
        printf("malloc plugin instance label\n");
        plugin_instance_label = (char *)malloc(strlen(metrics_list->metrics[index].plugin_instance) + 1);
        if(!plugin_instance_label)
          return json_object_new_string(ERR_ALLOC_CHAR);

        strcpy(plugin_instance_label, metrics_list->metrics[index].plugin_instance);
        printf("plugin instance : %s\n", plugin_instance_label);

        /* Json allocation */
        printf("malloc type json\n");
        type = json_object_new_object();
        if(!type)
          return json_object_new_string(ERR_ALLOC_CHAR);

        /* While the metrics plugin instance do not change */
        while(!strncmp(metrics_list->metrics[index].host, host_label,
              max_size(strlen(metrics_list->metrics[index].host), strlen(host_label))) &&
              !strncmp(metrics_list->metrics[index].plugin, plugin_label,
              max_size(strlen(metrics_list->metrics[index].plugin), strlen(plugin_label))) &&
              !strncmp(metrics_list->metrics[index].plugin_instance, plugin_instance_label,
              max_size(strlen(metrics_list->metrics[index].plugin_instance), strlen(plugin_instance_label))))
        {
          /* Retrieve and store the metrics type */
          printf("malloc type label\n");
          type_label = (char *)malloc(strlen(metrics_list->metrics[index].type) + 1);
          if(!type_label)
            return json_object_new_string(ERR_ALLOC_CHAR);

          strcpy(type_label, metrics_list->metrics[index].type);
          printf("type : %s\n", type_label);

          /* Json allocation */
          printf("malloc type instance json\n");
          type_instance = json_object_new_object();
          if(!type_instance)
            return json_object_new_string(ERR_ALLOC_CHAR);

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
            printf("malloc type instance label\n");
            type_instance_label = (char *)malloc(strlen(metrics_list->metrics[index].type_instance) + 1);
            if(!type_instance_label)
              return json_object_new_string(ERR_ALLOC_CHAR);

            strcpy(type_instance_label, metrics_list->metrics[index].type_instance);
            printf("type instance : %s\n", type_instance_label);

            /* Json allocation */
            printf("malloc value json\n");
            value = json_object_new_object();
            if(!value)
              return json_object_new_string(ERR_ALLOC_CHAR);

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
              /* Store the metrics in a json */
              value = json_object_new_double(metrics_list->metrics[index].values->gauge);
              if(!value)
                return json_object_new_string(ERR_ALLOC_CHAR);
                
              printf("value : %lf\n", metrics_list->metrics[index].values->gauge);

              /* Increment the index */
              index++;
            }

            /* If the previous metrics values have a type instance not empty */
            if(strncmp(type_instance_label, "", strlen(type_instance_label)))
              json_object_object_add(type_instance, type_instance_label, wrap_json_clone(value));

            else
              json_object_object_add(type_instance, plugin_label, wrap_json_clone(value));

            /* Json desallocation */
            /* printf("free value json\n");
            json_free(value); */
            
            /* Type instance label desallocation */
            printf("free type instance label\n");
            sfree(type_instance_label);
          }

          /* Associate the values with their type */
          json_object_object_add(type, type_label, wrap_json_clone(type_instance));

          /* Json_desallocation */
          printf("free type instance json\n");
          json_free(type_instance);

          /* Type label desallocation */
          printf("free type label\n");
          sfree(type_label);
        }

        /* If the current metrics do not have a plugin instance */
        if(!strncmp(plugin_instance_label, "", strlen(plugin_instance_label)))
          wrap_json_object_add(plugin_without_instance, type);

        /* Else the metrics has a plugin instance */
        else
          json_object_object_add(plugin_with_instance, plugin_instance_label, wrap_json_clone(type));

        /* Json desallocation */
        printf("free type json\n");
        json_free(type);

        /* Plugin instance label desallocation */
        printf("free plugin instance label\n");
        sfree(plugin_instance_label);
      }

      /* Gather the two previous json in a temporary json plugin */
      plugin_tmp = json_agregation(plugin_with_instance, plugin_without_instance);

      /* Associate the previous json with their associated key */
      json_object_object_add(plugin, plugin_label, wrap_json_clone(plugin_tmp));

      /* Json desallocation */
      printf("free plugin plugin without instance json\n");
      json_free(plugin_without_instance);
      printf("free plugin with instance json\n");
      json_free(plugin_with_instance);
      printf("free plugin tmp json\n");
      json_free(plugin_tmp);

      /* Plugin label desallocation */
      printf("free plugin label\n");
      sfree(plugin_label);
    }
    
    /* Associate the whole metrics value with their associated host */
    json_object_object_add(res, host_label, wrap_json_clone(plugin));

    /* Json desallocation */
    printf("free plugin json\n");
    json_free(plugin);

    /* Host label desallocation */
    printf("free host label\n");
    sfree(host_label);
    printf("fin host\n");
  }

  printf("fin while\n");
  return res;
}
