add_executable(libmdnscpp-example)

target_sources(libmdnscpp-example PRIVATE
  main.cpp
)

target_link_libraries(libmdnscpp-example PRIVATE libmdnscpp)
