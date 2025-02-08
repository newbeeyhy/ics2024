#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
    uint32_t data = inl(VGACTL_ADDR);
    int w = data >> 16, h = data & 0xffff; 
    *cfg = (AM_GPU_CONFIG_T) {
        .present = true,
        .has_accel = false,
        .width = w,
        .height = h,
        .vmemsz = w * h * sizeof(uint32_t)
    };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
    uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
    uint32_t *pix = (uint32_t *)(ctl->pixels) ;
    int now = 0;
    for (int i = ctl->y; i < ctl->y + ctl->h ; i++) {
        for (int j = ctl->x; j < ctl->x + ctl->w ; j++) {
            fb[i * 400 + j] = pix[now];
            now++;
        }
    }
    if (ctl->sync) {
        outl(SYNC_ADDR, 1);
    }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
    status->ready = true;
}
