#include "../../src/libelf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PTR32_FIX_LEN 10
#define PTR64_FIX_LEN 17

static void print_bytes(const void* data, int len){    
    uint8_t* ptr = (uint8_t* )data;
    for(int i=0; i<len; i++){        
        printf("%02X", ptr[i]);    
    }
}

static void print_fixed_string(const char* str, int len){
    int i,remain;
    int slen = printf("%s", str);
    remain = len - slen;
    for(i = 0; i < remain ; i++){
        putchar(' ');
    }
}

static void print_fixed_number(unsigned int val, int len){
    int i,remain;
    int slen = printf("%u", val);
    remain = len - slen;
    for(i = 0; i < remain ; i++){
        putchar(' ');
    }
}

static void print_fixed_number64(uint64_t val, int len){
    const char* base = "0123456789";    
    char temp[64];
    uint8_t c;
    char* ptr = &temp[62];
    temp[63] = 0;
    while(1){
        c = val % 10;
        *ptr = base[c];
        ptr --;
        val = val / 10;
        if(!val)break;
    }
    print_fixed_string(ptr + 1, len);
}

static void print_fixed_hex_number(unsigned int val, int len){
    int i,remain;
    int slen = printf("%08X", val);
    remain = len - slen;
    for(i = 0; i < remain ; i++){
        putchar(' ');
    }
}
static void print_fixed_hex_number64(uint64_t val, int len){
    const char* base = "0123456789ABCDEF";    
    char temp[64];
    int i;
    uint8_t c;
    char* ptr = &temp[62];
    temp[63] = 0;
    for(i=0;i<16;i++){
        c = val % 16;
        *ptr = base[c];
        ptr --;
        val = val / 16;        
    }    
    print_fixed_string(ptr + 1, len);
}


static void print_elf_type(uint16_t type){
    const char* s;
    switch (type){
    case ELF_TYPE_REL: s =  "REL"; break;
    case ELF_TYPE_EXEC: s =  "EXEC"; break;
    case ELF_TYPE_DYN: s =  "DYN"; break;
    default: s = "";        
    }
    printf("0x%X %s", (int)type, s);
}

static void print_elf_machine(uint16_t type){
    const char* s;
    switch (type){
    case ELF_MACHINE_X86: s =  "x86"; break;
    case ELF_MACHINE_X86_64: s =  "x86_64"; break;
    case ELF_MACHINE_ARM: s =  "arm"; break;
    case ELF_MACHINE_AARCH64: s =  "aarch64"; break;
    default: s = "";        
    }
    printf("%d %s", (int)type, s);
}
static void print_elf_class(uint8_t type){
    const char* s;
    switch (type){
    case ELF_CLASS_32: s =  "32-bit"; break;
    case ELF_CLASS_64: s =  "64-bit";  break;   
    default: s = "Unknown";        
    }
    printf("%d %s", (int)type, s);
}
static void print_elf_endian(uint8_t type){
    const char* s;
    switch (type){
    case ELF_ENDIAN_LITTLE: s =  "little-endian"; break;
    case ELF_ENDIAN_BIG: s =  "big-endian"; break;    
    default: s = "unknown";        
    }
    printf("%d %s", (int)type, s);
}

static void print_elf_abi(uint8_t type){       
    printf("%d", (int)type);
}

static void print_elf_program_type(uint32_t type, int fixlen){    
    const char* s;    
    switch (type){
    case ELF_PROGRAM_TYPE_LOAD: s =  "LOAD"; break;
    case ELF_PROGRAM_TYPE_DYNAMIC: s =  "DYNAMIC"; break;
    case ELF_PROGRAM_TYPE_INTERP: s =  "INTERP"; break;
    case ELF_PROGRAM_TYPE_NOTE: s =  "NOTE"; break;
    case ELF_PROGRAM_TYPE_SHLIB: s =  "SHLIB"; break;
    case ELF_PROGRAM_TYPE_PHDR: s =  "PHDR"; break;
    case ELF_PROGRAM_TYPE_TLS: s =  "TLS"; break;
    default: 
        print_fixed_hex_number(type, fixlen);
        return;
    }
    print_fixed_string(s, fixlen);
}

