# 概述

该项目运行于linux开发，主要包括以下部分：

命令行维护工具：主要是通过命令行进行业务状态的监控，进行动态配置，比较方便，当然这需要开发对应的业务程序



转发框架，包括视频传输格式的转换，ts、ps、rfc3984；流转发，等等



其它扩展：板级驱动层的扩展，即硬件抽象层

# 目录介绍

include  必须的头文件

kern  内核态运行的媒体框架

usr  用户态的库和终端工具mt



# 编译

```shell
make 即可
```

# 运行

```shell
insmod va.ko 
./mt
```

![](res\mt.png)