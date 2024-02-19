add_executable(event_sample main.cpp)
target_link_libraries(event_sample PRIVATE libevent::core)