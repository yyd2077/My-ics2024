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

#ifndef __ISA_H__
#define __ISA_H__

// 包含目标 ISA（Instruction Set Architecture）的定义文件
// 位于 src/isa/$(GUEST_ISA)/include/isa-def.h
#include <isa-def.h>

/*
 * 宏 `__GUEST_ISA__` 在编译选项 (CFLAGS) 中定义。
 * 它将被扩展为 "x86"、"mips32" 等，用于确定当前使用的指令集架构。
 *
 * concat() 宏用于将 `__GUEST_ISA__` 与后缀（例如 _CPU_state）拼接，
 * 以根据不同的 ISA 生成不同的类型。
 */
typedef concat(__GUEST_ISA__, _CPU_state) CPU_state;  // 根据目标 ISA 定义 CPU 状态结构体类型
typedef concat(__GUEST_ISA__, _ISADecodeInfo) ISADecodeInfo;  // 根据目标 ISA 定义解码信息结构体类型

// 监控相关的全局变量和函数声明
extern unsigned char isa_logo[];  // 存储 ISA 相关的 logo 图像数据
void init_isa();  // 初始化 ISA

// 寄存器相关的全局变量和函数声明
extern CPU_state cpu;  // 全局 CPU 状态变量
void isa_reg_display();  // 显示 CPU 寄存器状态
word_t isa_reg_str2val(const char *name, bool *success);  // 根据寄存器名称获取其值

// 执行相关函数声明
struct Decode;
int isa_exec_once(struct Decode *s);  // 执行一条指令并更新解码结构体

// 内存管理相关的枚举定义和函数声明
enum { MMU_DIRECT, MMU_TRANSLATE, MMU_FAIL };  // MMU（内存管理单元）模式：直接访问、翻译访问、失败
enum { MEM_TYPE_IFETCH, MEM_TYPE_READ, MEM_TYPE_WRITE };  // 内存操作类型：指令获取、数据读取、数据写入
enum { MEM_RET_OK, MEM_RET_FAIL, MEM_RET_CROSS_PAGE };  // 内存访问结果：成功、失败、跨页
#ifndef isa_mmu_check
int isa_mmu_check(vaddr_t vaddr, int len, int type);  // 检查虚拟地址是否可通过 MMU 访问
#endif
paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type);  // 将虚拟地址翻译为物理地址

// 中断和异常处理相关的函数声明
vaddr_t isa_raise_intr(word_t NO, vaddr_t epc);  // 触发中断或异常处理
#define INTR_EMPTY ((word_t)-1)  // 表示没有中断的特殊值
word_t isa_query_intr();  // 查询是否有待处理的中断

// 差异测试（Difftest）相关的函数声明
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc);  // 检查寄存器状态是否与参考状态一致
void isa_difftest_attach();  // 附加到差异测试模块

#endif

/*
 * 工作流程描述：
 * 1. 目标 ISA 的定义文件包含和宏定义：
 *    - 包含 isa-def.h 文件，该文件包含与目标指令集相关的定义。
 *    - 使用 concat() 宏来根据目标指令集架构生成 CPU 状态和解码信息的类型。
 * 2. 监控和初始化：
 *    - 提供 init_isa() 函数用于初始化与目标 ISA 相关的资源，例如寄存器、内存等。
 * 3. 寄存器操作：
 *    - 通过全局 CPU_state 结构体变量 cpu 操作 CPU 的寄存器。
 *    - 提供 isa_reg_display() 函数显示寄存器状态，isa_reg_str2val() 函数用于获取寄存器值。
 * 4. 指令执行：
 *    - 通过 isa_exec_once() 函数执行一条指令并更新解码信息。
 * 5. 内存管理：
 *    - 提供内存管理单元 (MMU) 相关的枚举和函数，用于处理虚拟地址到物理地址的转换和内存访问。
 * 6. 中断和异常处理：
 *    - 使用 isa_raise_intr() 触发中断或异常处理，isa_query_intr() 查询是否有待处理的中断。
 * 7. 差异测试：
 *    - 提供差异测试函数 isa_difftest_checkregs()，用于检查寄存器状态是否与参考状态一致。
 *    - 通过 isa_difftest_attach() 函数将系统附加到差异测试模块中。
 */
