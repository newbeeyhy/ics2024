/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

word_t get_status_bit(int n) {
    return (cpu.mstatus >> n) & 1;
}

void set_status_bit(int n, word_t x) {
    cpu.mstatus = (cpu.mstatus & (~(1 << n))) | (x << n);
}

#define MIE_offset 3
#define MPIE_offset 7

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
    /* Trigger an interrupt/exception with NO.
     * Then return the address of the interrupt/exception vector.
     */
    cpu.mcause = NO;
    cpu.mepc = epc;
    set_status_bit(MPIE_offset, get_status_bit(MIE_offset));
    set_status_bit(MIE_offset, 0);
    return cpu.mtvec;
}

word_t isa_query_intr() {
    return INTR_EMPTY;
}
