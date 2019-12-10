//
// Created by arattel on 10.12.19.
//
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <zconf.h>
#include <stdint.h>
#include <sys/mman.h>
#include <elf.h>
#include <string.h>


int main(int argc, char** argv){
    char *filename;
    struct stat sb;

    if(argc < 2){
        return -1;
    } else{
        filename = argv[1];
    }
    int fd = open(filename, O_RDONLY);
    if(fd < 0){
        return -1;
    }
    if(fstat(fd, &sb) < 0){
        close(fd);
        return  -1;
    }

    uint8_t *mem = mmap(NULL, sb.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    if(mem ==  MAP_FAILED){
        return -1;
    }
    Elf64_Ehdr* ehdr = (Elf64_Ehdr *) mem;
    Elf64_Phdr* phdr = (Elf64_Phdr *) &mem[ehdr->e_phoff];
    Elf64_Shdr* shdr = (Elf64_Shdr *) &mem[ehdr->e_shoff];
    Elf64_Addr text_beginning, text_end;


    for(int  i = 0; i < ehdr->e_phnum; i++){
        if(phdr[i].p_type == PT_LOAD){
            text_beginning = phdr[i].p_vaddr;
            text_end = text_beginning + phdr[i].p_filesz;
            break;
        }
    }
    char *character = mem + text_end - 5;
    char number[10];
    sprintf(number, "%X", character[0]);
    if(strstr(number, "E9") == NULL){
        printf("File %s is not infected\n", filename);
    } else{
        printf("File %s is infected\n", filename);
    }
    return 0;
}