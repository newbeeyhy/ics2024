#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
    [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
    [AM_KEY_NONE] = "NONE",
    AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
    const char *str = buf;
    for (size_t i = 0; i < len; i++) {
        putch(str[i]);
    }
    return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
    AM_INPUT_KEYBRD_T event;
    ioe_read(AM_INPUT_KEYBRD, &event);
    if (event.keycode == AM_KEY_NONE) {
        *(char *)buf = '\0';
        return 0;
    }
    return snprintf(buf, len, "k%c %s\n", (event.keydown ? 'd' : 'u'), keyname[event.keycode]);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
    AM_GPU_CONFIG_T gpu_config;
    ioe_read(AM_GPU_CONFIG, &gpu_config);
    return snprintf(buf, len, "WIDTH: %d\nHEIGHT: %d\n", gpu_config.width, gpu_config.height);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
    AM_GPU_CONFIG_T gpu_config = io_read(AM_GPU_CONFIG);
    int width = gpu_config.width;

    offset /= 4;
    len /= 4;

    int y = offset / width;
    int x = offset - y * width;

    io_write(AM_GPU_FBDRAW, x, y, (void *)buf, len, 1, true);

    return len;
}

void init_device() {
    Log("Initializing devices...");
    ioe_init();
}
