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

// 初始化监视器
void init_monitor(int, char *[]);
// 初始化AM监视器
void am_init_monitor();
// 启动引擎
void engine_start();
// 检查退出状态是否良好
int is_exit_status_bad();

int main(int argc, char *argv[]) {
  // 初始化监视器
#ifdef CONFIG_TARGET_AM
  // 如果定义了CONFIG_TARGET_AM，则初始化AM监视器
  am_init_monitor();
#else
  // 否则，初始化普通监视器
  init_monitor(argc, argv);
#endif

  // 启动引擎
  engine_start();

  // 返回退出状态是否良好
  return is_exit_status_bad();
}