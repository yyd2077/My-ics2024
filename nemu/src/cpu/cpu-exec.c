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

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>

/*
 * MAX_INST_TO_PRINT: 当执行的指令数小于该值时，指令的汇编代码会输出到屏幕上。
 * 可以通过修改此值来控制输出的指令数量，适用于 `si`（单步执行）命令的调试。
 */
#define MAX_INST_TO_PRINT 10

// CPU 状态结构体，存储 CPU 的寄存器和状态
CPU_state cpu = {};

// 全局变量，用于记录已经执行的访客指令数量
uint64_t g_nr_guest_inst = 0;

// 用于记录执行的时间，单位为微秒（us）
static uint64_t g_timer = 0;

// 控制是否逐步打印指令的执行过程
static bool g_print_step = false;

// 声明设备更新函数
void device_update();

/*
 * trace_and_difftest: 进行指令的跟踪和差分测试。
 * @param _this: 包含当前指令信息的 Decode 结构体。
 * @param dnpc: 指令执行后的下一条指令的地址。
 */
static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { log_write("%s\n", _this->logbuf); }  // 根据条件记录指令的执行日志
#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }  // 如果 g_print_step 为 true，打印指令信息
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));  // 进行差分测试，检查当前 PC 和下一条指令地址是否一致
}

/*
 * exec_once: 执行一条指令。
 * @param s: 用于存储当前指令的 Decode 结构体。
 * @param pc: 当前指令的程序计数器（PC）地址。
 */
static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;  // 设置当前指令的 PC 地址
  s->snpc = pc;  // 设置下一条指令的地址（初始为当前 PC）
  isa_exec_once(s);  // 执行指令并更新 Decode 结构体中的相关信息
  cpu.pc = s->dnpc;  // 更新 CPU 的程序计数器为当前指令执行后的下一条指令地址

#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);  // 将指令的地址写入日志缓冲区

  int ilen = s->snpc - s->pc;  // 计算指令长度
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst;

#ifdef CONFIG_ISA_x86
  for (i = 0; i < ilen; i ++) {
#else
  for (i = ilen - 1; i >= 0; i --) {
#endif
    p += snprintf(p, 4, " %02x", inst[i]);  // 将指令的字节码写入日志缓冲区
  }

  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);  // 最大指令长度（根据架构不同有所区别）
  int space_len = ilen_max - ilen;  // 计算需要补齐的空格长度
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);  // 在日志缓冲区中填充空格
  p += space_len;

  // 反汇编当前指令并写入日志缓冲区
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst, ilen);
#endif
}

/*
 * execute: 执行指定数量的指令。
 * @param n: 要执行的指令数量。
 */
static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) {
    exec_once(&s, cpu.pc);  // 执行一条指令
    g_nr_guest_inst ++;  // 记录已执行的访客指令数量
    trace_and_difftest(&s, cpu.pc);  // 进行指令的跟踪和差分测试
    if (nemu_state.state != NEMU_RUNNING) break;  // 如果 NEMU 状态不是运行状态，则停止执行
    IFDEF(CONFIG_DEVICE, device_update());  // 如果启用了设备，更新设备状态
  }
}

/*
 * statistic: 打印模拟器的统计信息。
 */
static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));  // 设置区域信息，便于格式化输出数字
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);  // 输出主机花费的时间
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);  // 输出访客指令总数
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);  // 计算模拟频率
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

/*
 * assert_fail_msg: 当程序断言失败时，打印寄存器状态和统计信息。
 */
void assert_fail_msg() {
  isa_reg_display();  // 显示寄存器信息
  statistic();  // 显示统计信息
}

/*
 * cpu_exec: 模拟 CPU 的工作过程，执行指定数量的指令。
 * @param n: 要执行的指令数量。
 */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);  // 如果执行的指令数少于 MAX_INST_TO_PRINT，则逐步打印指令
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT: case NEMU_QUIT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();  // 获取开始时间

  execute(n);  // 执行指令

  uint64_t timer_end = get_time();  // 获取结束时间
  g_timer += timer_end - timer_start;  // 计算执行时间

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: case NEMU_ABORT:
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic();
  }
}

/*
 * 工作流程描述：
 * 1. cpu_exec() 函数用于模拟 CPU 的工作，接受指令数量 n 作为参数。
 *    - 如果 n 小于 MAX_INST_TO_PRINT，则设置 g_print_step 为 true，逐步打印指令信息。
 *    - 根据当前 NEMU 的状态，判断是否可以开始执行，如果已经结束，则打印提示信息并退出。
 * 2. 调用 execute() 函数，执行指定数量的指令：
 *    - 每次执行一条指令，通过 exec_once() 完成指令的解码和执行。
 *    - 执行过程中更新已执行的指令数量并进行指令跟踪和差分测试。
 *    - 如果 NEMU 的状态变为非运行状态，则停止执行。
 * 3. 执行完成后，更新执行时间，调用 statistic() 函数打印执行统计信息，包括：
 *    - 主机花费的时间。
 *    - 访客指令总数。
 *    - 模拟器的执行频率。
 */
