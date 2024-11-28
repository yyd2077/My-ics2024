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

#include <memory/host.h>
#include <memory/paddr.h>
#include <device/mmio.h>
#include <isa.h>

/*
 * CONFIG_PMEM_MALLOC: 如果定义了该宏，物理内存 pmem 通过 malloc 动态分配。
 * CONFIG_PMEM_GARRAY: 如果未定义 CONFIG_PMEM_MALLOC，则使用全局数组定义物理内存。
 */
#if   defined(CONFIG_PMEM_MALLOC)
static uint8_t *pmem = NULL;  // 动态分配的物理内存指针
#else // CONFIG_PMEM_GARRAY
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};  // 静态分配的物理内存数组
#endif

/*
 * guest_to_host: 将访客物理地址转换为主机虚拟地址。
 * @param paddr: 访客物理地址。
 * @return: 主机虚拟地址。
 */
uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }

/*
 * host_to_guest: 将主机虚拟地址转换为访客物理地址。
 * @param haddr: 主机虚拟地址。
 * @return: 访客物理地址。
 */
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

/*
 * pmem_read: 从物理内存中读取数据。
 * @param addr: 物理内存地址。
 * @param len: 要读取的字节数。
 * @return: 读取的数据。
 */
static word_t pmem_read(paddr_t addr, int len) {
  printf("Reading memory at address: 0x%08x, length: %d\n", addr, len); 
  word_t ret = host_read(guest_to_host(addr), len);  // 通过转换为主机地址来读取物理内存
  return ret;
}

/*
 * pmem_write: 向物理内存中写入数据。
 * @param addr: 物理内存地址。
 * @param len: 要写入的字节数。
 * @param data: 要写入的数据。
 */
static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);  // 通过转换为主机地址来写入物理内存
}

/*
 * out_of_bound: 处理越界访问的情况。
 * @param addr: 发生越界访问的物理地址。
 */
static void out_of_bound(paddr_t addr) {
  printf("Trying to access address 0x%08x, physical memory range: [0x%08x, 0x%08x]\n", addr, PMEM_LEFT, PMEM_RIGHT);
  
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);  // 打印越界访问的地址及物理内存边界
}

/*
 * init_mem: 初始化物理内存。
 */
void init_mem() {
#if   defined(CONFIG_PMEM_MALLOC)
  pmem = malloc(CONFIG_MSIZE);  // 动态分配物理内存
  assert(pmem);  // 确保内存分配成功
#endif
  IFDEF(CONFIG_MEM_RANDOM, memset(pmem, rand(), CONFIG_MSIZE));  // 如果启用了 CONFIG_MEM_RANDOM，将物理内存初始化为随机值
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);  // 输出物理内存的范围
}

/*
 * paddr_read: 从物理地址读取数据。
 * @param addr: 物理地址。
 * @param len: 要读取的字节数。
 * @return: 读取的数据。
 */
word_t paddr_read(paddr_t addr, int len) {
  if (likely(in_pmem(addr))) return pmem_read(addr, len);  // 如果地址在物理内存范围内，调用 pmem_read
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));  // 如果地址是设备寄存器地址，调用 mmio_read
  out_of_bound(addr);  // 否则，处理越界访问
  return 0;
}

/*
 * paddr_write: 向物理地址写入数据。
 * @param addr: 物理地址。
 * @param len: 要写入的字节数。
 * @param data: 要写入的数据。
 */
void paddr_write(paddr_t addr, int len, word_t data) {
  if (likely(in_pmem(addr))) { pmem_write(addr, len, data); return; }  // 如果地址在物理内存范围内，调用 pmem_write
  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);  // 如果地址是设备寄存器地址，调用 mmio_write
  out_of_bound(addr);  // 否则，处理越界访问
}

/*
 * 工作流程描述：
 * 1. 初始化物理内存 (init_mem)：
 *    - 如果启用了 CONFIG_PMEM_MALLOC，通过 malloc 动态分配物理内存。
 *    - 如果启用了 CONFIG_MEM_RANDOM，将物理内存初始化为随机值。
 *    - 打印物理内存的范围信息。
 * 2. 地址转换：
 *    - guest_to_host() 用于将访客物理地址转换为主机虚拟地址。
 *    - host_to_guest() 用于将主机虚拟地址转换为访客物理地址。
 * 3. 读写物理内存：
 *    - paddr_read() 从物理地址读取数据：如果地址在物理内存范围内，调用 pmem_read()，否则可能是设备寄存器或越界访问。
 *    - paddr_write() 向物理地址写入数据：如果地址在物理内存范围内，调用 pmem_write()，否则可能是设备寄存器或越界访问。
 * 4. 越界处理：
 *    - 当访问的地址超出物理内存范围时，调用 out_of_bound()，打印错误信息并终止程序。
 */
