#include <common.h>
#include "syscall.h"
#include <fs.h>

struct timeval {
    long tv_sec;  // 从1970年1月1日00:00:00 UTC到当前时间的秒数
    long tv_usec; // 当前秒内的微秒数
};

struct timezone {
    int tz_minuteswest;  // 和格林威治时间（GMT）相差的分钟数，负数表示在格林威治时间西边
    int tz_dsttime;      // 夏令时相关信息（具体值和系统相关）
};

int sys_gettimeofday(struct timeval *tv, struct timezone *tz){
    if (tv == NULL && tz == NULL) return -1;
    if (tv != NULL) {
        int us;
        ioe_read(AM_TIMER_UPTIME, &us);
        tv->tv_sec = us / 1000000;
        tv->tv_usec = us % 1000000;
    }
    if (tz != NULL) {
        panic("Timezone wrong");
    }
    return 0;
}

void do_syscall(Context *c) {
    uintptr_t a[4];
    a[0] = c->GPR1;
    a[1] = c->GPR2;
    a[2] = c->GPR3;
    a[3] = c->GPR4;
    switch (a[0]) {
        case SYS_exit: halt(a[1]); break;
        case SYS_yield: c->GPRx = yield(); break;
        case SYS_brk: c->GPRx = 0; break;
        case SYS_open: c->GPRx = fs_open((char *)a[1], a[2], a[3]); break;
        case SYS_read: c->GPRx = fs_read(a[1], (char *)a[2], a[3]); break;
        case SYS_write: c->GPRx = fs_write(a[1], (char*)a[2], a[3]); break;
        case SYS_lseek: c->GPRx = fs_lseek(a[1], a[2], a[3]); break;
        case SYS_close: c->GPRx = fs_close(a[1]); break;
        case SYS_gettimeofday: c->GPRx = sys_gettimeofday((void *)a[1], (void *)a[2]); break;
        default: panic("Unhandled syscall ID = %d", a[0]);
    }
}
