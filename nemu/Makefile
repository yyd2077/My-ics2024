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

# 检查 NEMU_HOME 是否是一个有效的 NEMU 仓库
ifeq ($(wildcard $(NEMU_HOME)/src/nemu-main.c),)
  $(error NEMU_HOME=$(NEMU_HOME) is not a NEMU repo)
endif

# 包含由 menuconfig 生成的变量和规则
-include $(NEMU_HOME)/include/config/auto.conf
-include $(NEMU_HOME)/include/config/auto.conf.cmd

# 移除字符串中的双引号
remove_quote = $(patsubst "%",%,$(1))

# 从 menuconfig 中提取变量
GUEST_ISA ?= $(call remove_quote,$(CONFIG_ISA))
ENGINE ?= $(call remove_quote,$(CONFIG_ENGINE))
NAME    = $(GUEST_ISA)-nemu-$(ENGINE)

# 包含所有 filelist.mk 文件来合并文件列表
FILELIST_MK = $(shell find -L ./src -name "filelist.mk")
include $(FILELIST_MK)

# 过滤掉黑名单中的目录和文件，得到最终的源文件列表
DIRS-BLACKLIST-y += $(DIRS-BLACKLIST)
SRCS-BLACKLIST-y += $(SRCS-BLACKLIST) $(shell find -L $(DIRS-BLACKLIST-y) -name "*.c")
SRCS-y += $(shell find -L $(DIRS-y) -name "*.c")
SRCS = $(filter-out $(SRCS-BLACKLIST-y),$(SRCS-y))

# 从 menuconfig 中提取编译器及其选项
ifneq ($(CONFIG_CC),)
CC = $(call remove_quote,$(CONFIG_CC))
endif
CFLAGS_BUILD += $(call remove_quote,$(CONFIG_CC_OPT))
CFLAGS_BUILD += $(if $(CONFIG_CC_LTO),-flto,)
CFLAGS_BUILD += $(if $(CONFIG_CC_DEBUG),-Og -ggdb3,)
CFLAGS_BUILD += $(if $(CONFIG_CC_ASAN),-fsanitize=address,)
CFLAGS_TRACE += -DITRACE_COND=$(if $(CONFIG_ITRACE_COND),$(call remove_quote,$(CONFIG_ITRACE_COND)),true)
CFLAGS  += $(CFLAGS_BUILD) $(CFLAGS_TRACE) -D__GUEST_ISA__=$(GUEST_ISA)
LDFLAGS += $(CFLAGS_BUILD)

# 包含 menuconfig 的构建规则
include $(NEMU_HOME)/scripts/config.mk

# 根据不同的目标包含相应的规则
ifdef CONFIG_TARGET_AM
include $(AM_HOME)/Makefile
LINKAGE += $(ARCHIVES)
else
# 包含构建 NEMU 的规则
include $(NEMU_HOME)/scripts/native.mk
endif

# 工作流程描述：
# 1. 首先通过 `ifeq` 语句检查环境变量 `NEMU_HOME` 是否指向了一个有效的 NEMU 仓库，如果没有找到相应的主文件，则抛出错误信息。
# 2. 包含自动生成的配置文件 `auto.conf` 和 `auto.conf.cmd`，这些文件通常是由 `menuconfig` 生成的，用于存储配置信息。
# 3. 使用 `remove_quote` 函数移除字符串中的双引号，以便处理 `menuconfig` 中的变量。
# 4. 提取 `GUEST_ISA` 和 `ENGINE` 等变量，并构造目标名称 `NAME`，以便后续使用。
# 5. 使用 `find` 命令查找并包含所有的 `filelist.mk` 文件，合并所有源文件列表。
# 6. 过滤掉黑名单中的目录和文件，最终得到需要编译的源文件列表 `SRCS`。
# 7. 从 `menuconfig` 中提取编译器和编译选项，包含优化选项、调试选项和地址消毒器等选项，构建最终的编译参数 `CFLAGS` 和链接参数 `LDFLAGS`。
# 8. 包含配置脚本 `config.mk`，用于进一步的配置处理。
# 9. 根据目标系统选择相应的构建规则：
#    - 如果配置了 `CONFIG_TARGET_AM`，则包含 `AM_HOME` 下的 `Makefile`，并增加链接目标。
#    - 否则，包含 `NEMU_HOME` 下的 `native.mk`，用于构建 NEMU 的规则。
