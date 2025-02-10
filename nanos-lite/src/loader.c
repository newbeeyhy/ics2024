#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

#if defined(__ISA_AM_NATIVE__)
# define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_X86__)
# define EXPECT_TYPE EM_X86_64
#elif defined(__riscv)
# define EXPECT_TYPE EM_RISCV
#else
# error Unsupported ISA
#endif

#define ELF_OFFSET_IN_DISK 0

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t get_ramdisk_size();

static uintptr_t loader(PCB *pcb, const char *filename) {
    Log("Loading elf from disk...");

    Elf_Ehdr elf;
    Elf_Phdr ph;
    
    ramdisk_read(&elf, ELF_OFFSET_IN_DISK, sizeof(elf));

    assert(*(uint32_t *)elf.e_ident == 0x464c457f); // check magic
    assert(elf.e_machine == EXPECT_TYPE); // check arch

    for (size_t i = 0; i < elf.e_phnum; i++) {
        ramdisk_read(&ph, ELF_OFFSET_IN_DISK + elf.e_phoff + i * elf.e_phentsize, elf.e_phentsize);
        if (ph.p_type == PT_LOAD) {
            ramdisk_read((void *)ph.p_vaddr, ELF_OFFSET_IN_DISK + ph.p_offset, ph.p_filesz);
            memset((void *)ph.p_vaddr + ph.p_filesz, 0, ph.p_memsz - ph.p_filesz);
        }
    }

    return elf.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = %p", entry);
    ((void(*)())entry) ();
}

