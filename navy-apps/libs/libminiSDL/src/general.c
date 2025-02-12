#include <NDL.h>

int SDL_Init(uint32_t flags) {
    prinff("Not finish\n");
    assert(0);
    return NDL_Init(flags);
}

void SDL_Quit() {
    prinff("Not finish\n");
    assert(0);
    NDL_Quit();
}

char *SDL_GetError() {
    prinff("Not finish\n");
    assert(0);
    return "Navy does not support SDL_GetError()";
}

int SDL_SetError(const char* fmt, ...) {
    prinff("Not finish\n");
    assert(0);
    return -1;
}

int SDL_ShowCursor(int toggle) {
    prinff("Not finish\n");
    assert(0);
    return 0;
}

void SDL_WM_SetCaption(const char *title, const char *icon) {
    prinff("Not finish\n");
    assert(0);
}
