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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	char EXPR[128];
	uint32_t value;

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
	int i;
	for (i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
	}

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

static WP *new_wp() {
	WP *tmp = free_;
	free_ = free_->next;
	Assert(tmp != NULL, "Watchpoint pool is empty!");
	tmp->next = NULL;
	return tmp;
}

static void free_wp(WP *wp) {
	Assert(wp != NULL, "Free a NULL watchpoint!");
	wp->next = free_;
	free_ = wp;
}

static bool init_wp(WP *wp, char *e) {
	bool success;
	uint32_t val = expr(e, &success);
	if (success) {
		strcpy(wp->EXPR, e);
		wp->value = val;
		return true;
	} else {
		puts("Init watchpoint failure.");
		return false;
	}
}

static void wp_info(WP *wp) {
	printf("[NO %d]  EXPR: %s    Value: 0x%x\n", wp->NO, wp->EXPR, wp->value);
}

bool check_wp() {
	WP *now = head;
	bool flag = false;
	while (now != NULL) {
		bool success;
		uint32_t val = expr(now->EXPR, &success);
		if (val != now->value) {
			puts("Watchpoint's value has changed.");
			printf("[NO %d]  EXPR: %s    Old value: 0x%x    New value: 0x%x\n", now->NO, now->EXPR, now->value, val);		
			now->value = val;
			flag = true;
		}
		now = now->next;
	}
	return flag;
}

void display_wp() {
	WP *now = head;
	while (now != NULL) {
		wp_info(now);
		now = now->next;
	}
}

void add_wp(char *e) {
	WP *tmp = new_wp();
	if (init_wp(tmp, e)) {
		tmp->next = head;
		head = tmp;
		puts("Successfully add watchpoint.");
	}
}

void delete_wp(int n) {
	WP *now = head, *pre = NULL;
	while (now != NULL) {
		if (now->NO == n) {
			if (pre != NULL) {
				pre->next = now->next;
				free_wp(now);
				now = pre->next;
			} else {
				head = now->next;
				free_wp(now);
				now = head;
			}
			printf("Successfully delete NO %d watchpoint.\n", n);
			return;
		}
		pre = now;
		now = now->next;
	}
	printf("NO %d watchpoint not found.\n", n);
}

