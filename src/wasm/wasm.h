#pragma once

typedef struct
{
    char magic_number[4];
    char version[4];
} wasm_module_header;

typedef struct
{
    wasm_module_header header;
} wasm_module;

wasm_module *wasm_load_module_from_file(const char *file_name);