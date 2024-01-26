cmake_minimum_required(VERSION 3.26)
project(libevent_exercise)

set(CMAKE_CXX_STANDARD 11)

INCLUDE_DIRECTORIES(
        /usr/local/include/
        /usr/include
)

link_directories(
        /usr/local/lib
        /usr/lib
)

find_package(Libevent REQUIRED)

add_subdirectory(basic_helloworld)
add_subdirectory(pipe)
add_subdirectory(simple_http)
add_subdirectory(cpp_style)


