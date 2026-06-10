cmake_minimum_required(VERSION 3.20)

project(formatting C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(formatting
formatter.c
)

target_include_directories(formatting PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (MSVC)
target_compile_options(formatting PRIVATE /W4 /Zi)
endif()