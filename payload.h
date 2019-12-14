//
// Created by arattel on 13.12.19.
//

#ifndef ELF_INFECTOR_PAYLOAD_H
#define ELF_INFECTOR_PAYLOAD_H

#include <elf.h>
#include <glob.h>

void get_virus_payload(char* filename, uint8_t** memory, size_t* virus_size, Elf64_Addr* jump_off_t);


#endif //ELF_INFECTOR_PAYLOAD_H
