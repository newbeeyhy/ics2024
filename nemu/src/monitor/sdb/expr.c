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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */

#include <regex.h>

enum {
	TK_NOTYPE = 256,
	TK_EQ,
	TK_NEQ,
	TK_LEQ,
	TK_GEQ,
	TK_AND,
	TK_OR,
	TK_HEX,
	TK_DEC,
	TK_REG

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

	{" +", TK_NOTYPE},						// spaces
	{"\\+", '+'},							// add
	{"-", '-'},								// sub
	{"\\*", '*'},							// mul
	{"/", '/'},								// div
	{"\\(", '('},							// left
	{"\\)", ')'},							// right
	{"<=", TK_LEQ}, 						// less or equal
	{">=", TK_GEQ}, 						// greater or equal
	{"<", '<'},								// less
	{">", '>'},								// greater
	{"==", TK_EQ},							// equal
	{"!=", TK_NEQ},							// not equal
	{"&&", TK_AND},							// and
	{"\\|\\|", TK_OR},						// or
	{"!", '!' },							// not
	{"0x[0-9a-fA-F]+", TK_HEX},				// hex number
	{"[0-9]+", TK_DEC},						// dec number
	{"\\$[a-zA-Z]*[0-9]*", TK_REG},			// register
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static int num(char c) {
	if (c >= '0' && c <= '9') {
		return c - '0';
	} else if (c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	} else if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	} else {
		assert(0);
	}
}

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0') {
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i ++) {
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				// Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);

				position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
			
				switch (rules[i].token_type) {
				case TK_NOTYPE:
					break;
				case TK_HEX:
					tokens[nr_token].type = rules[i].token_type;
					for (int j = 2; j < substr_len; j++) {
						tokens[nr_token].str[j - 2] = substr_start[j];
					}
					tokens[nr_token].str[substr_len - 2] = '\0';
					nr_token++;
					break;
				case TK_DEC:
					tokens[nr_token].type = rules[i].token_type;
					for (int j = 0; j < substr_len; j++) {
						tokens[nr_token].str[j] = substr_start[j];
					}
					tokens[nr_token].str[substr_len] = '\0';
					nr_token++;
					break;
				case TK_REG:
					tokens[nr_token].type = rules[i].token_type;
					for (int j = 0; j < substr_len; j++) {
						tokens[nr_token].str[j] = substr_start[j];
					}
					tokens[nr_token].str[substr_len] = '\0';
					nr_token++;
					break;
				default:
					tokens[nr_token].type = rules[i].token_type;
					nr_token++;
				}
				break;
			}
		}

		if (i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true;
}

static bool check_parentheses(int p, int q) {
	int cnt = 0;
	for (int i = p; i <= q; i++) {
		if (tokens[i].type == '(') {
			cnt++;
		} else if (tokens[i].type == ')') {
			cnt--;
		}
	}
	Assert(cnt == 0, "");
	if (tokens[p].type == '(' && tokens[q].type == ')') {
		cnt = 0;
		for (int i = p + 1; i <= q - 1; i++) {
			if (tokens[i].type == '(') {
				cnt++;
			} else if (tokens[i].type == ')') {
				cnt--;
			}
			if (cnt < 0) {
				return false;
			}
		}
		if (cnt == 0) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

static int tokens_priority(int type) {
	switch (type) {
		case '!': return 5;
		case '*': return 4;
		case '/': return 4;
		case '+': return 3;
		case '-': return 3;
		case '<': return 2;
		case '>': return 2;
		case TK_LEQ: return 2;
		case TK_GEQ: return 2;
		case TK_EQ: return 2;
		case TK_NEQ: return 2;
		case TK_AND: return 1;
		case TK_OR: return 0;
		default: return 0x7fffffff;
	}
}

extern const char *regs[];

static uint32_t eval(int p, int q) {
    if(p > q) {
        return 0;
    } else if(p == q) {

        /* Single token.
         * For now this token should be a number.
         * Return the value of the number.
         */

		if (tokens[p].type == TK_REG) {
			char *reg_name = tokens[p].str + 1;
			if (*reg_name == '0') {
				return cpu.gpr[0];
			}
			for (int i = 1; i < 32; i++) {
				if (strcmp(reg_name, regs[i]) == 0) {
					return cpu.gpr[i];
				}
			}
			printf("Reg %s not found!", reg_name);
			return 0;
		} else if (tokens[p].type == TK_HEX || tokens[p].type == TK_DEC) {
			uint32_t res = 0;
			if (tokens[p].type == TK_DEC) {
				for (int i = 0; tokens[p].str[i] != 0; i++) {
					res = res * 10 + num(tokens[p].str[i]);
				}
			} else {
				for (int i = 0; tokens[p].str[i] != 0; i++) {
					res = res * 16 + num(tokens[p].str[i]);
				}
			}
			return res;
		}
    } else if(check_parentheses(p, q) == true) {

        /* The expression is surrounded by a matched pair of parentheses.
         * If that is the case, just throw away the parentheses.
         */

        return eval(p + 1, q - 1);

    } else {
		int op = -1;
		int op_type = 0;
		int cnt = 0;
		for (int i = p; i <= q; i++) {
			if (tokens[i].type == '(') {
				cnt++;
				continue;
			} else if (tokens[i].type == ')') {
				cnt--;
				continue;
			}
			if (cnt == 0 && check_parentheses(p, i - 1) && check_parentheses(i + 1, q)) {
				op = i;
				op_type = tokens[i].type;
				break;
			}
		}
		if (op == -1) {
			cnt = 0;
			for (int i = p; i <= q; i++) {
				if (tokens[i].type == '(') {
					cnt++;
					continue;
				} else if (tokens[i].type == ')') {
					cnt--;
					continue;
				}
				if (cnt == 0 && tokens_priority(tokens[i].type) < tokens_priority(op_type)) {
					op = i;
					op_type = tokens[i].type;
				}
			}
		}

		uint32_t val1 = eval(p, op - 1);
        uint32_t val2 = eval(op + 1, q);

        switch(op_type) {
            case '+': return val1 + val2;
            case '-': return val1 - val2;
            case '*': return val1 * val2;
            case '/': return val1 / val2;
			case TK_EQ: return val1 == val2;
			case TK_NEQ: return val1 != val2;
			case TK_LEQ: return val1 <= val2;
			case TK_GEQ: return val1 >= val2;
			case '<': return val1 < val2;
			case '>': return val1 > val2;
			case TK_AND: return val1 && val2;
			case TK_OR: return val1 || val2;
			case '!': return !val2;
            default: break;
        }
    }
	Assert(0, "");
}

word_t expr(char *e, bool *success) {
	if (!make_token(e)) {
		*success = false;
		return 0;
	}

	*success = true;
	return eval(0, nr_token - 1);
}
