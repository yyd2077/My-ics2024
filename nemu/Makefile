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

# 检查 NEMU_HOME 环境变量是否有效
ifeq ($(wildcard $(NEMU_HOME)/src/nemu-main.c),)
  $(error NEMU_HOME=$(NEMU_HOME) is not a NEMU repo)
endif
# 如果没有找到 NEMU_HOME 目录下的 src/nemu-main.c 文件，则输出错误信息

# 包含 menuconfig 自动生成的变量和规则文件
-include $(NEMU_HOME)/include/config/auto.conf
-include $(NEMU_HOME)/include/config/auto.conf.cmd
# 通过 "-include" 包含配置文件，这样如果文件不存在不会报错

# 定义函数 remove_quote 用于去除字符串中的引号
remove_quote = $(patsubst "%",%,$(1))

# 从 menuconfig 中提取变量
GUEST_ISA ?= $(call remove_quote,$(CONFIG_ISA))
ENGINE ?= $(call remove_quote,$(CONFIG_ENGINE))
NAME    = $(GUEST_ISA)-nemu-$(ENGINE)
# 使用 remove_quote 去掉配置变量中的引号并生成变量，GUEST_ISA 和 ENGINE 表示目标 ISA 和引擎类型，NAME 用于命名

# 包含所有 filelist.mk 文件以合并文件列表
FILELIST_MK = $(shell find -L ./src -name "filelist.mk")
include $(FILELIST_MK)
# 使用 find 命令查找 src 目录下所有名为 "filelist.mk" 的文件并包含它们

# 过滤掉目录和黑名单中的文件，得到最终的源文件集
DIRS-BLACKLIST-y += $(DIRS-BLACKLIST)
SRCS-BLACKLIST-y += $(SRCS-BLACKLIST) $(shell find -L $(DIRS-BLACKLIST-y) -name "*.c")
SRCS-y += $(shell find -L $(DIRS-y) -name "*.c")
SRCS = $(filter-out $(SRCS-BLACKLIST-y),$(SRCS-y))
# 将黑名单中的目录和文件进行合并，查找需要排除的源文件，最后过滤掉这些文件以得到最终的源文件集

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
# 根据配置文件，设置编译器、编译选项和链接选项，包含代码优化选项、调试选项以及地址消毒等安全检查选项

# 包含 menuconfig 的规则文件
include $(NEMU_HOME)/scripts/config.mk
# 包含配置脚本，以便对编译过程进行进一步控制

# 根据配置决定是否包含目标 AM 的 Makefile
ifdef CONFIG_TARGET_AM
include $(AM_HOME)/Makefile
LINKAGE += $(ARCHIVES)
# 如果目标为 AM，则包含 AM_HOME 目录下的 Makefile，设置链接选项
else
# 包含用于编译 NEMU 的规则
include $(NEMU_HOME)/scripts/native.mk
# 如果不是目标 AM，则包含用于本地编译的规则文件
endif

# 工作流程描述：
# 1. 首先进行基本的检查，通过 "ifeq" 判断 $(NEMU_HOME)/src/nemu-main.c 文件是否存在，确保 NEMU_HOME 环境变量设置正确。
# 2. 然后使用 "-include" 包含由 menuconfig 自动生成的配置文件，这些文件中包含了项目的各种编译和链接配置选项。
# 3. 使用 "remove_quote" 函数去除 CONFIG_ISA 和 CONFIG_ENGINE 中的引号，并设置 NAME 变量，用于表示构建的目标名称。
# 4. 使用 "find" 查找 src 目录下的 filelist.mk 文件，并包含它们，合并不同目录中的源文件列表。
# 5. 过滤掉黑名单中的目录和源文件，以得到最终的源文件集 SRCS。
# 6. 根据 menuconfig 提供的配置，设置编译器和编译选项，包括调试、LTO 优化、ASAN 安全检查等。
# 7. 包含 config.mk 文件，以便执行进一步的配置规则。
# 8. 根据是否为 CONFIG_TARGET_AM 目标，决定包含不同的编译规则文件：如果是 AM 目标，包含 AM_HOME 下的 Makefile；否则包含 NEMU_HOME 下的 native.mk，用于构建 NEMU。
