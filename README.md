目录：

[TOC]

- 实验指导书：https://chyyuu.gitbooks.io/ucore_os_docs/content/
- 代码参考：https://github.com/chyyuu/os_kernel_lab/tree/master，请选择master分支
- 学堂在线：https://www.xuetangx.com/learn/THU08091000267/THU08091000267/4231154/video/6286999



## uCore OS实验指导书和源码网址 (2020)

- [ucore实验指导书](https://chyyuu.gitbooks.io/ucore_os_docs/content/)
- [ucore labs 1-8 源码和参考答案](https://github.com/chyyuu/ucore_lab)
- [os tutorial lab](https://github.com/chyyuu/os_tutorial_lab) 暂时不用管





## 实验总体流程

1. 在[学堂在线](https://www.xuetangx.com/courses/TsinghuaX/30240243X/2015_T1/about)查看OS相关原理和labX的视频；
2. 在[实验指导书 on gitbook](https://chyyuu.gitbooks.io/ucore_os_docs/content/)上阅读实验指导书，并参考其内容完成练习和实验报告；
3. 在实验环境中完成实验并提交实验到git server（清华学生需要在学校内部的git server上，其他同学可提交在其他git server上）；
4. 如实验中碰到问题，在[在线OS课程问题集](https://chyyuu.gitbooks.io/os_course_qa/content/)查找是否已经有解答；
5. 如没有解答，可在[piazza在线OS课程问答和交流区](https://piazza.com/tsinghua.edu.cn/spring2015/30240243x/home)提问,每天（一周七日）都有助教或老师在piazza在线答疑。（QQ群 181873534主要用于OS课程一般性交流）；
6. 可进一步在[学堂在线](https://www.xuetangx.com/courses/TsinghuaX/30240243X/2015_T1/about)或[在线的操作系统课程练习题](https://chyyuu.gitbooks.io/os_course_exercises/content/)完成实验相关的练习题；



## 四种学习目标和对应手段

1. 掌握OS基本概念：看在线课程，能理解OS原理与概念；看在线实验指导书并分析源码，能理解labcodes_answer的labs运行结果
2. 掌握OS设计实现：在1的基础上，能够通过编程完成labcodes的8个lab实验中的基本练习和实验报告
3. 掌握OS核心功能：在2的基础上，能够通过编程完成labcodes的8个lab实验中的challenge练习
4. 掌握OS科学研究：在3的基础上，能够通过阅读论文、设计、编程、实验评价等过程来完成课程设计（大实验）

【**注意**】

- **筑基内功**--请提前学习计算机原理、C语言、数据结构课程
- **工欲善其事，必先利其器**--请掌握七种武器 [实验常用工具列表](https://github.com/chyyuu/ucore_os_docs/blob/master/lab0/lab0_ref_ucore-tools.md)
- **学至于行之而止矣**--请在实验中体会操作系统的精髓
- **打通任督二脉**--lab1和lab2比较困难，有些同学由于畏难而止步与此，很可惜。通过lab1和lab2后，对计算机原理中的中断、段页表机制、特权级等的理解会更深入，等会有等同于打通了任督二脉，后面的实验将一片坦途。

> [实验指导书 on gitbook](https://chyyuu.gitbooks.io/ucore_os_docs/content/)中会存在一些bug，欢迎在在[piazza在线OS课程问答和交流区](https://piazza.com/tsinghua.edu.cn/spring2015/30240243x/home)提出问题或修改意见，会有赞和奖分！



[TOC]



## 实验进程

- 启动操作系统的bootloader：

  ```
  用于了解操作系统启动前的状态和要做的准备工作，了解运行操作系统的硬件支持，操作系统如何加载到内存中，理解两类中断--“外设中断”，“陷阱中断”等；
  ```

  

- 物理内存管理子系统：

  ```
  用于理解x86分段/分页模式，了解操作系统如何管理物理内存；
  ```

  

- 虚拟内存管理子系统：

  ```
  通过页表机制和换入换出（swap）机制，以及中断-“故障中断”、缺页故障处理等，实现基于页的内存替换算法；
  ```

  

- 内核线程子系统：

  ```
  用于了解如何创建相对与用户进程更加简单的内核态线程，如果对内核线程进行动态管理等；
  ```

  

- 用户进程管理子系统：

  ```
  用于了解用户态进程创建、执行、切换和结束的动态管理过程，了解在用户态通过系统调用得到内核态的内核服务的过程；
  ```

  

- 处理器调度子系统：

  ```
  用于理解操作系统的调度过程和调度算法；
  ```

  

- 同步互斥与进程间通信子系统：

  ```
  了解进程间如何进行信息交换和共享，并了解同步互斥的具体实现以及对系统性能的影响，研究死锁产生的原因，以及如何避免死锁；
  ```

  

- 文件系统：

  ```
  了解文件系统的具体实现，与进程管理等的关系，了解缓存对操作系统IO访问的性能改进，了解虚拟文件系统（VFS）、buffer cache和disk driver之间的关系。
  ```






## ucore系统结构图



> 其中每个开发步骤都是建立在上一个步骤之上的，就像搭积木，从一个一个小木块，最终搭出来一个小房子。在搭房子的过程中，完成从理解操作系统原理到实践操作系统设计与实现的探索过程。这个房子最终的建筑架构和建设进度如下图所示：


![ucore系统结构图](instruction.assets/image001.png)

图1 ucore系统结构图

如果完成上诉实验后还想做更大的挑战，那么可以参加ucore的研发项目，我们可以完成ucore的网络协议栈，增加图形系统，在ARM嵌入式系统上运行，支持虚拟化功能等。这些项目已经有同学参与，欢迎有兴趣的同学加入！





## 源代码下载

- `gitclone`

```bash
$ git clone https://github.com/chyyuu/os_kernel_lab.git
```

- `切换分支`

```
$ git branch -a										# 查看所有的分支
(头指针分离于 origin/rcore_tutorial_v3)
rcore_tutorial_v3
remotes/origin/HEAD -> origin/rcore_tutorial_v3
remotes/origin/gh-pages
remotes/origin/lab1_X
remotes/origin/lab2_X
remotes/origin/lab3_X
remotes/origin/lab4_X
remotes/origin/lab5_X
remotes/origin/master
remotes/origin/rcore_tutorial_v3
remotes/origin/ricv64-opensbi
remotes/origin/riscv32-priv-1.10
remotes/origin/riscv64-priv-1.10
remotes/origin/x86-32

$ git checkout remotes/origin/master				# 切换
之前的 HEAD 位置是 f2fa0b7 Update README.md
HEAD 目前位于 b12d4b9 Update README.md
```







