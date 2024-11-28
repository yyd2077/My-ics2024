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
#include <memory/paddr.h>
#include <memory/vaddr.h>


static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
void wp_display();
void wp_add(vaddr_t addr);
void wp_delete(int N);



/* 使用 `readline` 库从标准输入读取命令，更加灵活 */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);  // 释放之前读取的内存
    line_read = NULL;
  }

  line_read = readline("(nemu) ");  // 提示符为 (nemu)

  if (line_read && *line_read) {
    add_history(line_read);  // 将输入的命令添加到历史记录中
  }

  return line_read;
}


/* 命令 'c'：继续执行程序 */
/* 命令 'c'：继续执行程序 */
static int cmd_c(char *args) {
  if (nemu_state.state == NEMU_STOP) {
    nemu_state.state = NEMU_RUNNING;  // 恢复执行
    printf("Execution continues...\n");
    cpu_exec(-1);  // 继续执行直到程序结束或者再次被暂停
  } else {
    printf("Program is already running.\n");
    cpu_exec(-1);  // 继续执行直到程序结束
  }
  return 0;
}


/* 命令 'q'：退出 NEMU */
static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;  // 设置状态为退出
  return -1;
}

static int cmd_si(char *args) {
  int n = 1;  // 默认执行一条指令

  // 检查是否有参数
  if (args != NULL && strlen(args) > 0) {
    char *endptr;
    long val = strtol(args, &endptr, 10);

    // 检查参数是否有效
    if (*endptr != '\0' || val <= 0) {
      printf("Invalid argument for 'si'. Please provide a positive integer.\n");
    } else {
      n = (int)val;

      // 设置合理的执行步数上限
      #define MAX_EXEC_STEPS 10000
      if (n > MAX_EXEC_STEPS) {
        printf("Too many steps requested. Limiting to %d steps.\n", MAX_EXEC_STEPS);
        n = MAX_EXEC_STEPS;
      }
    }
  }

  // 执行 n 条指令
  cpu_exec(n);
  return 0;
}



/* 命令 'info'：打印程序状态 */
static int cmd_info(char *args){

  if (args != NULL && strcmp(args, "r") == 0) {
    isa_reg_display();
  }

  else if (args != NULL && strcmp(args, "w") == 0) {
    wp_display();
  }
  return 0;
}

static int cmd_x(char *args) {
  if (args == NULL) {
    printf("Usage: x N EXPR\n");
    return 0;
  }

  // 提取 N 和 EXPR 参数
  char *n_str = strtok(args, " ");
  char *expr_str = strtok(NULL, "");

  if (n_str == NULL || expr_str == NULL) {
    printf("Usage: x N EXPR\n");
    return 0;
  }

  // 将 N 转换为整数
  char *endptr;
  long n = strtol(n_str, &endptr, 10);
  if (*endptr != '\0' || n <= 0) {
    printf("Invalid value for N. Please provide a positive integer.\n");
    return 0;
  }

  // 计算内存地址
  // printf("Evaluating expression: %s\n", expr_str);
  bool success = false;  // 初始化 success
  vaddr_t addr = expr(expr_str, &success);
  // printf("Computed address: 0x%08x\n", addr);

  if (addr == -1) {
    printf("Invalid expression: %s\n", expr_str);
    return 0;
  }

  // 读取内存并打印
  for (int i = 0; i < n; i++) {
    uint32_t data = vaddr_read(addr + i * 4, 4);
    printf("0x%08x: 0x%08x\n", addr + i * 4, data);
  }
  return 0;
}

//表达式求值
static int cmd_p(char *args) {
  bool success = true;  // 用于判断表达式是否计算成功
  word_t result;

  // 计算表达式
  result = expr(args, &success);  // 使用 expr 函数进行求值

  if (success) {
    printf("Result: 0x%x\n", result);  // 输出计算结果，使用 %x 来匹配 word_t
  } else {
    printf("Invalid expression: %s\n", args);  // 输出错误信息
  }
  return 0;
}

