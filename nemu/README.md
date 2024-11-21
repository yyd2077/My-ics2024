# NEMU

NEMU（南京大学模拟器）是一个简单但完整的全系统模拟器，设计用于教学目的。
目前，它支持 x86、mips32、riscv32 和 riscv64。
要构建运行在 NEMU 上的程序，请参考 [AM 项目](https://github.com/NJU-ProjectN/abstract-machine)。

NEMU 的主要特性包括：
* 一个小型监控器，带有简单的调试器
  * 单步执行
  * 寄存器/内存检查
  * 不支持符号的表达式求值
  * 观察点
  * 与参考设计（例如 QEMU）的差分测试
  * 快照
* 支持常用指令的 CPU 核心
  * x86
    * 不支持实模式
    * 不支持 x87 浮点指令
  * mips32
    * 不支持 CP1 浮点指令
  * riscv32
    * 仅支持 RV32IM
  * riscv64
    * 仅支持 RV64IM
* 内存
* 分页
  * TLB 是可选的（但对 mips32 是必要的）
  * 不支持保护功能
* 中断和异常
  * 不支持保护功能
* 5 种设备
  * 串口、计时器、键盘、VGA、音频
  * 大多数设备被简化且不可编程
* 2 种 I/O 类型
  * 端口映射 I/O 和内存映射 I/O





# NEMU

NEMU(NJU Emulator) is a simple but complete full-system emulator designed for teaching purpose.
Currently it supports x86, mips32, riscv32 and riscv64.
To build programs run above NEMU, refer to the [AM project](https://github.com/NJU-ProjectN/abstract-machine).

The main features of NEMU include
* a small monitor with a simple debugger
  * single step
  * register/memory examination
  * expression evaluation without the support of symbols
  * watch point
  * differential testing with reference design (e.g. QEMU)
  * snapshot
* CPU core with support of most common used instructions
  * x86
    * real mode is not supported
    * x87 floating point instructions are not supported
  * mips32
    * CP1 floating point instructions are not supported
  * riscv32
    * only RV32IM
  * riscv64
    * only RV64IM
* memory
* paging
  * TLB is optional (but necessary for mips32)
  * protection is not supported
* interrupt and exception
  * protection is not supported
* 5 devices
  * serial, timer, keyboard, VGA, audio
  * most of them are simplified and unprogrammable
* 2 types of I/O
  * port-mapped I/O and memory-mapped I/O
