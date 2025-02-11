#include <proc.h>
#include <elf.h>
#include <fs.h>

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

static uintptr_t loader(PCB *pcb, const char *filename) {
    Log("Loading elf \"%s\" from disk...", filename);

    Elf_Ehdr elf;
    Elf_Phdr ph;

    int fd = fs_open(filename, 0, 0);

    if (fd == -1) {
        panic("Load file %s fault!", filename);
        return -1;
    }

    fs_lseek(fd, ELF_OFFSET_IN_DISK, 0);
    fs_read(fd, &elf, sizeof(elf));

    assert(*(uint32_t *)elf.e_ident == 0x464c457f); // check magic
    assert(elf.e_machine == EXPECT_TYPE); // check arch

    fs_lseek(fd, elf.e_phoff, 1);

    for (size_t i = 0; i < elf.e_phnum; i++) {
        fs_lseek(fd, ELF_OFFSET_IN_DISK + elf.e_phoff + i * elf.e_phentsize, 0);
        fs_read(fd, &ph, elf.e_phentsize);
        if (ph.p_type == PT_LOAD) {
            fs_lseek(fd, ELF_OFFSET_IN_DISK + ph.p_offset, 0);
            fs_read(fd, (void *)ph.p_vaddr, ph.p_filesz);
            memset((void *)ph.p_vaddr + ph.p_filesz, 0, ph.p_memsz - ph.p_filesz);
        }
    }

    fs_close(fd);

    return elf.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = %p", entry);
    ((void(*)())entry) ();
}

