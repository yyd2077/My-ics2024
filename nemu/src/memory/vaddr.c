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
 * vaddr_ifetch: 从虚拟地址获取指令。
 * @param addr: 虚拟地址。
 * @param len: 要读取的字节数。
 * @return: 读取的数据。
 *
 * 通过调用 paddr_read() 来实现指令的获取，实际上是直接从物理地址读取数据，
 * 因为此时没有启用虚拟地址到物理地址的转换。
 */
word_t vaddr_ifetch(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

/*
 * vaddr_read: 从虚拟地址读取数据。
 * @param addr: 虚拟地址。
 * @param len: 要读取的字节数。
 * @return: 读取的数据。
 *
 * 通过调用 paddr_read() 来读取数据，类似于 vaddr_ifetch，直接使用物理地址读取，
 * 此函数用于从虚拟地址读取普通数据。
 */
word_t vaddr_read(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

/*
 * vaddr_write: 向虚拟地址写入数据。
 * @param addr: 虚拟地址。
 * @param len: 要写入的字节数。
 * @param data: 要写入的数据。
 *
 * 通过调用 paddr_write() 来实现数据写入，同样未进行虚拟地址到物理地址的转换，
 * 直接写入物理内存。
 */
void vaddr_write(vaddr_t addr, int len, word_t data) {
  paddr_write(addr, len, data);
}

/*
 * 工作流程描述：
 * 1. 虚拟地址的指令获取 (vaddr_ifetch)：
 *    - 调用 paddr_read() 从指定的虚拟地址读取指令，长度由参数 len 决定。
 *    - 因为没有实际的虚拟地址到物理地址转换，所以直接操作物理内存。
 * 2. 虚拟地址的数据读取 (vaddr_read)：
 *    - 与 vaddr_ifetch 类似，调用 paddr_read() 从指定的虚拟地址读取数据。
 * 3. 虚拟地址的数据写入 (vaddr_write)：
 *    - 调用 paddr_write() 向指定的虚拟地址写入数据，长度由参数 len 决定。
 *
 * 目前此代码未实现虚拟地址到物理地址的映射转换，所有的虚拟地址操作实际上都是直接访问物理内存。
 */