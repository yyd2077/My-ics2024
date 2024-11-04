/***************************************************************************************
* 版权所有 (c) 2014-2024 Zihao Yu, 南京大学
*
* NEMU遵循Mulan PSL v2.0协议。
* 您可以根据Mulan PSL v2.0协议的条款和条件使用此软件。
* 您可以在以下地址获取Mulan PSL v2.0协议的副本：
*          http://license.coscl.org.cn/MulanPSL2
*
* 此软件按“原样”提供，不附带任何明示或暗示的保证，
* 包括但不限于非侵权、适销性或适用于特定目的的保证。
*
* 请参阅Mulan PSL v2.0协议以获取更多详细信息。
***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm();

// 欢迎信息
static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("如果启用了跟踪，将生成一个日志文件来记录跟踪。这可能会导致日志文件非常大。"
        "如果不需要，可以在menuconfig中禁用它"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
  //Log("Exercise: Please remove me in the source code and compile NEMU again.");
  //assert(0);
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

// 设置批处理模式
void sdb_set_batch_mode();

// 日志文件
static char *log_file = NULL;
// 差分测试文件
static char *diff_so_file = NULL;
// 图像文件
static char *img_file = NULL;
// 差分测试端口
static int difftest_port = 1234;

// 加载图像
static long load_img() {
  if (img_file == NULL) {
    Log("没有给出图像。使用默认的内置图像。");
    return 4096; // 内置图像大小
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "无法打开'%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("图像为%s，大小为%ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}

// 解析参数
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
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 1: img_file = optarg; return 0;
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

// 初始化监视器
void init_monitor(int argc, char *argv[]) {
  /* 执行一些全局初始化。 */

  /* 解析参数。 */
  parse_args(argc, argv);

  /* 设置随机种子。 */
  init_rand();

  /* 打开日志文件。 */
  init_log(log_file);

  /* 初始化内存。 */
  init_mem();

  /* 初始化设备。 */
  IFDEF(CONFIG_DEVICE, init_device());

  /* 执行与ISA相关的初始化。 */
  init_isa();

  /* 将图像加载到内存中。这将覆盖内置图像。 */
  long img_size = load_img();

  /* 初始化差分测试。 */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* 初始化简单调试器。 */
  init_sdb();

  IFDEF(CONFIG_ITRACE, init_disasm());

  /* 显示欢迎信息。 */
  welcome();
}
#else // CONFIG_TARGET_AM
// 加载图像
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

// 初始化监视器
void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