/* cmd_w：设置新的监视点 */
static int cmd_w(char *args) {
  if (args == NULL) {
    printf("Usage: w EXPR\n");
    return 0;
  }

  // 添加监视点
  bool success = false; 
  vaddr_t addr = expr(args, &success);
  if (success) {
    wp_add(addr);  // 添加监视点
  } else {
    printf("Invalid expression: %s\n", args);
  }
  return 0;
}

static int cmd_d(char *args){
  if (args == NULL) {
    printf("Usage: d N\n");
    return 0;
  }

  // 提取 N 参数
  char *n_str = strtok(args, " ");
  if (n_str == NULL) {
    printf("Usage: d N\n");
    return 0;
  }

  // 将 N 转换为整数
  char *endptr;
  long n = strtol(n_str, &endptr, 10);
  if (*endptr != '\0' || n < 0) {
    printf("Invalid value for N. Please provide a positive integer.\n");
    return 0;
  }

  // 删除监视点
  wp_delete(n);

  return 0;
}

/* 命令 'help'：显示所有支持的命令信息 */
static int cmd_help(char *args);

/* 命令表，包含命令名称、描述和处理函数 */
static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "显示所有支持的命令的信息", cmd_help },
  { "c", "继续执行程序", cmd_c },
  { "q", "退出 NEMU", cmd_q },
  { "si", "让程序单步执行N条指令后暂停执行,当N没有给出时, 缺省为1", cmd_si },
  { "info", "打印程序状态, \"info r\": 打印寄存器状态, \"info w\": 打印监视点信息", cmd_info },
  { "x", "扫描内存, 格式: x N EXPR, 从表达式 EXPR 的结果开始读取 N 个 4 字节数据", cmd_x },
  { "p", "计算表达式的值并打印结果", cmd_p },
  { "w", "设置新的监视点", cmd_w },
  { "d", "删除监视点", cmd_d },
};


#define NR_CMD ARRLEN(cmd_table)  // 命令数量

/* 命令 'help' 的处理函数 */
static int cmd_help(char *args) {
  /* 提取第一个参数 */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* 没有提供参数 */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);  // 打印所有命令及其描述
    }
  }
  else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);  // 打印指定命令的描述
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);  // 如果命令不存在，打印错误信息
  }
  return 0;
}

/* 设置简单调试器的批处理模式 */
void sdb_set_batch_mode() {
  is_batch_mode = true;
}

/* 调试器主循环 */
void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);  // 如果是批处理模式，直接继续执行程序
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {  // 循环读取用户输入
    char *str_end = str + strlen(str);

    /* 提取第一个单词作为命令 */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* 剩余部分作为命令的参数 */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();  // 清空 SDL 事件队列（如果启用设备）
#endif

    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }  // 调用对应的命令处理函数
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }  // 如果命令不存在，打印错误信息
  }



}

/* 初始化简单调试器 */
void init_sdb() {
  /* 编译正则表达式 */
  init_regex();

  /* 初始化监视点池 */
  init_wp_pool();
}

/* 工作流程描述：
 * 1. 初始化简单调试器 (init_sdb)：
 *    - 编译正则表达式。
 *    - 初始化监视点池。
 * 2. 进入调试器主循环 (sdb_mainloop)：
 *    - 如果是批处理模式，直接调用 cmd_c() 执行程序。
 *    - 否则，使用 readline 库从标准输入读取命令。
 *    - 提取命令并调用对应的命令处理函数。
 *    - 支持的命令包括 'c' (继续执行)、'q' (退出)、'help' (显示帮助信息)，并可以通过扩展 cmd_table 添加更多命令。
 * 3. 提供批处理模式和交互式模式两种方式：
 *    - 批处理模式通过 sdb_set_batch_mode() 设置，直接执行程序。
 *    - 交互式模式下，用户可以输入各种命令与模拟器交互。
 */
