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
#include <memory/paddr.h>
#include <memory/vaddr.h>

#define NR_WP 32  // 定义最大监视点数量为 32
// 定义监视点地址的合法范围
#define WATCHPOINT_ADDR_MIN 0x80000000
#define WATCHPOINT_ADDR_MAX 0x87ffffff

/*
 * 监视点结构体定义。
 * @NO: 监视点编号。
 * @next: 指向下一个监视点的指针。
 *
 * 监视点用于在程序执行过程中监控某个变量或内存的值是否发生变化，
 * 当值发生变化时，模拟器会暂停执行，方便调试。
 */
typedef struct watchpoint {
  int NO;            // 监视点编号
  struct watchpoint *next; // 指向下一个监视点
  vaddr_t addr;      // 监视的内存地址
  uint32_t Watchpoint_value; // 存储监视点的旧值
} WP;

// 监视点池，包含最多 NR_WP 个监视点
static WP wp_pool[NR_WP] = {};
// 已激活的监视点链表头指针和空闲监视点链表头指针
static WP *head = NULL, *free_ = NULL;

/*
 * init_wp_pool: 初始化监视点池。
 *
 * 初始化所有监视点，将所有监视点加入空闲链表，并设置编号。
 */
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i++) {
    wp_pool[i].NO = i;  // 设置监视点编号
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);  // 将监视点链接成一个链表
  }

  head = NULL;  // 激活的监视点链表初始化为空
  free_ = wp_pool;  // 空闲链表指向整个监视点池
}

/*
 * wp_add: 添加一个新的监视点。
 *
 * @addr: 需要监视的内存地址。
 */
void wp_add(vaddr_t addr) {
  // 检查地址是否在合法范围内
  if (addr < WATCHPOINT_ADDR_MIN || addr > WATCHPOINT_ADDR_MAX) {
    printf("Error: Watchpoint address 0x%x is out of range. Valid range is [0x%x, 0x%x].\n",
           addr, WATCHPOINT_ADDR_MIN, WATCHPOINT_ADDR_MAX);
    return;
  }

  // 检查当前激活的监视点数量
  int count = 0;
  WP *current = head;
  while (current != NULL) {
    count++;
    current = current->next;
  }

  // 如果监视点数量已经达到最大值（32），则不能添加新监视点
  if (count >= NR_WP) {
    printf("Cannot add new watchpoint. Maximum number of watchpoints reached (%d).\n", NR_WP);
    return;
  }

  // 如果没有可用的监视点，则返回
  if (free_ == NULL) {
    printf("No available watchpoints!\n");
    return;
  }

  // 从空闲链表中获取一个监视点并更新空闲链表
  WP *wp = free_;
  free_ = free_->next;

  wp->next = head;  // 将新监视点添加到激活链表头部
  head = wp;

  wp->addr = addr;  // 设置监视点地址
  wp->Watchpoint_value = vaddr_read(addr,4);  
  printf("Watchpoint %d added at address %#x\n", wp->NO, addr);
  printf("Watchpoint value: 0x%x\n", wp->Watchpoint_value);
}

void wp_display() {
  if (head == NULL) {
    printf("No active watchpoints.\n");
    return;
  }

  WP *current = head;
  printf("Num  Watchpoint\n");
  while (current != NULL) {
    // 使用 %#x 打印地址
    printf("%-4d at address %#x\n", current->NO, current->addr);
    current = current->next;
  }
}

/*
 * wp_delete: 删除指定序号的监视点。
 *
 * @N: 要删除的监视点序号。
 */
void wp_delete(int N) {
  WP *prev = NULL, *current = head;

  // 遍历激活链表，查找指定序号的监视点
  while (current != NULL) {
    if (current->NO == N) {
      // 如果是头结点
      if (prev == NULL) {
        head = current->next;
      } else {
        prev->next = current->next;
      }

      // 将该监视点放回空闲链表
      current->next = free_;
      free_ = current;

      // 打印删除的监视点信息
      printf("Watchpoint %d deleted.\n", N);
      return;
    }

    prev = current;
    current = current->next;
  }

  // 如果没有找到监视点，打印错误信息
  printf("Watchpoint %d not found.\n", N);
}

//监视点检查函数
void check_wp() {
  WP *current = head;
  while (current != NULL) {
    uint32_t new_value = vaddr_read(current->addr, 4);

    if (new_value != current->Watchpoint_value) {
      // 打印监视点信息，使用正确的格式符
      printf("Watchpoint %d: value at address 0x%08x changed from 0x%08x to 0x%08x\n",
             current->NO, current->addr, current->Watchpoint_value, new_value);

      //  nemu_state.state
      nemu_state.state = NEMU_STOP;
      return;
    }

    current = current->next;
  }
}



/* TODO: 实现监视点的功能
 *
 * 监视点的功能通常包括：
 * 1. 设置新的监视点：从空闲链表中取出一个监视点，并设置监视的表达式或内存地址。
 * 2. 删除监视点：将指定的监视点从激活链表中移除，并放回空闲链表。
 * 3. 检查监视点：在每次程序执行中断时，检查所有激活的监视点，看其值是否发生变化。
 *
 * 当前代码仅完成了监视点池的初始化，监视点的其他功能还需进一步实现。
 */

/* 工作流程描述：
 * 1. 初始化监视点池 (init_wp_pool)：
 *    - 创建一个具有 NR_WP 个监视点的数组 wp_pool，并将所有监视点链接成一个空闲链表。
 *    - 每个监视点通过 next 指针指向下一个监视点，最后一个监视点的 next 指向 NULL。
 *    - 将 free_ 指针指向空闲链表的头部，表示所有监视点当前都是可用的。
 *    - 将 head 指针设置为 NULL，表示当前没有激活的监视点。
 * 2. 监视点功能实现（未完成）：
 *    - 监视点用于调试程序，当程序执行到某个内存位置或变量值发生变化时触发。
 *    - 后续需要实现设置、删除和检查监视点的功能。
 */
