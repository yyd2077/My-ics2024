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
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>

// typedef MUXDEF(CONFIG_ISA64, uint64_t, uint32_t) word_t;

/* 定义枚举类型，用于表示不同的 Token 类型 */
enum {
  TK_NOTYPE = 256,  // 空白符
  TK_EQ,            // 等号 "=="
  TK_NEQ,           // 不等号 "!="
  TK_AND,           // 逻辑与 "&&"
  TK_OR,            // 逻辑或 "||"
  TK_NUM,           // 数字
  TK_HEX,           // 十六进制数字
  TK_REG,           // 寄存器
  TK_DEREF,         // 解引用 *
  TK_NEG,           // 负号（单目运算符）-
  /* 其他操作符直接使用其对应的ASCII码，如 '+'，'-'，'*'，'/'，'('，')' */
};

/* 定义规则结构体，包含正则表达式字符串和对应的 Token 类型 */
static struct rule {
  const char *regex;  // 正则表达式字符串
  int token_type;     // 对应的 Token 类型
} rules[] = {
  {" +", TK_NOTYPE},             // 空白符
  {"\\+", '+'},                  // 加号 '+'
  {"\\-", '-'},                  // 减号 '-'
  {"\\*", '*'},                  // 乘号 '*'
  {"\\/", '/'},                  // 除号 '/'
  {"\\(", '('},                  // 左括号 '('
  {"\\)", ')'},                  // 右括号 ')'
  {"==", TK_EQ},                 // 等号 '=='
  {"!=", TK_NEQ},                // 不等号 '!='
  {"&&", TK_AND},                // 逻辑与 '&&'
  {"\\|\\|", TK_OR},             // 逻辑或 '||'
  {"0x[0-9a-fA-F]+", TK_HEX},    // 十六进制数字
  {"[0-9]+", TK_NUM},            // 十进制数字
  {"\\$\\$0", TK_REG},         // 处理 $$0
  {"\\$[a-zA-Z][a-zA-Z0-9]*", TK_REG},  // 匹配其他寄存器名
  {"!", '!'},                    // 逻辑非 '!'
  {"&&", TK_AND},                // 逻辑与 '&&'
  {"\\|\\|", TK_OR},             // 逻辑或 '||'
};

/* 计算规则数组的大小 */
#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

/* 定义正则表达式数组，用于存储编译后的正则表达式 */
static regex_t re[NR_REGEX];

/* 初始化正则表达式
 * 该函数会将规则数组中的每个正则表达式进行编译，并将编译后的结果存储在 re 数组中，
 * 以供后续解析表达式时使用。
 */
void init_regex() {
  int ret;
  char error_msg[128];

  // 编译每个规则中的正则表达式
  for (int i = 0; i < NR_REGEX; i++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      // 如果编译失败，输出错误信息
      regerror(ret, &re[i], error_msg, sizeof(error_msg));
      panic("正则表达式编译失败: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

/* 定义 Token 结构体，用于存储解析得到的 Token */
typedef struct token {
  int type;          // Token 类型
  char str[32];      // Token 的字符串值
} Token;

/* 定义 Token 数组和 Token 数量，用于存储解析得到的 Token */
static Token tokens[64];  // 最多存储 64 个 Token
static int nr_token;      // 当前 Token 数量

/* 从输入表达式中生成 Token 序列
 * 该函数将输入的表达式字符串按照规则进行分词，并将每个 Token 存储在 tokens 数组中。
 * 每个 Token 包含一个类型和对应的字符串值。
 * 如果输入表达式无法匹配任何规则，则返回 false。
 */
static bool make_token(char *e) {
  int position = 0;
  regmatch_t pmatch;

  nr_token = 0;  // 初始化 Token 数量为 0

  while (e[position] != '\0') {
    bool matched = false;

    // 尝试逐个匹配所有规则
    for (int i = 0; i < NR_REGEX; i++) {
      // 使用 regexec 函数匹配正则表达式
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // 更新当前解析的位置
        position += substr_len;
        matched = true;

        // 如果匹配到空白符，直接跳过
        if (rules[i].token_type == TK_NOTYPE) {
          break;
        }

        // 记录当前 Token
        Token *tk = &tokens[nr_token++];
        tk->type = rules[i].token_type;

        // 对于数字、十六进制数和寄存器，需要保存其字符串值
        if (tk->type == TK_NUM || tk->type == TK_HEX) {
          memset(tk->str, 0, sizeof(tk->str));
          strncpy(tk->str, substr_start, substr_len > 31 ? 31 : substr_len);
        } else if (tk->type == TK_REG) {
          memset(tk->str, 0, sizeof(tk->str));
          strncpy(tk->str, substr_start, substr_len > 31 ? 31 : substr_len);

          // 特殊处理 $$0 的情况
          if (strncmp(tk->str, "$$0", 3) == 0) {
            // 将 $$0 直接解析为数值 0
            tk->type = TK_NUM;  // 将它当做一个数字处理
            strcpy(tk->str, "0");  // 设置为 "0"
          }
        }

        break;
      }
    }

    // 如果没有匹配的规则，输出错误信息
    if (!matched) {
      printf("无法解析的符号: '%c' at position %d\n", e[position], position);
      return false;
    }
  }

  // 处理单目运算符（如解引用 '*' 和负号 '-'）
  for (int i = 0; i < nr_token; i++) {
    // 判断 '*' 是否为解引用操作符
    if (tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != TK_NUM && tokens[i - 1].type != TK_HEX && tokens[i - 1].type != TK_REG && tokens[i - 1].type != ')'))) {
      tokens[i].type = TK_DEREF;  // 将 '*' 标记为解引用操作符
    }
    // 判断 '-' 是否为负号（单目运算符）
    if (tokens[i].type == '-' && (i == 0 || (tokens[i - 1].type != TK_NUM && tokens[i - 1].type != TK_HEX && tokens[i - 1].type != TK_REG && tokens[i - 1].type != ')'))) {
      tokens[i].type = TK_NEG;    // 将 '-' 标记为负号（单目运算符）
    }
  }

  return true;
}

