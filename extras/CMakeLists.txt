# Copyright (c) 2022 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

add_library(http-router-filters INTERFACE)

if (HTTP_ROUTER_WITH_STATIC_FILES)
  add_subdirectory(static_files)
  target_link_libraries(http-router-filters INTERFACE static_files-filter)
endif()

if (HTTP_ROUTER_WITH_CORS)
  add_subdirectory(cors)
  target_link_libraries(http-router-filters INTERFACE cors-filter)
endif()

if (HTTP_ROUTER_WITH_LOGGER)
  add_subdirectory(logger)
  target_link_libraries(http-router-filters INTERFACE logger-filter)
endif()

if (HTTP_ROUTER_WITH_HEADERS)
  add_subdirectory(headers)
  target_link_libraries(http-router-filters INTERFACE headers-filter)
endif()
