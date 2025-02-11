#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
    char *name;
    size_t size;
    size_t disk_offset;
    size_t open_offset;
    ReadFn read;
    WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
    panic("should not reach here");
    return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
    panic("should not reach here");
    return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
    [FD_STDIN]  = {"stdin", 0, 0, 0, invalid_read, invalid_write},
    [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, NULL},
    [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, NULL},
#include "files.h"
};

static int file_num = sizeof(file_table) / sizeof(Finfo);

size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t ramdisk_read(void *buf, size_t offset, size_t len);

char *fs_name(int fd) {
    if (fd >= 0 && fd < file_num) return file_table[fd].name;
    else return "Wrong file";
}

int fs_open(const char *pathname, int flags, int mode) {
    for (int i = FD_FB; i < file_num; i++ ) {
        if (strcmp(file_table[i].name, pathname) == 0) {
            return i;
        }
    }
    panic("Invalid file name \"%s\"!", pathname);
    return -1;
}

size_t fs_write(int fd, const void *buf, size_t len) {
    if (fd >= FD_FB && fd < file_num) {
        if (file_table[fd].write != NULL) {
            return file_table[fd].write(buf, file_table[fd].open_offset, len);
        } else {
            int write_len = (len < file_table[fd].size - file_table[fd].open_offset) ? len : (file_table[fd].size - file_table[fd].open_offset);
            ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset , write_len);
            file_table[fd].open_offset += write_len;
            return write_len;
        }
    } else if (fd == FD_STDOUT || fd == FD_STDERR) {
        const char *str = buf;
        for (size_t i = 0; i < len; i++) {
            putch(str[i]);
        }
        return len;
    } else if (fd == FD_STDIN) {
        return 0;
    }
    panic("Error, fd %d out of bound %d!", fd, file_num);
    return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
    if (fd >= FD_FB && fd < file_num) {
        if (file_table[fd].read != NULL) {
            return file_table[fd].read(buf, file_table[fd].open_offset, len);
        } else {
            size_t read_len = (len + file_table[fd].open_offset <= file_table[fd].size) ? len : (file_table[fd].size - file_table[fd].open_offset);
            ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset , read_len);
            file_table[fd].open_offset += read_len;
            return read_len;
        }
    } else if (fd >= 0 && fd < FD_FB) {
        return 0;
    }
    panic("Error, fd %d out of bound %d!", fd, file_num);
    return -1;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
    if (fd >= FD_FB && fd < file_num) {
        switch (whence) {
            case SEEK_SET: file_table[fd].open_offset = offset; break;
            case SEEK_CUR: file_table[fd].open_offset += offset; break;
            case SEEK_END: file_table[fd].open_offset = file_table[fd].size + offset; break;
            default: return -1; break;
        }
        if (file_table[fd].open_offset > file_table[fd].size) {
            panic("New offset %d out of bound %d!", file_table[fd].open_offset, file_table[fd].size);
            file_table[fd].open_offset = 0;
            return -1;
        } else {
            return file_table[fd].open_offset;
        }
    } else if (fd >= 0 && fd < FD_FB) {
        return 0;
    }
    panic("Error, fd %d out of bound %d!", fd, file_num);
    return -1;
}

int fs_close(int fd) {
    if(fd >= 0 && fd < file_num) {
        file_table[fd].open_offset = 0;
        return 0;
    }
    panic("Invalid fd %d!", fd);
    return -1;
}

void init_fs() {
    //AM_GPU_CONFIG_T gpu_config;
    //ioe_read(AM_GPU_CONFIG, &gpu_config);
    //file_table[fs_open("/dev/fb", 0, 'r')].size = gpu_config.width * gpu_config.height;
}
