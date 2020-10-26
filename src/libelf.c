#include "libelf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if 0
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

#if __SIZEOF_POINTER__ == 4  //32位

#define ELF_HOST_CLASS ELF_CLASS_32
typedef elf32_header elf_header;
typedef elf32_program_header elf_program_header;
typedef elf32_section_header elf_section_header;
typedef elf32_symbol elf_symbol;

#else //64位
#define ELF_HOST_CLASS ELF_CLASS_64
typedef elf64_header elf_header;
typedef elf64_program_header elf_program_header;
typedef elf64_section_header elf_section_header;
typedef elf64_symbol elf_symbol;
#endif

#if defined(__x86_64__) || defined(_WIN64)
#define ELF_HOST_MACHINE ELF_MACHINE_X86_64
#elif defined(__i686__) || defined(__i586__) || defined(_WIN32)
#define ELF_HOST_MACHINE ELF_MACHINE_X86
#elif defined(__arm__)
#define ELF_HOST_MACHINE ELF_MACHINE_ARM
#elif defined(__aarch64__)
#define ELF_HOST_MACHINE ELF_MACHINE_AARCH64
#elif defined(__mips__)
#define ELF_HOST_MACHINE 8
#else
#define ELF_HOST_MACHINE 0 
#endif

struct elf_module{
    size_t vstart; //最小虚拟地址
    uint8_t* elfdata;
    void* entry;
    elf_symbol* sym;
    size_t symnum;
    char* strtab;
    size_t strtabsz;
};


//验证elf格式是否正确，是否可加载， 正常返回0， 异常返回非0
int elf_check(const uint8_t * d, size_t elflen){
    elf_header* h = (elf_header*)d;
    uint32_t endian_test = 0x02000001;
    uint8_t* plendian = (uint8_t*)&endian_test;
    if(elflen <= sizeof(elf_header))return ELF_CHECK_ERR_LENGTH;
    if(h->magic[0] != ELF_MAGIC_0 
        || h->magic[1] != ELF_MAGIC_1 
        || h->magic[2] != ELF_MAGIC_2 
        || h->magic[3] != ELF_MAGIC_3 
    )return ELF_CHECK_ERR_MAGIC;
    if(h->cls != ELF_HOST_CLASS)return ELF_CHECK_ERR_CLASS;
    if(h->endian != *plendian)return ELF_CHECK_ERR_ENDIAN;
    
    if(h->type != ELF_TYPE_DYN && h->type != ELF_TYPE_EXEC)return ELF_CHECK_ERR_TYPE;
    if(ELF_HOST_MACHINE && h->machine != ELF_HOST_MACHINE)return ELF_CHECK_ERR_MACHINE;
    return 0;
}


//计算需额外分配内存大小
size_t elf_memory_size(const uint8_t * elfdata, size_t elflen){
    elf_header* h = (elf_header*)elfdata;
    elf_program_header* ph;
    size_t i;
    size_t vstart = (size_t)-1; //最小虚拟内存地址 
    size_t vend = 0;  //最大虚拟内存地址 
    size_t curvend;
    ph = (elf_program_header*)(elfdata + h->phoff);
    for(i = 0; i < h->phnum; i++){
        if(ph->type == ELF_PROGRAM_TYPE_LOAD){
            if(ph->vaddr < vstart) vstart = ph->vaddr /* & ALIGN_VADDR*/;
            curvend = ph->vaddr + ph->memsz;
            if(curvend > vend) vend = curvend;
        }
        ph ++; 
    }
    DEBUG_PRINTF("calced vstart: %p vend: %p\n", (void*)vstart,  (void*)vend);
    if(vstart > vend)return 0;    
    return vend - vstart + sizeof(elf_module);
}


