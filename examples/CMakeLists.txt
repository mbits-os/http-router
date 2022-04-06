if (HTTP_ROUTER_EXAMPLES)

set(EXAMPLES_FOLDER "${HTTP_ROUTER_FOLDER}examples")

find_package(Threads REQUIRED)

add_executable(simple-server simple-server.cc)
target_link_libraries(simple-server PRIVATE http-router http-router-filters Threads::Threads)

set_target_properties(simple-server PROPERTIES
  FOLDER ${EXAMPLES_FOLDER}
)

endif()
