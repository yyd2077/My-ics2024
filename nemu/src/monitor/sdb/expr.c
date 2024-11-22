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

/* 使用 POSIX 正则表达式库来处理正则表达式。
 * 输入 'man regex' 可以获取更多关于 POSIX 正则表达式函数的信息。
 */
#include <regex.h>

// 定义枚举类型，用于表示不同的 token 类型
enum {
  TK_NOTYPE = 256, TK_EQ,  // TK_NOTYPE 表示空白符，TK_EQ 表示 "=="

  /* TODO: 添加更多的 token 类型 */

};

// 定义规则结构体，包含正则表达式字符串和对应的 token 类型
static struct rule {
  const char *regex;  // 正则表达式字符串
  int token_type;     // 对应的 token 类型
} rules[] = {

  /* TODO: 添加更多的规则。
   * 注意不同规则的优先级。
   */

  {" +", TK_NOTYPE},    // 空白符
  {"\\+", '+'},         // 加号
  {"==", TK_EQ},        // 等号 "=="
};

#define NR_REGEX ARRLEN(rules)  // 计算规则数组的大小

// 定义正则表达式数组，用于存储编译后的正则表达式
static regex_t re[NR_REGEX] = {};

/* 规则会被多次使用，因此在使用前只编译一次。 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);  // 编译正则表达式
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);  // 如果编译失败，输出错误信息
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

// 定义 token 结构体，用于存储解析得到的 token
typedef struct token {
  int type;        // token 类型
  char str[32];    // token 的字符串表示（最多 32 个字符）
} Token;

// 定义 token 数组和 token 数量，用于存储解析得到的 token
static Token tokens[32] __attribute__((used)) = {};  // 最多存储 32 个 token
static int nr_token __attribute__((used))  = 0;  // 当前 token 数量

/*
 * make_token: 从输入表达式中生成 token。
 * @param e: 输入的表达式字符串。
 * @return: 如果成功解析所有 token，返回 true；否则返回 false。
 */
static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;  // 初始化 token 数量为 0

  while (e[position] != '\0') {
    /* 尝试逐个匹配所有规则 */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;  // 更新当前解析的位置

        /* TODO: 现在已经识别到一个新的 token。添加代码将 token 记录到 `tokens` 数组中。
         * 对于某些特定类型的 token，可能需要执行额外的操作。
         */

        switch (rules[i].token_type) {
          default: TODO();  // 目前未实现，需补充完整逻辑
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");  // 没有匹配的规则，打印错误信息
      return false;
    }
  }

  return true;
}

/*
 * expr: 计算表达式的值。
 * @param e: 输入的表达式字符串。
 * @param success: 用于指示计算是否成功。
 * @return: 计算结果，如果失败则返回 0。
 */
word_t expr(char *e, bool *success) {
  if (!make_token(e)) {  // 首先将表达式解析为 token
    *success = false;
    return 0;
  }

  /* TODO: 添加代码来计算表达式的值 */
  TODO();

  return 0;
}

/*
 * 工作流程描述：
 * 1. 初始化正则表达式 (init_regex)：
 *    - 将所有规则中的正则表达式进行编译，以便后续用于解析输入表达式。
 * 2. 解析表达式 (make_token)：
 *    - 输入一个表达式字符串，将其解析为一系列 token，存储在 tokens 数组中。
 *    - 逐个尝试匹配所有定义的正则表达式规则。
 *    - 如果匹配成功，则记录该 token，并继续解析下一个部分。
 *    - 如果未匹配到任何规则，则输出错误信息并返回失败。
 * 3. 计算表达式 (expr)：
 *    - 首先调用 make_token 将表达式解析为 token。
 *    - 然后对 token 进行处理，计算表达式的值（当前未实现）。
 */

