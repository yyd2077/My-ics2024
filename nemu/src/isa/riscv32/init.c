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
#include <memory/paddr.h>

/*
 * 定义了一个静态常量数组 img，用于存储内置的指令和数据。
 * 这些数据是以 32 位无符号整数形式表示的。
 * 尽管与 uint8_t 类型不一致，但由于不直接访问该数组，所以这样定义是可以的。
 */
static const uint32_t img [] = {
  0x00000297,  // auipc t0,0  （将当前 PC 加上立即数 0，结果存入寄存器 t0）
  0x00028823,  // sb  zero,16(t0)  （将寄存器 zero 的值存入地址 t0 + 16 的内存位置）
  0x0102c503,  // lbu a0,16(t0)  （从地址 t0 + 16 处加载一个字节到寄存器 a0）
  0x00100073,  // ebreak (used as nemu_trap) （触发软件中断，这里用作 NEMU 的陷阱指令）
  0xdeadbeef,  // some data （一些数据，用于后续读取）
};

/*
 * restart: 设置 CPU 的初始状态。
 *
 * 设置程序计数器（PC）为 RESET_VECTOR，表示从系统复位向量处开始执行。
 * 将寄存器 gpr[0]（即零号寄存器）设为 0，因为 RISC-V 的零号寄存器总是等于 0。
 */
static void restart() {
  /* 设置初始的程序计数器。 */
  cpu.pc = RESET_VECTOR;

  /* 零号寄存器的值总是 0。 */
  cpu.gpr[0] = 0;
}

/*
 * init_isa: 初始化 ISA（指令集架构）。
 *
 * 将内置的镜像（img）加载到访客内存的 RESET_VECTOR 地址处，
 * 作为系统的初始程序。
 * 调用 restart() 函数设置 CPU 的初始状态。
 */
void init_isa() {
  /* 加载内置镜像到访客内存的 RESET_VECTOR 地址。 */
  memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));

  /* 初始化虚拟计算机系统。 */
  restart();
}

/*
 * 工作流程描述：
 * 1. 初始化指令集架构 (init_isa)：
 *    - 将预定义的内置镜像（指令和数据）加载到访客内存的起始地址（RESET_VECTOR）。
 *    - 这些指令包括简单的指令，如加载、存储和 ebreak，用于模拟一个小程序。
 * 2. 设置 CPU 的初始状态 (restart)：
 *    - 设置程序计数器（PC）为 RESET_VECTOR，表示从系统复位向量处开始执行。
 *    - 将寄存器 gpr[0] 设为 0，这是 RISC-V 架构的特性，gpr[0] 总是等于 0。
 *
 * 此代码的主要目的是加载初始程序并设置 CPU 的初始状态，以便模拟器可以开始执行该程序。
 */

