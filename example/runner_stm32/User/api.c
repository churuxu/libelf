#include "stm32f1xx_hal.h"
#include "../../api.h"
#include <string.h>

extern UART_HandleTypeDef huart1;

static ELF_API void println(const char* msg){
    return;
    HAL_UART_Transmit((UART_HandleTypeDef*)&huart1, (uint8_t*)msg, strlen(msg), 3000);
    HAL_UART_Transmit((UART_HandleTypeDef*)&huart1, (uint8_t*)"\n", 1, 1000);
}

static ELF_API void sleep(int ms){
    HAL_Delay(ms);
}

static ELF_API void* device_get(const char* name){
    if(strcmp(name, "LED0") == 0){
        return (void*)(uintptr_t)1;
    }
    return NULL;
}

static ELF_API void gpio_set(void* pdev, int v){
    if((uintptr_t)pdev == 1){
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, v);
    }
}

const runtime_api api_ = {
    println,
    sleep,
    device_get,
    gpio_set
};


