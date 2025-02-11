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
    AM_INPUT_KEYBRD_T ev;
    ioe_read(AM_INPUT_KEYBRD, &ev);
    if (ev.keycode == AM_KEY_NONE) {
        *(char *)buf = '\0';
        return 0;
    }
    return snprintf(buf, len, "k%c %s\n", (ev.keydown ? 'd' : 'u'), keyname[ev.keycode]);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
    return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
    return 0;
}

void init_device() {
    Log("Initializing devices...");
    ioe_init();
}
