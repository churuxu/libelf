#pragma once

#include <stdlib.h>
#include <stdint.h>

#if defined(_WIN64) && defined(__GNUC__)
#define ELF_API __attribute__((sysv_abi)) 
#else
#define ELF_API 
#endif

typedef struct runtime_api{
	ELF_API void (*println)(const char* msg);
	ELF_API void (*delay)(int ms);
	ELF_API void* (*device_get)(const char* name);
	ELF_API void (*gpio_set)(void* dev, int v);
}runtime_api;


typedef ELF_API int (*app_entry_func)(const runtime_api* api);
