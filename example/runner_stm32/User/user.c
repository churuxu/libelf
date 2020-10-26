#include "stm32f1xx_hal.h"
#include "../../api.h"
#include "../../../src/libelf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define LOAD_SZ (1024*8)
static uint8_t runmem_[LOAD_SZ];
extern const runtime_api api_;

void blink_led(){

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
}


void user_app(){
    
    elf_module* mod;
    blink_led();
    uint8_t* elfptr = (uint8_t*)(uintptr_t)0x08004000;
    
    mod = elf_module_init(runmem_, LOAD_SZ, elfptr, LOAD_SZ);
    if(!mod){
        return;
    }

    app_entry_func func = elf_module_sym(mod, "app");
    if(!func){
        //printf("sym not found\n");
        return;
    } 

    func(&api_);
}