static void print_elf_program_flags(uint32_t flags, int fixlen){    
    char s[16];
    s[0] = (flags & ELF_PROGRAM_FLAG_R)?'R':' ';
    s[1] = (flags & ELF_PROGRAM_FLAG_W)?'W':' ';
    s[2] = (flags & ELF_PROGRAM_FLAG_X)?'X':' ';
    s[3] = 0;
    print_fixed_string(s, fixlen);
}

static void print_elf_section_type(uint32_t type, int fixlen){  
    const char* s;
    switch (type){
    case ELF_SECTION_TYPE_PROGBITS: s =  "PROGBITS"; break;
    case ELF_SECTION_TYPE_SYMTAB: s =  "SYMTAB"; break;
    case ELF_SECTION_TYPE_STRTAB: s =  "STRTAB"; break;
    case ELF_SECTION_TYPE_RELA: s =  "RELA"; break;
    case ELF_SECTION_TYPE_HASH: s =  "HASH"; break;
    case ELF_SECTION_TYPE_DYNAMIC: s =  "DYNAMIC"; break;
    case ELF_SECTION_TYPE_NOTE: s =  "NOTE"; break;
    case ELF_SECTION_TYPE_NOBITS: s =  "NOBITS"; break;
    case ELF_SECTION_TYPE_REL: s =  "REL"; break;
    case ELF_SECTION_TYPE_SHLIB: s =  "SHLIB"; break;
    case ELF_SECTION_TYPE_DYNSYM: s =  "DYNSYM"; break;
    case ELF_SECTION_TYPE_INIT_ARRAY: s =  "INIT"; break;
    case ELF_SECTION_TYPE_FINI_ARRAY: s =  "FINI"; break;
    case ELF_SECTION_TYPE_PREINIT_ARRAY: s =  "PREINIT"; break;
    case ELF_SECTION_TYPE_GROUP: s =  "GROUP"; break;
    case ELF_SECTION_TYPE_SYMTAB_SHNDX: s =  "SHNDX"; break;
    default:
        print_fixed_hex_number(type, fixlen);
        return;    
    }
    print_fixed_string(s, fixlen);
}

static void print_elf_section_flags(uintptr_t flags, int fixlen){
    
    char temp[16];
    temp[0] = (flags & ELF_SECTION_FLAG_WRITE)?'W':' ';
    temp[1] = (flags & ELF_SECTION_FLAG_ALLOC)?'A':' ';
    temp[2] = (flags & ELF_SECTION_FLAG_EXECINSTR)?'X':' ';
    temp[3] = (flags & ELF_SECTION_FLAG_STRINGS)?'S':' ';
    temp[4] = (flags & ELF_SECTION_FLAG_INFO_LINK)?'I':' ';
    temp[5] = (flags & ELF_SECTION_FLAG_LINK_ORDER)?'L':' ';
    temp[6] = (flags & ELF_SECTION_FLAG_OS_NONCONFORMING)?'O':' ';
    temp[7] = (flags & ELF_SECTION_FLAG_GROUP)?'G':' ';
    temp[8] = (flags & ELF_SECTION_FLAG_TLS)?'T':' ';
    temp[9] = (flags & ELF_SECTION_FLAG_COMPRESSED)?'C':' ';
    temp[10] = 0;
    print_fixed_string(temp, fixlen);    
}
static void print_elf_symbol_bind(uint8_t type, int fixlen){
    const char* s;      
    switch (type){
    case ELF_SYMBOL_BIND_LOCAL: s =  "LOCAL"; break;
    case ELF_SYMBOL_BIND_GLOBAL: s =  "GLOBAL"; break;
    case ELF_SYMBOL_BIND_WEAK: s =  "WEAK"; break;
    default: 
        print_fixed_number(type, fixlen);
        return;
    }
    print_fixed_string(s, fixlen);
}

