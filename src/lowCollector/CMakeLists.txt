###########################################################################
# Copyright 2015, 2016, 2017 IoT.bzh
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

# Add target to project dependency list
PROJECT_TARGET_ADD(lowCollector)

    set(SRC_LOWCOLLECTOR
    "lowCollector.c"
	  )

	set(SRC_API
    "api.c"
  )
  
  set(SRC_API_CPU
    "api_cpu.c"
	)

	# Define project Targets
	ADD_LIBRARY(${TARGET_NAME} MODULE ${SRC_LOWCOLLECTOR};
                                    ${SRC_API};
                                    ${SRC_API_CPU})

  TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../collectd
                                                   ${CMAKE_CURRENT_SOURCE_DIR}/../collectd/sources/src
                                                   ${CMAKE_CURRENT_SOURCE_DIR}/../collectd/sources/src/daemon
  )

  # FIXME - Try to remove FP_LAYOUT_NEED_NOTHING and use config.h instead
  TARGET_COMPILE_DEFINITIONS(${TARGET_NAME} PUBLIC -DFP_LAYOUT_NEED_NOTHING
                                            PUBLIC -DHAVE_CONFIG_H
  )


	# Binder exposes a unique public entry point
	SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES PREFIX "afb-"
		                                              LABELS "BINDING"
		                                              OUTPUT_NAME ${TARGET_NAME}
	)
