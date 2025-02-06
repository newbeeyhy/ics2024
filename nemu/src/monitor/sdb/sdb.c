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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
    static char *line_read = NULL;

    if (line_read) {
        free(line_read);
        line_read = NULL;
    }

    line_read = readline("(nemu) ");

    if (line_read && *line_read) {
        add_history(line_read);
    }

    return line_read;
}

static int cmd_c(char *args) {
    cpu_exec(-1);
    return 0;
}


static int cmd_q(char *args) {
    nemu_state.state = NEMU_QUIT;
    return -1;
}

static int cmd_si(char *args) {
    if (args == NULL) {
        cpu_exec(1);
        return 1;
    }
    int step = 0;
    int len = strlen(args);
    for (int i = 0; i < len; i++) {
        step = step * 10 + *(args + i) - '0';
    }
    cpu_exec(step);
    return step;
}

void display_wp();

static int cmd_info(char *args) {
    Assert(args != NULL, "");
    if (*args == 'r') {
        isa_reg_display();
        return 0;
    }
    if (*args == 'w') {
        display_wp();
        return 0;
    }
    Log("Info Args Error!");
    return -1;
}

word_t paddr_read(paddr_t addr, int len);

static int cmd_x(char *args) {
    if (args == NULL) {
        puts("Command format: \"x N EXPR\"");
        return 0;
    }

    char *p = strtok(NULL, " ");

    if (p == NULL) {
        puts("Command format: \"x N EXPR\"");
        return 0;
    }

    args += strlen(p) + 1;
    int len;
    sscanf(p, "%d", &len);

    bool success;
    uint32_t start = expr(args, &success);
    if (!success) {
        printf("invalid expression: '%s'\n", args);
    } else {
        int i;
        for (i = 0; i < len; i++) {
            printf("0x%08x: 0x%08x\n", start + i, paddr_read(start + i * 4, 4));
        }
    }
    return 0;
}

static int cmd_p(char *args) {
    if (args == NULL) {
        puts("Command format: \"p EXPR\"");
        return 0;
    }

    bool success;
    uint32_t val = expr(args, &success);
    if (!success) {
        printf("invalid expression: '%s'\n", args);
    } else {
        printf("HEX: 0x%x    DEC: %u\n", val, val);
    }
    return 0;
}

void add_wp(char *e);
void delete_wp(int n);

static int cmd_w(char *args) {
    add_wp(args);
    return 0;
}

static int cmd_d(char *args) {
    int n;
    sscanf(args, "%d", &n);
    delete_wp(n);
    return 0;
}

static int cmd_help(char *args);

static struct {
    const char *name;
    const char *description;
    int (*handler) (char *);
} cmd_table [] = {
    { "help", "Display information about all supported commands", cmd_help },
    { "c", "Continue the execution of the program", cmd_c },
    { "q", "Exit NEMU", cmd_q },
    { "si", "Single step execution", cmd_si },
    { "info", "Print program status", cmd_info },
    { "x", "Scan memory", cmd_x },
    { "p", "Calc expr", cmd_p },
    { "w", "Add watchpoint", cmd_w },
    { "d", "Delete watchpoint", cmd_d }

    /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
    /* extract the first argument */
    char *arg = strtok(NULL, " ");
    int i;

    if (arg == NULL) {
        /* no argument given */
        for (i = 0; i < NR_CMD; i ++) {
            printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        }
    }
    else {
        for (i = 0; i < NR_CMD; i ++) {
            if (strcmp(arg, cmd_table[i].name) == 0) {
                printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
                return 0;
            }
        }
        printf("Unknown command '%s'\n", arg);
    }
    return 0;
}

void sdb_set_batch_mode() {
    Log("Run with batch mode.");
    is_batch_mode = true;
}

void sdb_mainloop() {
    if (is_batch_mode) {
        cmd_c(NULL);
        return;
    }

    for (char *str; (str = rl_gets()) != NULL; ) {
        char *str_end = str + strlen(str);

        /* extract the first token as the command */
        char *cmd = strtok(str, " ");
        if (cmd == NULL) { continue; }

        /* treat the remaining string as the arguments,
        * which may need further parsing
        */
        char *args = cmd + strlen(cmd) + 1;
        if (args >= str_end) {
            args = NULL;
        }

#ifdef CONFIG_DEVICE
        extern void sdl_clear_event_queue();
        sdl_clear_event_queue();
#endif

        int i;
        for (i = 0; i < NR_CMD; i ++) {
            if (strcmp(cmd, cmd_table[i].name) == 0) {
                if (cmd_table[i].handler(args) < 0) { return; }
                break;
            }
        }

        if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
    }
}

void init_sdb() {
    /* Compile the regular expressions. */
    init_regex();

    /* Initialize the watchpoint pool. */
    init_wp_pool();
}
