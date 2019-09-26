cmake_minimum_required(VERSION 3.10)

project(wasm LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(wasm
    src/main.c
    src/wasm/wasm.c
)

include_directories("src")