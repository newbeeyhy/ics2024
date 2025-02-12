#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;
static int canvas_x = 0, canvas_y = 0;

uint32_t NDL_GetTicks() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
    int fd = open("/dev/events", 0, 0);
    int ret = read(fd, buf, len);
    close(fd);
    return ret;
}

static int decode(int i, char *buf, const char *key, int *value) {
    #define buf_size 1024

    int len = strlen(key);

    assert(strncmp(buf + i, key, len) == 0);
    i += len;

    for (; i < buf_size; i++) {
        if (buf[i] == ':') { i++; break; }
        assert(buf[i] == ' ');
    }
    for (; i < buf_size; i++) {
        if (buf[i] >= '0' && buf[i] <= '9') break;
        assert(buf[i] == ' ');
    }
    for (; i < buf_size; i++) {
        if (buf[i] >= '0' && buf[i] <= '9') {
            *value = (*value) * 10 + buf[i] - '0';
        } else {
            break;
        }
    }
    return i;
}

void NDL_OpenCanvas(int *w, int *h) {
    if (getenv("NWM_APP")) {
        int fbctl = 4;
        fbdev = 5;
        screen_w = *w; screen_h = *h;
        char buf[64];
        int len = sprintf(buf, "%d %d", screen_w, screen_h);
        // let NWM resize the window and create the frame buffer
        write(fbctl, buf, len);
        while (1) {
            // 3 = evtdev
            int nread = read(3, buf, sizeof(buf) - 1);
            if (nread <= 0) continue;
            buf[nread] = '\0';
            if (strcmp(buf, "mmap ok") == 0) break;
        }
        close(fbctl);
    }

    char buf[1024];
    int fd = open("/proc/dispinfo", 0, 0);
    int ret = read(fd, buf, buf_size);

    assert(ret < buf_size); // to be cautious...

    close(fd);

    int i = 0;
    int width = 0, height = 0;

    i = decode(i, buf, "WIDTH", &width);
    assert(buf[i++] == '\n');
    i = decode(i, buf, "HEIGHT", &height);

    screen_w = width;
    screen_h = height;

    if (*w == 0 && *h == 0) {
        *w = screen_w;
        *h = screen_h;
    }

    canvas_w = *w;
    canvas_h = *h;

    assert(canvas_w <= screen_w && canvas_h <= screen_h);

    canvas_x = (screen_w - canvas_w) / 2;
    canvas_y = (screen_h - canvas_h) / 2;
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
    int fd = open("/dev/fb", 0, 0);
    for (int i = 0; i < h && y + i < canvas_h; i++) {
        lseek(fd, ((y + canvas_y + i) * screen_w + (x + canvas_x)) * 4, 0);
        write(fd, pixels + i * w, 4 * ((w < canvas_w - x) ? w : (canvas_w - x)));
    }
    close(fd);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
    return 0;
}

int NDL_QueryAudio() {
    return 0;
}

int NDL_Init(uint32_t flags) {
    if (getenv("NWM_APP")) {
        evtdev = 3;
    }
    return 0;
}

void NDL_Quit() {
}
