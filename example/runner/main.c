#include "../../src/libelf.h"
#include "../api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#ifdef _WIN32
#include <windows.h>

void* vmem_alloc(size_t sz) {
	return VirtualAlloc(NULL, sz, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}
void vmem_free(void* mem) {
	VirtualFree(mem, 0, MEM_RELEASE);
}
#else
#include <malloc.h>
#include <sys/mman.h>

void* vmem_alloc_once(size_t sz) {	
	void* ptr = memalign(0x200000, sz);    
	if(ptr){
		if(mprotect(ptr, 0x200000, PROT_READ|PROT_WRITE|PROT_EXEC)<0){
			//printf("mprotect error %p\n", ptr);
			free(ptr);
			return NULL;
		}
	}
    return ptr;
}

void* vmem_alloc(size_t sz){
	int i;
	void* ret;
	for(i=0;i<50;i++){
		ret = vmem_alloc_once(sz);	
		if(ret)return ret;
	}
	return NULL;
}

void vmem_free(void* mem) {
	free(mem);
}
#endif


static size_t file_size(const char* name){
    size_t ret;
    FILE* fd = fopen(name, "rb");
    if(!fd)return 0;
    fseek(fd, 0, SEEK_END);
    ret = ftell(fd);
    fclose(fd);
    return ret;
}


static int file_load(const char* name, void* buf, size_t* buflen){
    FILE* fd = fopen(name, "rb");
    int ret = -1;
    int len;
    if(fd){
        len = (int)fread(buf, 1, *buflen, fd); 
        if(len>0){
            *buflen = (size_t)len; 
            ret = 0;           
        }
        fclose(fd);
    } 
    return ret;   
}


static elf_module* load_module(const char* name){
    uint8_t* vmem = NULL;    
    uint8_t* mem = NULL;
    size_t vmemsz;
    size_t memsz;
    int ret;
    elf_module* result;

    vmemsz = file_size(name);
    if(!vmemsz)goto error;
    vmem = (uint8_t*)vmem_alloc(vmemsz);
    if(!vmem)goto error;
    ret = file_load(name, vmem, &vmemsz);
    if(ret)goto error;
    ret = elf_check(vmem, vmemsz);
    //if(ret)goto error;
    memsz = elf_memory_size(vmem, vmemsz);
	printf("elf require size: %d\n", (int)memsz);
    if(!memsz)goto error;
    mem = (uint8_t*)vmem_alloc(memsz);
    if(!mem)goto error;
    result = elf_module_init(mem, memsz, vmem, vmemsz);
    if(!result)goto error;
    return result;
    
error:   
    if(vmem)vmem_free(vmem);
    if(mem)vmem_free(mem);
    return NULL;
}


extern const runtime_api api_;

int main(){
    elf_module* mod;

    mod = load_module("app.so");
    if(!mod){
        printf("load error\n");
        return 1;
    }
	printf("load ok\n");
    app_entry_func func = elf_module_sym(mod, "app");
    if(!func){
        printf("sym not found\n");
        return 1;
    }

    return func(&api_);
}



