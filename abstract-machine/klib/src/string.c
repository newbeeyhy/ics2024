#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

void *memset(void *s, int c, size_t n) {
    char *char_s = (char *)s;
    for (size_t i = 0; i < n; i++) {
        char_s[i] = (char)c;
    }
    return s;
}

void *memmove(void *dst, const void *src, size_t n) {
    char *char_dst = (char *)dst;
    char *char_src = (char *)src;
    if (dst < src){
        for (size_t i = 0; i < n; i++) {
            char_dst[i] = char_src[i];
        }
    } else if (dst > src){
        for (size_t i = 0; i < n; i++) {
            char_dst[n - 1 - i] = char_src[n - 1 - i];
        }
    }
    return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
    char *char_dst = (char *)out;
    char *char_src = (char *)in;
    for (size_t i = 0; i < n; i++) {
        char_dst[i] = char_src[i];
    }
    return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    char *char_dst = (char *)s1;
    char *char_src = (char *)s2;
    for (int i = 0; i < n; i++) {
        int cmp = (int)char_dst[i] - (int)char_src[i];
        if (cmp != 0) {
            return cmp;    
        }
    }
    return 0;
}

size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

char *strcpy(char *dst, const char *src) {
    size_t len = strlen(src);
    memcpy(dst, src, len);
    dst[len] = '\0';
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
    size_t len = strlen(src);
    if (len <= n) {
        memcpy(dst, src, len);
        memset(dst + n, 0, n - len);
    } else {
        memcpy(dst, src, n);
    }
    return dst;
}

char *strcat(char *dst, const char *src) {
    strcpy(dst + strlen(dst), src);
    return dst;
}

int strcmp(const char *s1, const char *s2) {
    for (size_t i = 0; ; i++) {
        int cmp = (int)s1[i] - (int)s2[i];
        if (s1[i] == '\0' && s2[i] == '\0') {
            break;
        } else if (cmp != 0) {
            return cmp;
        }
    }
    return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        int cmp = (int)s1[i] - (int)s2[i];
        if (s1[i] == '\0' && s2[i] == '\0') {
            break;
        } else if (cmp != 0) {
            return cmp;
        }
    }
    return 0; 
}

#endif
