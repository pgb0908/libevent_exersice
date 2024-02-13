
add_executable(echo main.cpp)

target_link_libraries(echo PRIVATE libevent::core)