static void print_elf_symbol_type(uint8_t type, int fixlen){
    const char* s; 
    switch (type){
    case ELF_SYMBOL_TYPE_NOTYPE: s =  "NOTYPE"; break;
    case ELF_SYMBOL_TYPE_OBJECT: s =  "OBJECT"; break;
    case ELF_SYMBOL_TYPE_FUNC: s =  "FUNC"; break;
    case ELF_SYMBOL_TYPE_SECTION: s =  "SECTION"; break;
    case ELF_SYMBOL_TYPE_FILE: s =  "FILE"; break;
    case ELF_SYMBOL_TYPE_COMMON: s =  "COMMON"; break;
    case ELF_SYMBOL_TYPE_TLS: s =  "TLS"; break;
    default: 
        print_fixed_number(type, fixlen);
        return;
    }
    print_fixed_string(s, fixlen);
}

static void print_elf_symbol_visibility(uint8_t type, int fixlen){     
    const char* s; 
    switch (type){
    case ELF_SYMBOL_VISIBILITY_DEFAULT: s =  "DEFAULT"; break;
    case ELF_SYMBOL_VISIBILITY_INTERNAL: s =  "INTERNAL"; break;
    case ELF_SYMBOL_VISIBILITY_HIDDEN: s =  "HIDDEN"; break;
    case ELF_SYMBOL_VISIBILITY_PROTECTED: s =  "PROTECTED"; break;
    default:       
        print_fixed_number(type, fixlen);
        return;
    }
    print_fixed_string(s, fixlen);
}


static void print_elf32_header(const elf32_header* h){    
    printf("ELF Header:\n");
    printf("  Magic:    "); print_bytes(h->magic, 4); printf("\n");
    printf("  Class:    "); print_elf_class(h->cls); printf("\n");
    printf("  Endian:   "); print_elf_endian(h->endian); printf("\n");
    printf("  ABI:      "); print_elf_abi(h->abi); printf("\n");    
    printf("  Type:     "); print_elf_type(h->type); printf("\n");
    printf("  Machine:  "); print_elf_machine(h->machine);  printf("\n");
    printf("  Version:  %d\n", (int)h->version);
    printf("  Entry:    "); print_fixed_hex_number(h->entry, 18);  printf("\n");
    printf("  Flags:    "); print_fixed_hex_number(h->flags, 10);  printf("\n");
    printf("  Program Header:\n");
    printf("    Offset:        "); print_fixed_hex_number(h->phoff, 18);  printf("\n");
    printf("    EntSize:       "); print_fixed_number(h->phentsize, 8);  printf("\n");
    printf("    Count:         "); print_fixed_number(h->phnum, 8);  printf("\n");
    printf("  Section Header:\n");
    printf("    Offset:        "); print_fixed_hex_number(h->shoff, 18);  printf("\n");
    printf("    EntSize:       "); print_fixed_number(h->shentsize, 8);  printf("\n");
    printf("    Count:         "); print_fixed_number(h->shnum, 8);  printf("\n");
    printf("    String Index:  %d\n", (int)h->shstrndx); 
    printf("\n");
}

