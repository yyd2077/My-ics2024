/***************************************************************************************
* 版权所有 (c) 2014-2024 周子豪，南京大学
*
* NEMU遵循Mulan PSL v2.0协议。
* 您可以根据Mulan PSL v2.0协议的条款和条件使用此软件。
* 您可以在以下链接获取Mulan PSL v2.0协议的副本：
*          http://license.coscl.org.cn/MulanPSL2
*
* 此软件按“原样”提供，不附带任何形式的明示或暗示保证，
* 包括但不限于非侵权、适销性或适用于特定目的的保证。
*
* 有关Mulan PSL v2.0协议的详细信息，请参阅Mulan PSL v2.0协议。
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
  /* 初始化监视器。 */
#ifdef CONFIG_TARGET_AM
  // 如果定义了CONFIG_TARGET_AM，则调用am_init_monitor()函数
  am_init_monitor();
#else
  // 否则，调用init_monitor()函数
  init_monitor(argc, argv);
#endif

  /* 启动引擎。 */
  // 调用engine_start()函数
  engine_start();

  // 返回退出状态是否良好
  return is_exit_status_bad();
}