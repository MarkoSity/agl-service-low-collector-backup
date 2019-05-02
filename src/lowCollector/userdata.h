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
                            DEFINE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#ifndef USERDATA_H
#define USERDATA_H

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            GLOBAL CHAR
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* Useful function */
#define MAX_SIZE_CHAR "max_size"
#define INDEX_PLUGIN_LABEL_CHAR "index_plugin_label"

/* Global variable */
#define PLUGIN_LIST_CHAR "Plugin_list"
#define METRICS_LIST_CHAR "Metrics_list"

/* Function related to the global variable */
#define PLUGIN_DEINIT_CHAR "plugin_deinit"
#define METRICS_DEINIT_CHAR "metrics_deinit"

/* Module register function */
#define MODULE_REGISTER_CHAR "module_register"

/* Plugin */
#define PLUGIN_CHAR "plugin"

/* Plugin cpu */
#define CPU_CHAR "cpu"
#define CPU_MEAN_CHAR "mean"
#define CPU_MEAN_STATE_CHAR "mean_state"
#define CPU_MEAN_CPU_CHAR "mean_cpu"
#define CPU_JIFFIES_STATE_CPU_CHAR "jiffies_state_cpu"
#define CPU_PERCENT_STATE_CPU_CHAR "percent_state_cpu"
#define CPU_NUMBER_CHAR "number"

/* Plugin memory */
#define MEMORY_CHAR "memory"

/* Plugin processes */
#define PROCESSES_CHAR "processes"

/* Plugin processes */
#define PROCESSES_CHAR "processes"

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            USERDATA STRUCTURE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

typedef struct userdata_s userdata_t;
struct userdata_s
{
  void *handle_collectd;
  void *handle_cpu;
  void *handle_memory;
  void *handle_processes;
};

#endif /* USERDATA_H */
