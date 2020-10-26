# 概述
- 可用于嵌入式程序动态加载执行elf(.so)文件

- 可用于Windows程序动态加载执行elf(.so)文件

  

# 目录说明

- `src` 源码

- `example` 示例

  - `blink_led_app` 动态库-实现LED闪烁功能

  - `runner` 程序-PC上加载运行动态库

  - `runner_stm32` 程序-stm32上加载运行动态库
  
  - `readelf`  程序-读取elf文件并打印
    
    

# 目前限制

- 不支持依赖

- 不支持重定向表

- 不支持非static全局变量

  

# 动态库编译说明

- 必须加上参数  `-fPIC`

- 嵌入式程序的须加上 `-Wl,-N` 