/* 前向声明求值函数 */
static word_t eval(int p, int q, bool *success);

/* 计算表达式的值
 * 该函数首先调用 make_token 函数将输入的表达式字符串解析为 Token 序列。
 * 然后，使用递归下降算法计算该表达式的值。
 * 如果计算成功，返回计算结果，否则返回 0。
 */
word_t expr(char *e, bool *success) {
  if (!make_token(e)) {  // 如果解析 Token 失败，返回失败
    *success = false;
    return 0;
  }

  *success = true;
  return eval(0, nr_token - 1, success);  // 计算表达式的值
}

/* 判断是否为括号包围的表达式
 * 该函数用于判断一个表达式是否被括号包围。
 * 如果是，返回 true；否则返回 false。
 */
static bool check_parentheses(int p, int q, bool *success) {
  if (tokens[p].type != '(' || tokens[q].type != ')') {
    return false;
  }

  int count = 0;
  for (int i = p + 1; i < q; i++) {
    if (tokens[i].type == '(') count++;
    if (tokens[i].type == ')') count--;
    if (count < 0) {
      *success = false;
      printf("括号不匹配\n");
      return false;
    }
  }
  if (count != 0) {
    *success = false;
    printf("括号不匹配\n");
    return false;
  }
  return true;
}

/* 获取运算符的优先级
 * 返回给定运算符的优先级。优先级越高，返回值越大。
 */
static int get_precedence(int type) {
  switch (type) {
    case TK_OR: return 1;  // 逻辑或
    case TK_AND: return 2; // 逻辑与
    case TK_EQ: 
    case TK_NEQ: return 3;  // 等号和不等号
    case '+':
    case '-': return 4;  // 加减运算
    case '*':
    case '/': return 5;  // 乘除运算
    case TK_DEREF:  // 解引用运算符
    case TK_NEG: return 6;  // 单目负号运算符
    default: return 0;  // 默认优先级
  }
}

/* 计算表达式的值（递归函数）
 * 该函数使用递归的方式来计算表达式的值。
 * 它首先检查是否是单目操作符，然后递归处理操作数。
 * 处理不同类型的操作符时，分情况处理二目运算符和单目运算符。
 */
static word_t eval(int p, int q, bool *success) {
  // 处理括号包围的表达式
  if (check_parentheses(p, q, success)) {
    return eval(p + 1, q - 1, success);  // 递归处理括号内的表达式
  }

  // 如果只有一个 Token，直接返回该 Token 的值
  if (p == q) {
    Token *tk = &tokens[p];
    switch (tk->type) {
      case TK_NUM: 
        return strtol(tk->str, NULL, 10);  // 十进制数字
      case TK_HEX:
        return strtol(tk->str + 2, NULL, 16);  // 十六进制数字（跳过 "0x" 前缀）
      case TK_REG: {
        word_t reg_val;
        // 获取寄存器的值
        reg_val = isa_reg_str2val(tk->str + 1, success);
        if (!*success) {
          return 0;
        }
        return reg_val;
      }
      default:
        *success = false;
        return 0;
    }
  }

  // 递归处理二目运算符
  int op = -1;
  int level = -1;

  // 查找优先级最低的运算符
  for (int i = p; i <= q; i++) {
    int cur_level = get_precedence(tokens[i].type);
    if (cur_level > level) {
      op = i;
      level = cur_level;
    }
  }

  // 如果没有找到运算符，说明表达式出错
  if (op == -1) {
    *success = false;
    return 0;
  }

  // 处理单目运算符
  if (tokens[op].type == TK_NEG || tokens[op].type == TK_DEREF) {
    word_t val = eval(op + 1, q, success);
    switch (tokens[op].type) {
      case TK_NEG:
        return -val;
      case TK_DEREF:
        return vaddr_read(val, 4);  // 解引用操作
      default:
        *success = false;
        return 0;
    }
  }

  // 处理二目运算符
  word_t val1 = eval(p, op - 1, success);
  word_t val2 = eval(op + 1, q, success);
  if (!*success) return 0;

  switch (tokens[op].type) {
    case '+': return val1 + val2;
    case '-': return val1 - val2;
    case '*': return val1 * val2;
    case '/': return val1 / val2;
    case TK_EQ: return val1 == val2;
    case TK_NEQ: return val1 != val2;
    case TK_AND: return val1 && val2;
    case TK_OR: return val1 || val2;
    default:
      *success = false;
      return 0;
  }
}

// 注释和工作流程说明：
// 正则表达式初始化： init_regex()函数加载并编译所有的规则，通过 regcomp 函数处理规则数组 rules 中的正则表达式。若遇到编译错误，会打印错误信息并退出程序。

// Token 生成： make_token()函数负责从输入字符串中提取符合规则的 Token。此过程涉及到逐字符扫描和正则表达式匹配。解析成功后，将每个 Token 存储在 tokens 数组中。

// 表达式求值： eval() 递归地计算表达式的值。首先处理括号和单目运算符，然后递归处理二目运算符。对于每个 Token，根据其类型执行相应操作。

// 单目和二目运算符处理： 在递归过程中，识别并处理加法、减法、乘法等二目运算符，以及负号和解引用等单目运算符。

// 优先级计算： get_precedence() 根据运算符的类型返回其优先级，用于帮助找到表达式中优先级最低的操作符，确保运算顺序正确。