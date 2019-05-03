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

/* Success messages */
#define SUCCESS_RESET_CHAR "removed"
#define SUCCESS_INIT_CHAR "loaded"
#define SUCCESS_CONFIG_CHAR "configuration applied"

/* Error messages */
#define ERR_PLUGIN_IS_STORED_CHAR "plugin already stored"
#define ERR_PLUGIN_UNKOWN "unknown plugin"
#define ERR_RESET_CHAR "reset failure"
#define ERR_READ_CHAR "read failure"
#define ERR_CONFIG_CHAR "configuration failure"
#define ERR_CONFIG_UNKNOWN_CHAR "configuration unknown"
#define ERR_INIT_CHAR "initialization failure"
#define ERR_PLUGIN_STORED_CHAR "plugin not registered"
#define ERR_PLUGIN_NULL_CHAR "plugin list NULL"
#define ERR_PLUGIN_EMPTY_CHAR "plugin list empty"
#define ERR_LIB_CHAR "plugin library closed"
#define ERR_ARG_CHAR "argument(s) missconfigured"

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
#define MEMORY_ABSOLUTE_CHAR "absolute"
#define MEMORY_PERCENT_CHAR "percent"
#define MEMORY_ALL_CHAR "all"

/* Plugin processes */
#define PROCESSES_CHAR "processes"
#define PROCESSES_FILE_CHAR "file"
#define PROCESSES_MEMORY_CHAR "memory"
#define PROCESSES_CONTEXT_CHAR "context"

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
