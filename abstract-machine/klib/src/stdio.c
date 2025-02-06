#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static int printf_len;

static char *sprintf_out;
static char *snprintf_out;
static char *snprintf_maxout;

static void printf_char(char c){
    putch(c);
    printf_len++ ;
}

static void sprintf_char(char c){
    *sprintf_out = c;
    sprintf_out++;
}

static void snprintf_char(char c){
    if (snprintf_out < snprintf_maxout) {
        *snprintf_out = c;
        snprintf_out++;
    }
}

static void print_c(void (*putc)(char), char c) {
    putc(c);
}

static void print_s(void (*putc)(char), char *s) {
    while (*s != '\0') {
        putc(*s);
        s++;
    }
}

static void print_d(void (*putc)(char), int d) {
    char num[10];
    if (d < 0){
        d = -d;
        putc('-');
    }
    int i = 0;
    num[i] = '0' + d % 10;
    d = d / 10;
    i++;
    while (d != 0) {
        num[i] = '0' + d % 10;
        d = d / 10;
        i++;
    }
    while (i--) {
        putc(num[i]);
    }
}

static void print_x(void (*putc)(char), uint32_t x) {
    char num[10];
    int i = 0;
    num[i] = (0 <= x % 16 && 9 >= x % 16) ? ('0' + x % 16) : ('a' + x % 16 - 10);
    x = x / 16;
    i++;
    while (x != 0) {
        num[i] = (0 <= x % 16 && 9 >= x % 16) ? ('0' + x % 16) : ('a' + x % 16 - 10);
        x = x / 16;
        i++;
    }
    while (i--) {
        putc(num[i]);
    }
}

static void print_p(void (*putc)(char), void *p) {
    print_x(putc, (uint32_t)p);
}

static void print(void (*putc)(char), va_list ap, char *first) {
    int f = 0;
    while (*first != '\0') {
        if (f == 1) {
            if (*first != '%') {
                f = 0;
            }
            switch (*first) {
                case 's':
                    char *s = va_arg(ap, char *);
                    print_s(putc, s);
                    break;
                case 'd':
                    int d = va_arg(ap, int);
                    print_d(putc, d);
                    break;
                case 'c':
                    char c = va_arg(ap, int);
                    print_c(putc, c);
                    break;
                case 'x':
                    uint32_t x = va_arg(ap, int);
                    print_x(putc, x);
                    break;
                case 'p':
                    void *p = va_arg(ap, void *);
                    print_p(putc, p);
                    break;
                case '%':
                    putc('%');
                    break;
                default:
                    putc('%');
                    putc(*first);
                    break;
            }
        } else if (*first == '%') {
            f = 1;
        } else {
            putc(*first);
        }
        first++;
    } 
    if (f == 1) {
        putc('%');
    }
}

int printf(const char *fmt, ...) {
    printf_len = 0;
    va_list ap;
    va_start(ap, fmt);
    char *first = (char *)fmt;
    print(printf_char, ap, first);
    va_end(ap);
    return printf_len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
    //out <- fmt
    sprintf_out = out;
    va_list ap;
    va_start(ap, fmt);
    char *first = (char *)fmt;
    print(sprintf_char, ap, first);
    va_end(ap);
    *sprintf_out = '\0';
    return sprintf_out - out;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    snprintf_maxout = out + n - 1;
    snprintf_out = out;
    va_list ap;
    va_start(ap, fmt);
    char *first = (char *)fmt;
    print(snprintf_char, ap, first);
    va_end(ap);
    *snprintf_out = '\0';
    return snprintf_out - out;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
