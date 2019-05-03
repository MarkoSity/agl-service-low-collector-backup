###########################################################################
# Copyright (C) 2016-2019 IoT.bzh
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
###########################################################################

############################################################################
#                                   MACRO                                  #
############################################################################
MACRO(collectd_lib src_file label)

ADD_LIBRARY(${TARGET_NAME} SHARED ${src_file})

    TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME} PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/sources/src
      ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/daemon
    )

    # FIXME - Try to remove FP_LAYOUT_NEED_NOTHING and use config.h instead
    TARGET_COMPILE_DEFINITIONS(${TARGET_NAME}
        PUBLIC -DFP_LAYOUT_NEED_NOTHING
        PUBLIC -DHAVE_CONFIG_H
    )

    if(NOT ${src_file} STREQUAL "collectd_glue.c")
      target_compile_options(${TARGET_NAME}
        PUBLIC -Wno-conversion -Wno-int-conversion
        PUBLIC -std=c99
    )
    endif()

  SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES
        LABELS label
        PREFIX ""
        SUFFIX ".so"
        OUTPUT_NAME ${TARGET_NAME}
  )

ENDMACRO()

############################################################################
#                             COLLECTD GLUE                                #
############################################################################
PROJECT_TARGET_ADD(collectd_glue)
collectd_lib(collectd_glue.c COLLECTD_GLUE)

############################################################################
#                             CPU plugin                                   #
############################################################################
PROJECT_TARGET_ADD(cpu)
collectd_lib(${CMAKE_CURRENT_SOURCE_DIR}/sources/src/cpu.c PLUGIN)

############################################################################
#                             MEMORY plugin                                #
############################################################################
PROJECT_TARGET_ADD(memory)
collectd_lib(${CMAKE_CURRENT_SOURCE_DIR}/sources/src/memory.c PLUGIN)

############################################################################
#                           PROCESSES plugin                               #
############################################################################
PROJECT_TARGET_ADD(processes)
collectd_lib(${CMAKE_CURRENT_SOURCE_DIR}/sources/src/processes.c PLUGIN)
