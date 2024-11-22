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

// 函数声明
void init_rand();                         // 初始化随机数种子
void init_log(const char *log_file);      // 初始化日志系统
void init_mem();                          // 初始化内存
void init_difftest(char *ref_so_file, long img_size, int port);  // 初始化差异测试
void init_device();                       // 初始化设备
void init_sdb();                          // 初始化简单调试器
void init_disasm();                       // 初始化反汇编器

// 静态函数，打印欢迎信息
static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
  //Log("Exercise: Please remove me in the source code and compile NEMU again.");
  //assert(0);
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();                // 设置简单调试器的批处理模式

// 静态变量，用于存储命令行参数
static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static int difftest_port = 1234;

// 加载镜像文件到内存中
static long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // 内置镜像大小为 4096 字节
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}

// 解析命令行参数
static int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;  // 设置批处理模式
      case 'p': sscanf(optarg, "%d", &difftest_port); break;  // 设置差异测试端口
      case 'l': log_file = optarg; break;  // 设置日志文件路径
      case 'd': diff_so_file = optarg; break;  // 设置差异测试参考库文件路径
      case 1: img_file = optarg; return 0;  // 设置镜像文件路径
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

// 初始化监控程序
void init_monitor(int argc, char *argv[]) {
  /* 执行一些全局初始化操作 */

  /* 解析命令行参数 */
  parse_args(argc, argv);

  /* 设置随机数种子 */
  init_rand();

  /* 打开日志文件 */
  init_log(log_file);

  /* 初始化内存 */
  init_mem();

  /* 初始化设备 */
  IFDEF(CONFIG_DEVICE, init_device());

  /* 执行与 ISA 相关的初始化 */
  init_isa();

  /* 加载镜像文件到内存，这将覆盖内置镜像 */
  long img_size = load_img();

  /* 初始化差异测试 */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* 初始化简单调试器 */
  init_sdb();

  IFDEF(CONFIG_ITRACE, init_disasm());

  /* 显示欢迎信息 */
  welcome();
}
#else // CONFIG_TARGET_AM
// 针对 AM 的镜像加载函数
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

// 针对 AM 的监控程序初始化
void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif

/* 工作流程描述：
 * 1. 初始化监控程序，分为普通环境和 AM 目标环境：
 *    - 对于普通环境，通过 init_monitor() 进行初始化：
 *      - 解析命令行参数，设置镜像文件路径、日志文件路径、差异测试库和端口等信息。
 *      - 初始化随机数种子、日志系统、内存、设备（如果启用）和 ISA。
 *      - 加载镜像文件到内存，覆盖内置镜像。
 *      - 初始化差异测试和简单调试器，并显示欢迎信息。
 *    - 对于 AM 目标环境，通过 am_init_monitor() 进行初始化：
 *      - 初始化随机数种子、内存、ISA。
 *      - 加载镜像文件到内存。
 *      - 如果启用了设备，则初始化设备，最后显示欢迎信息。
 */

