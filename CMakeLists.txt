cmake_minimum_required(VERSION 3.10)

project(wasm LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(TESTING "build in test mode" 0)

add_executable(wasm
    src/wasm/wasm.c
    src/wasm/wasm_reader.c
    src/wasm/wasm_common.c
    src/wasm/wasm_vec.c
)

# different main()s for testing and release 
if (TESTING) 
    target_sources(wasm PUBLIC tests/tests.c)
    include_directories("tests")
else()
    target_sources(wasm PUBLIC src/main.c)
endif()

include_directories("src")

# add compiler specific options
if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(wasm PRIVATE "-Wall" "-Wextra" "-Werror=return-type")
endif()
