# ELF-infector 

## Basic description

This infector is an implementation of Silvio Cesare ELf infector algorithm. 
Pseusodocode:
```
	* Increase p_shoff by PAGE_SIZE in the ELF header
	* Patch the insertion code (parasite) to jump to the entry point
	  (original)
	* Locate the text segment program header
		* Modify the entry point of the ELF header to point to the new
		  code (p_vaddr + p_filesz)
		* Increase p_filesz by account for the new code (parasite)
		* Increase p_memsz to account for the new code (parasite)
	* For each phdr who's segment is after the insertion (text segment)
		* increase p_offset by PAGE_SIZE
	* For the last shdr in the text segment
		* increase sh_len by the parasite length
	* For each shdr who's section resides after the insertion
		* Increase sh_offset by PAGE_SIZE
	* Physically insert the new code (parasite) and pad to PAGE_SIZE, into
	  the file - text segment p_offset + p_filesz (original)
```

[Pseudocode source](https://www.win.tue.nl/~aeb/linux/hh/virus/unix-viruses.txt)

## Demonstration

Demonstration is basically compiling source code, payload, and copying `ls` from /bin/ to current directory and then 
infecting `ls` with payload. To do this, type 
```
chmod +x demonstration.sh 
./demonstration.sh
```

Then file `out_file` will be infected instance of `ls`.

## Later usage

For ELF executable we have 2 options: general, where jump to entry point is just appended to the end of .text segment 
of a virus and "perkele", which is a bit more complicated, because it's adapted to a my virus payload. 

./infector [-h|-p] <infected_filename> <payload_filename> <output_filename> <br>
Options: <br>
-h --help     Show this screen. <br>
-p     Payload path is a path to perkele virus.