static void print_elf64_header(const elf64_header* h){    
    printf("ELF Header:\n");
    printf("  Magic:    "); print_bytes(h->magic, 4); printf("\n");
    printf("  Class:    "); print_elf_class(h->cls); printf("\n");
    printf("  Endian:   "); print_elf_endian(h->endian); printf("\n");
    printf("  ABI:      "); print_elf_abi(h->abi); printf("\n");    
    printf("  Type:     "); print_elf_type(h->type); printf("\n");
    printf("  Machine:  "); print_elf_machine(h->machine);  printf("\n");
    printf("  Version:  %d\n", (int)h->version);
    printf("  Entry:    "); print_fixed_hex_number64(h->entry, 18);  printf("\n");
    printf("  Flags:    "); print_fixed_hex_number(h->flags, 10);  printf("\n");
    printf("  Program Header:\n");
    printf("    Offset:        "); print_fixed_hex_number64(h->phoff, 18);  printf("\n");
    printf("    EntSize:       "); print_fixed_number(h->phentsize, 8);  printf("\n");
    printf("    Count:         "); print_fixed_number(h->phnum, 8);  printf("\n");
    printf("  Section Header:\n");
    printf("    Offset:        "); print_fixed_hex_number64(h->shoff, 18);  printf("\n");
    printf("    EntSize:       "); print_fixed_number(h->shentsize, 8);  printf("\n");
    printf("    Count:         "); print_fixed_number(h->shnum, 8);  printf("\n");
    printf("    String Index:  %d\n", (int)h->shstrndx); 
    printf("\n");
}


static void print_elf32_program_header(const elf32_program_header* ph, size_t count){
	uint32_t i;
    int ptrlen = PTR32_FIX_LEN;
    printf("Program Header Table:\n");
    print_fixed_string("#", 5);
    print_fixed_string("Type", 10);
    print_fixed_string("Flags", 10);
    print_fixed_string("Offset", ptrlen);
    print_fixed_string("VAddr", ptrlen);
    print_fixed_string("PAddr", ptrlen);
    print_fixed_string("FileSize", 10);
    print_fixed_string("MemSize", 10);
    print_fixed_string("Align", 10);
    printf("\n");
    for(i = 0; i< count; i++){
        print_fixed_number(i, 5);
        print_elf_program_type(ph->type, 10);
        print_elf_program_flags(ph->flags, 10);
        print_fixed_hex_number(ph->offset, ptrlen);
        print_fixed_hex_number(ph->vaddr, ptrlen);
        print_fixed_hex_number(ph->paddr, ptrlen);
        print_fixed_number(ph->filesz, 10);
        print_fixed_number(ph->memsz, 10);
        print_fixed_number(ph->align, 10);
        printf("\n");
        ph ++; 
    }
    printf("\n");     
}


static void print_elf64_program_header(const elf64_program_header* ph, size_t count){
	uint32_t i;
    int ptrlen = PTR64_FIX_LEN;
    printf("Program Header Table:\n");
    print_fixed_string("#", 5);
    print_fixed_string("Type", 10);
    print_fixed_string("Flags", 10);
    print_fixed_string("Offset", ptrlen);
    print_fixed_string("VAddr", ptrlen);
    print_fixed_string("PAddr", ptrlen);
    print_fixed_string("FileSize", 10);
    print_fixed_string("MemSize", 10);
    print_fixed_string("Align", 10);
    printf("\n");
    for(i = 0; i< count; i++){
        print_fixed_number(i, 5);
        print_elf_program_type(ph->type, 10);
        print_elf_program_flags(ph->flags, 10);
        print_fixed_hex_number64(ph->offset, ptrlen);
        print_fixed_hex_number64(ph->vaddr, ptrlen);
        print_fixed_hex_number64(ph->paddr, ptrlen);
        print_fixed_number64(ph->filesz, 10);
        print_fixed_number64(ph->memsz, 10);
        print_fixed_number64(ph->align, 10);
        printf("\n");
        ph ++; 
    }
    printf("\n");     
}


static void print_elf32_section_header(const elf32_section_header* sh, size_t count, const char* strtab){
    uint32_t i; 
    int ptrlen = PTR32_FIX_LEN;      
    printf("Section Header Table:\n");
    print_fixed_string("#", 5);
    print_fixed_string("Type", 10);
    print_fixed_string("Flags", 10);
    print_fixed_string("Addr", ptrlen);
    print_fixed_string("Offset", ptrlen);
    print_fixed_string("Size", 10);
    print_fixed_string("Link", 10);
    //print_fixed_string("Info", 10);
    print_fixed_string("Align", 10);
    //print_fixed_string("EntSize", 10);
    print_fixed_string("Name", 10);
    printf("\n");
    for(i = 0; i < count; i++){
        print_fixed_number(i, 5);
        print_elf_section_type(sh->type, 10);
        print_elf_section_flags(sh->flags, 10);
        print_fixed_hex_number(sh->addr, ptrlen);
        print_fixed_hex_number(sh->offset, ptrlen);
        print_fixed_number(sh->size, 10);
        print_fixed_number(sh->link, 10);
        //print_fixed_number(sh->info, 10);
        print_fixed_number(sh->align, 10);
        //print_fixed_number(sh->entsize, 10);
        printf("%s", strtab + sh->name);
        printf("\n");
        sh ++;           
    }
    printf("\n"); 
}


