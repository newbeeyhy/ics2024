#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
    uint32_t key_data = inl(KBD_ADDR);
    kbd->keycode = key_data & (~KEYDOWN_MASK) ;
    kbd->keydown = ((kbd->keycode | KEYDOWN_MASK) == key_data) ? true : false;
}
