#***************************************************************************************
# Copyright (c) 2014-2024 Zihao Yu, Nanjing University
#
# NEMU is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
# See the Mulan PSL v2 for more details.
#**************************************************************************************/

# 添加源文件到 SRCS-y
SRCS-y += src/nemu-main.c  # 将 src/nemu-main.c 添加到源文件列表中

# 添加目录到 DIRS-y
DIRS-y += src/cpu src/monitor src/utils  # 将 src/cpu、src/monitor 和 src/utils 目录添加到目录列表中

# 如果 CONFIG_MODE_SYSTEM 被定义，则添加内存模块目录
DIRS-$(CONFIG_MODE_SYSTEM) += src/memory  # 根据 CONFIG_MODE_SYSTEM 的值决定是否添加 src/memory 目录

# 如果目标是 AM，则将调试器目录加入黑名单
DIRS-BLACKLIST-$(CONFIG_TARGET_AM) += src/monitor/sdb  # 如果是 CONFIG_TARGET_AM，禁止使用 src/monitor/sdb 目录

# 根据 CONFIG_TARGET_SHARE 设置 SHARE 变量
SHARE = $(if $(CONFIG_TARGET_SHARE),1,0)  # 如果 CONFIG_TARGET_SHARE 被定义，SHARE 为 1，否则为 0

# 根据 CONFIG_TARGET_NATIVE_ELF 设置库链接选项
LIBS += $(if $(CONFIG_TARGET_NATIVE_ELF),-lreadline -ldl -pie,)  # 如果定义了 CONFIG_TARGET_NATIVE_ELF，则链接 readline、dl 库和 PIE 可执行文件支持

# 如果 mainargs 被定义，添加路径参数到汇编标志
ifdef mainargs
ASFLAGS += -DBIN_PATH="$(mainargs)"  # 如果 mainargs 被定义，则将其作为 BIN_PATH 添加到汇编选项中
endif

# 如果目标是 AM，则添加汇编源文件
SRCS-$(CONFIG_TARGET_AM) += src/am-bin.S  # 如果是 CONFIG_TARGET_AM，添加 src/am-bin.S 到源文件列表中

# 声明伪目标
.PHONY: src/am-bin.S  # 声明 src/am-bin.S 为伪目标，避免与实际文件冲突

# /* 工作流程描述：
#  * 1. 添加 src/nemu-main.c 到源文件列表 SRCS-y 中，确保此文件被编译。
#  * 2. 将指定的目录添加到 DIRS-y 中，以确保这些目录下的源文件参与编译过程。
#  * 3. 如果定义了 CONFIG_MODE_SYSTEM，则 src/memory 目录也会被添加到 DIRS 列表中。
#  * 4. 如果 CONFIG_TARGET_AM 被定义，src/monitor/sdb 会被添加到黑名单目录列表中，防止该目录下的文件参与编译。
#  * 5. 使用 CONFIG_TARGET_SHARE 变量来决定 SHARE 的值，SHARE 可以用于共享内存或资源的控制。
#  * 6. 根据 CONFIG_TARGET_NATIVE_ELF 是否被定义，决定是否添加 readline、dl 库和 PIE 可执行文件支持的链接选项。
#  * 7. 如果 mainargs 被定义，则添加该参数到 ASFLAGS 中，作为汇编文件的路径。
#  * 8. 如果目标是 AM，则将 src/am-bin.S 添加到源文件列表中进行编译。
#  * 9. 最后声明 src/am-bin.S 为伪目标，防止与同名文件之间的冲突。
#  */

