/***************************************************************************************
* 版权所有 (c) 2014-2024 Zihao Yu, 南京大学
*
* NEMU遵循Mulan PSL v2.0开源许可协议。
* 您可以根据Mulan PSL v2.0的条款和条件使用此软件。
* 您可以在以下链接获取Mulan PSL v2.0的副本：
*          http://license.coscl.org.cn/MulanPSL2
*
* 此软件按“原样”提供，不附带任何明示或暗示的保证，
* 包括但不限于非侵权、适销性或特定用途适用性。
*
* 有关详细信息，请参阅Mulan PSL v2.0。
***************************************************************************************/

#include <common.h>

// 声明函数的原型
void init_monitor(int, char *[]);  // 初始化监控程序函数
void am_init_monitor();            // 针对 AM 目标的初始化监控程序函数
void engine_start();               // 启动引擎的函数
int is_exit_status_bad();          // 检查程序退出状态是否异常的函数

int main(int argc, char *argv[]) {
  /* 初始化监控程序 */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();  // 如果定义了 CONFIG_TARGET_AM，则调用 am_init_monitor() 进行初始化
#else
  init_monitor(argc, argv);  // 否则调用 init_monitor()，并传入命令行参数
#endif

  /* 启动引擎 */
  engine_start();  // 调用 engine_start() 函数启动模拟器引擎

  return is_exit_status_bad();  // 返回退出状态，判断是否存在异常
}

/* 工作流程描述：
 * 1. main() 函数接受命令行参数，并首先根据是否定义了 CONFIG_TARGET_AM 来选择初始化监控程序的方式。
 *    - 如果 CONFIG_TARGET_AM 被定义，则调用 am_init_monitor() 函数，这是针对 AM 目标的特殊初始化。
 *    - 否则，调用 init_monitor(argc, argv)，用于进行一般的初始化并传入命令行参数。
 * 2. 调用 engine_start() 函数，启动引擎，这通常是开始进行仿真或者计算的核心步骤。
 * 3. 最后调用 is_exit_status_bad() 函数，检查程序的退出状态是否有问题，如果有问题则返回错误。
 */