static void print_elf64_section_header(const elf64_section_header* sh, size_t count, const char* strtab){
    uint32_t i; 
    int ptrlen = PTR64_FIX_LEN;
    printf("Section Header Table:\n");
    print_fixed_string("#", 5);
    print_fixed_string("Type", 10);
    print_fixed_string("Flags", 10);
    print_fixed_string("Addr", ptrlen);
    print_fixed_string("Offset", ptrlen);
    print_fixed_string("Size", 10);
    print_fixed_string("Link", 10);
    //print_fixed_string("Info", 10);
    print_fixed_string("Align", 10);
    //print_fixed_string("EntSize", 10);
    print_fixed_string("Name", 10);
    printf("\n");
    for(i = 0; i < count; i++){
        print_fixed_number(i, 5);
        print_elf_section_type(sh->type, 10);
        print_elf_section_flags(sh->flags, 10);
        print_fixed_hex_number64(sh->addr, ptrlen);
        print_fixed_hex_number64(sh->offset, ptrlen);
        print_fixed_number64(sh->size, 10);
        print_fixed_number(sh->link, 10);
        //print_fixed_number(sh->info, 10);
        print_fixed_number64(sh->align, 10);
        //print_fixed_number64(sh->entsize, 10);
        printf("%s", strtab + sh->name);
        printf("\n");
        sh ++;           
    }
    printf("\n"); 
}


static void print_elf32_symbol_table(const elf32_symbol* sym, size_t count, const char* strtab){
	uint32_t i;
    int ptrlen = PTR32_FIX_LEN;
    printf("Symbol Table:\n");
    print_fixed_string("#", 5);
    print_fixed_string("Value", ptrlen);
    print_fixed_string("Size", 10);
    print_fixed_string("Type", 10);
    print_fixed_string("Bind", 10);
    print_fixed_string("Visible", 10);
    print_fixed_string("NIndex", 10);
    print_fixed_string("Name", 10);
    printf("\n");    
    for(i = 0; i< count; i++){
        print_fixed_number(i, 5);
        print_fixed_hex_number(sym->value, ptrlen);
        print_fixed_number(sym->size, 10);
        print_elf_symbol_type(sym->type, 10);
        print_elf_symbol_bind(sym->bind, 10);
        print_elf_symbol_visibility(sym->visibility, 10);
        print_fixed_number(sym->shndx, 10);
        printf("%s", strtab + sym->name);
        printf("\n");
        sym ++;
    }
    printf("\n"); 
}


static void print_elf64_symbol_table(const elf64_symbol* sym, size_t count, const char* strtab){
	uint32_t i;
    int ptrlen = PTR64_FIX_LEN;
    printf("Symbol Table:\n");
    print_fixed_string("#", 5);
    print_fixed_string("Value", ptrlen);
    print_fixed_string("Size", 10);
    print_fixed_string("Type", 10);
    print_fixed_string("Bind", 10);
    print_fixed_string("Visible", 10);
    print_fixed_string("NIndex", 10);
    print_fixed_string("Name", 10);   
    printf("\n"); 
    for(i = 0; i< count; i++){
        print_fixed_number(i, 5);
        print_fixed_hex_number64(sym->value, ptrlen);
        print_fixed_number64(sym->size, 10);
        print_elf_symbol_type(sym->type, 10);
        print_elf_symbol_bind(sym->bind, 10);
        print_elf_symbol_visibility(sym->visibility, 10);
        print_fixed_number(sym->shndx, 10);
        printf("%s", strtab + sym->name);
        printf("\n");
        sym ++;
    }
    printf("\n"); 
}

