cmake_minimum_required(VERSION 3.10)
project(HelloGNSS C)

add_subdirectory(lib/RTKLIB)
add_executable(main src/main.c)

target_include_directories(main PRIVATE 
    ${CMAKE_SOURCE_DIR}/lib/RTKLIB/src
)
target_link_libraries(main PRIVATE rtklib)

if(WIN32)
    target_link_libraries(main PRIVATE winmm ws2_32)
endif()

add_dependencies(main rtklib_static)