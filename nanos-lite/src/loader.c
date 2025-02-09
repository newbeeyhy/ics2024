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

    Elf_Ehdr *elf;
    Elf_Phdr *ph, *eph;
    
    uint8_t buf[4096];

    ramdisk_read(buf, ELF_OFFSET_IN_DISK, 4096);
    elf = (void *)buf;

    assert(*(uint32_t *)elf->e_ident == 0x464c457f); // check magic
    assert(elf->e_machine == EXPECT_TYPE); // check arch

    ph = (void *)elf + elf->e_phoff;
    eph = ph + elf->e_phnum;

    for (; ph < eph; ph++) {
        if (ph->p_type == PT_LOAD) {
            uint32_t vaddr;
            vaddr = ph->p_vaddr;
            memcpy((void *)vaddr, (void *)elf + ph->p_offset, ph->p_filesz);
            memset((void *)vaddr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
        }
    }

    volatile uint32_t entry = elf->e_entry;

    return entry;
}

void naive_uload(PCB *pcb, const char *filename) {
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = %p", entry);
    ((void(*)())entry) ();
}