static void print_elf32(const uint8_t* data, size_t len){
	uint32_t i;
    elf32_section_header* cur_sh;    
    const char* shstrtab;
    elf32_header* h = (elf32_header*)data;
    elf32_section_header* sh = (elf32_section_header*)(data + h->shoff);
    elf32_program_header* ph = (elf32_program_header*)(data + h->phoff);
    shstrtab = (char*)data + sh[h->shstrndx].offset;

    print_elf32_header(h);
    print_elf32_program_header(ph, h->phnum);
    print_elf32_section_header(sh, h->shnum, shstrtab);

    cur_sh = sh;
    for(i = 0; i < h->shnum; i++){
        if(cur_sh->type == ELF_SECTION_TYPE_DYNSYM || cur_sh->type == ELF_SECTION_TYPE_SYMTAB){
            printf("Section \"%s\" ", (char*)&shstrtab[cur_sh->name]);
            print_elf32_symbol_table((elf32_symbol*)(data + cur_sh->offset), cur_sh->size / sizeof(elf32_symbol), (char*)data + sh[cur_sh->link].offset);            
        }  
        cur_sh ++;           
    }
}

static void print_elf64(const uint8_t* data, size_t len){
	uint32_t i;
    elf64_section_header* cur_sh;    
    const char* shstrtab;
    elf64_header* h = (elf64_header*)data;
    elf64_section_header* sh = (elf64_section_header*)(data + h->shoff);
    elf64_program_header* ph = (elf64_program_header*)(data + h->phoff);
    shstrtab = (char*)data + sh[h->shstrndx].offset;

    print_elf64_header(h);
    print_elf64_program_header(ph, h->phnum);
    print_elf64_section_header(sh, h->shnum, shstrtab);

    cur_sh = sh;
    for(i = 0; i < h->shnum; i++){
        if(cur_sh->type == ELF_SECTION_TYPE_DYNSYM || cur_sh->type == ELF_SECTION_TYPE_SYMTAB){
            printf("Section \"%s\" ", (char*)&shstrtab[cur_sh->name]);
            print_elf64_symbol_table((elf64_symbol*)(data + cur_sh->offset), cur_sh->size / sizeof(elf64_symbol), (char*)data + sh[cur_sh->link].offset);            
        }  
        cur_sh ++;           
    }
}


static int print_elf(const uint8_t* data, size_t len){
    elf32_header* h = (elf32_header*)data;
    if(h->magic[0] != ELF_MAGIC_0 
    || h->magic[1] != ELF_MAGIC_1 
    || h->magic[2] != ELF_MAGIC_2 
    || h->magic[3] != ELF_MAGIC_3 
    ){
        printf("not elf file\n");
        return -1;
    }
    if(h->cls == ELF_CLASS_32){
        print_elf32(data, len);
    }else if(h->cls == ELF_CLASS_64){
        print_elf64(data, len);
    }else{
        printf("invalid elf file\n");
        return -1;
    }
    return 0;
}



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

static int read_elf(const char* name){
    uint8_t* buf;
    int ret;
    size_t sz = file_size(name);
    if(!sz){
        printf("not file: %s\n", name);
        return -1;
    }
    buf = malloc(sz);
    if(!buf){
        printf("out of memory\n");
        return -1;        
    }
    ret = file_load(name, buf, &sz);
    if(ret){
        printf("read file error: %s\n", name);
        return -1;
    }  
    return print_elf(buf, sz);    
}



int main(int argc, char* argv[]){
    if(argc<=1){
        printf("usage:%s <elffile>\n", argv[0]);
        return 1;
    }else{
        return read_elf(argv[1]);
    }
}


