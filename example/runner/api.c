#include "../api.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(ms) usleep(1000*ms)	
#endif


typedef struct gpio_dev{
    char name[16];
}gpio_dev;


static ELF_API void println(const char* msg){
    printf("%s\n", msg);
}

static ELF_API void delay(int ms){
    Sleep(ms);
}

static ELF_API void* device_get(const char* name){
    gpio_dev* dev = malloc(sizeof(gpio_dev));
    int len = strlen(name);
    if(!dev || len > 15)return NULL;
    memcpy(dev->name, name, len + 1);
    return dev;
}

static ELF_API void gpio_set(void* pdev, int v){
    gpio_dev* dev = (gpio_dev*)pdev;
    printf(" gpio set %s %d\n", dev->name, v);
}

const runtime_api api_ = {
    println,
    delay,
    device_get,
    gpio_set
};


