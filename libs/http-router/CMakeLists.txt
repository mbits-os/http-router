# Copyright (c) 2022 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

if (HTTP_ROUTER_TESTING)
  set(COVERALLS_PREFIX HTTP_ROUTER_)
  list(APPEND HTTP_ROUTER_COVERALLS_DIRS
    include/router
    src
  )

  include(${CMAKE_SOURCE_DIR}/tools/coveralls/Coveralls.cmake)
endif()

configure_file(src/version.hh.in gen/include/http-router/version.hh @ONLY)

set(SRCS
  include/http-router/beast/patched/basic_file_body.hh
  include/http-router/beast/patched/file_body_win32.hh
  include/http-router/beast/patched/file_body.hh
  include/http-router/delegate.hh
  include/http-router/filters/base.hh
  include/http-router/path_compiler.hh
  include/http-router/range.hh
  include/http-router/request.hh
  include/http-router/response.hh
  include/http-router/route.hh
  include/http-router/router.hh
  include/http-router/server.hh
  include/http-router/shared_roots.hh
  include/http-router/uri.hh

  src/path_compiler.cc
  src/range.cc
  src/response.cc
  src/router.cc
  src/server/beast.cc
  src/server/beast.hh
  src/server/http_session.cc
  src/server/http_session.hh
  src/server/server.cc
  src/shared_roots.cc
  src/uri.cc
  src/version.cc
  src/version.hh.in
)

set(GEN_SRCS
  "${CMAKE_CURRENT_BINARY_DIR}/gen/include/http-router/version.hh"
)


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX code FILES ${SRCS})
source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} FILES ${GEN_SRCS})

add_library(http-router STATIC ${SRCS} ${GEN_SRCS})

target_compile_options(http-router PRIVATE ${HTTP_ROUTER_ADDITIONAL_WALL_FLAGS})
target_compile_options(http-router PUBLIC -DBOOST_BEAST_USE_STD_STRING_VIEW)
target_compile_features(http-router PRIVATE cxx_std_17)
target_include_directories(http-router
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/gen/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(http-router PUBLIC fmt::fmt Boost::headers)
set_target_properties(http-router PROPERTIES
  VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if (NOT HTTP_ROUTER_STANDALONE)
set_target_properties(http-router PROPERTIES
  FOLDER ${HTTP_ROUTER_FOLDER}
)
endif()

##################################################################
##  INSTALL
##################################################################

if (HTTP_ROUTER_INSTALL)
  install(TARGETS http-router)
  install(DIRECTORY include/http-router DESTINATION include)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gen/include/http-router DESTINATION include)
endif()
