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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static uint32_t choose(uint32_t n) {
	n++;
	return rand() % n;	
}

static void gen(char c, char **start) {
	*(*start) = c;
	*start = (*start) + 1;
}

static void gen_s(char *s, char **start) {
	int n = strlen(s);
	for (int i = 0; i < n; i++) {
		gen(s[i], start);
	}
}

static void gen_num(char **start) {
	uint32_t num = 1 + choose(0xff);
	uint32_t tmp = num, pow = 1;
	while (tmp) {
		tmp /= 10;
		pow *= 10;
	}
	pow /= 10;
	while (pow) {
		gen(num / pow % 10 + '0', start);
		pow /= 10;
	}
}

static void gen_rand_2op(char **start) {
	switch(choose(11)) {
		case 0: gen_s(" + ", start); break;
		case 1: gen_s(" - ", start); break;
		case 2: gen_s(" * ", start); break;
		case 3: gen_s(" / ", start); break;
		case 4: gen_s(" =", start); gen_s("= ", start); break;
		case 5: gen_s(" !", start); gen_s("= ", start); break;
		case 6: gen_s(" <", start); gen_s("= ", start); break;
		case 7: gen_s(" >", start); gen_s("= ", start); break;
		case 8: gen_s(" < ", start); break;
		case 9: gen_s(" > ", start); break;
		case 10: gen_s(" &", start); gen_s("& ", start); break;
		case 11: gen_s(" |", start); gen_s("| ", start); break;
        default: break;
    }
}

static void gen_rand_expr(char **start, int dep) {
	switch (dep < 5 ? choose(3) : 0) {
		case 0: gen_num(start); break;
		case 1: gen('(', start); gen_rand_expr(start, dep + 1); gen(')', start); break;
		default: gen_rand_expr(start, dep + 1); gen_rand_2op(start); gen_rand_expr(start, dep + 1); break;
	}
}

int main(int argc, char *argv[]) {
	int seed = time(0);
  	srand(seed);
  	int loop = 1;
  	if (argc > 1) {
		sscanf(argv[1], "%d", &loop);
  	}
  	int i;
  	for (i = 0; i < loop; i ++) {
		memset(buf, 0, sizeof(buf));

		char **start, *tmp;
		start = &tmp;
		tmp = buf;

		gen_rand_expr(start, 0);

  	  	sprintf(code_buf, code_format, buf);

  	  	FILE *fp = fopen("/tmp/.code.c", "w");
  	  	assert(fp != NULL);
  	  	fputs(code_buf, fp);
  	  	fclose(fp);

  	  	int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
  	  	if (ret != 0) continue;

  	  	fp = popen("/tmp/.expr", "r");
  	  	assert(fp != NULL);

	  	int result;
  	  	ret = fscanf(fp, "%d", &result);
  	  	pclose(fp);

  	  	printf("%u %s\n", result, buf);
  	}
  	return 0;
}