//初始化elf模块
elf_module* elf_module_init(uint8_t* mem, size_t memsz, const uint8_t * elfdata, size_t elflen){
    elf_header* h = (elf_header*)elfdata;
    elf_program_header* ph;
    elf_program_header* psh = NULL;
    elf_section_header* sh;
    elf_section_header* basesh;
    size_t i;     
    elf_module* elf = (elf_module*)mem;
    uint8_t* ptr = mem + sizeof(elf_module);
    uint8_t* newbase = ptr;
    size_t vstart = (size_t)-1;
    size_t vend = 0;
    size_t curvend;
    size_t sz;
    uint8_t* psrc;
	elf_symbol* sym = NULL;
	
    //计算虚拟地址
    vstart = (size_t)-1;
    ph = (elf_program_header*)(elfdata + h->phoff);
    for(i = 0; i < h->phnum; i++){
        if(ph->type == ELF_PROGRAM_TYPE_LOAD){
            if(!psh)psh = ph;
            if(ph->vaddr < vstart) vstart = ph->vaddr /*& ALIGN_VADDR*/;
            curvend = ph->vaddr + ph->memsz;
            if(curvend > vend) vend = curvend;            
        }
        ph ++; 
    }
    if(vstart > vend)return NULL;
    if(vend - vstart > memsz + sizeof(elf_module))return NULL;
    elf->vstart = vstart;

    //入口点位置计算
    elf->elfdata = newbase;
    elf->entry = ptr + h->entry - vstart;    
    DEBUG_PRINTF("module: %p size: %d\n", elf, (int)memsz);
    DEBUG_PRINTF("  elf: %p\n", elf->elfdata);

    //符号表位置计算
    basesh = (elf_section_header*)(elfdata + h->shoff);
    sh = basesh;
    elf->sym = NULL;
    for(i = 0; i < h->shnum; i++){
        if(sh->type == ELF_SECTION_TYPE_DYNSYM){
            elf->sym = (elf_symbol*)(newbase + sh->offset - psh->offset);
            elf->symnum = sh->size / sizeof(elf_symbol);
            elf->strtab = (char*)newbase + basesh[sh->link].offset - psh->offset;
            elf->strtabsz = basesh[sh->link].size;

            DEBUG_PRINTF("  dynsym: %p -> %p\n", (void*)sh->offset, elf->sym);
            DEBUG_PRINTF("  strtab: %p -> %p\n", (void*)basesh[sh->link].offset, elf->strtab);
            break;
        }
        sh ++; 
    }
    if(!elf->sym)return NULL;

    //执行段拷贝到内存
    ph = (elf_program_header*)(elfdata + h->phoff);
    for(i = 0; i < h->phnum; i++){
        if(ph->type == ELF_PROGRAM_TYPE_LOAD){
            ptr = newbase + ph->vaddr - elf->vstart;
            psrc = (uint8_t*)elfdata + ph->offset;
            sz = ph->filesz;
            
            DEBUG_PRINTF("  load: %p-%p -> %p-%p size: %d\n", (void*)(uintptr_t)(psrc - elfdata), 
                (void*)(uintptr_t)(psrc - elfdata + sz), ptr, ptr + sz, (int)sz);
            memcpy(ptr, psrc, sz);
            ptr += sz;
            sz = ph->memsz - ph->filesz;
            if(sz){
                DEBUG_PRINTF("  bzero: %p-%p size: %d\n", ptr, ptr + sz, (int)sz);
                memset(ptr, 0, sz);  
            }          
        }
        ph ++; 
    }

    
    DEBUG_PRINTF("  vstart: %07X\n", (int)elf->vstart);    
    DEBUG_PRINTF("  size: %07X\n", (int)(vend - vstart));
    DEBUG_PRINTF("  entry: %p\n", elf->entry);
    

    DEBUG_PRINTF("  symbols:\n");
    sym = elf->sym;    
    for(i = 0; i < elf->symnum; i++ ){
        DEBUG_PRINTF("  %p -> %p  %s\n", (void*)sym->value,  elf->elfdata + sym->value - elf->vstart , elf->strtab + sym->name);        
        sym ++;
    }
    return elf;
}


//获取入口函数地址
void *elf_module_entry(elf_module* m){
    return m->entry;
}


//获取函数地址
void *elf_module_sym(elf_module* m, const char* name){
    size_t i;
    elf_symbol* sym = NULL;

    sym = m->sym;    
    for(i = 0; i < m->symnum; i++ ){
        if(sym->name && sym->name < m->strtabsz && strcmp(m->strtab + sym->name, name) == 0){
            return m->elfdata + sym->value - m->vstart;
        }
        sym ++;
    } 

    return NULL;   
}
























