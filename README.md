# OS   
## Description  
Based on ARMv8, mainly referring to xv6 and Linux kernel     
### Source directory structure      
.      
│   
├── Documentation   
│   └── translations   
│       └── arm64    
│           ├── amu.rst   
│           ├── booting.txt   
│           ├── hugetlbpage.rst   
│           ├── index.rst   
│           ├── legacy_instructions.txt   
│           ├── memory.txt   
│           ├── silicon-errata.txt   
│           └── tagged-pointers.txt   
│   
├── kernel   
│   ├── arch   
│   │   └── aarch64   
│   │       ├── board   
│   │       │   └── raspi3   
│   │       │       ├── gpio.h   
│   │       │       ├── kpg.c   
│   │       │       ├── mbox.h   
│   │       │       ├── peripherals_base.h   
│   │       │       ├── uart.h   
│   │       │       ├── mbox.c   
│   │       │       ├── uart.c   
│   │       │       ├── memlayout.h   
│   │       │       ├── page.c   
│   │       │       ├── local_peripherals.h   
│   │       │       ├── irq.h   
│   │       │       └── irq.c   
│   │       ├── include   
│   │       │   ├── exception.h   
│   │       │   ├── trapframe.h   
│   │       │   └── context.h   
│   │       ├── sysreg.h   
│   │       ├── bitops.h   
│   │       ├── timer.h   
│   │       ├── timer.c   
│   │       ├── entry.S   
│   │       ├── trap_asm.S   
│   │       ├── arm.h   
│   │       ├── mmu.h   
│   │       ├── switch.S   
│   │       └── trap.c   
│   ├── buffer   
│   │   ├── buf.c   
│   │   └── buf.h   
│   ├── drivers   
│   │   └── mmc   
│   │       ├── sd.h   
│   │       └── bcm2835_sd.c   
│   ├── file  
│   │   ├── fcntl.h  
│   │   ├── file.c  
│   │   └── file.h  
│   ├── interrupt  
│   │   ├── interrupt.c  
│   │   └── interrupt.h  
│   ├── lib  
│   │   ├── string.c  
│   │   └── string.h   
│   ├── memory  
│   │   ├── vm.h  
│   │   ├── memory.h  
│   │   ├── internal.h  
│   │   ├── kalloc.h  
│   │   ├── kalloc.c  
│   │   └── vm.c  
│   ├── pipe  
│   │   ├── pipe.h  
│   │   └── pipe.c  
│   ├── proc  
│   │   ├── elf.h  
│   │   ├── proc.h  
│   │   ├── proc.c  
│   │   └── exec.c  
│   ├── sync  
│   │   ├── spinlock.c  
│   │   ├── sleeplock.c  
│   │   ├── sleeplock.h   
│   │   └── spinlock.h  
│   ├── syscall   
│   │   ├── sysproc.h   
│   │   ├── syscall.h  
│   │   ├── syscall.c  
│   │   ├── arg.h  
│   │   ├── sysproc.c  
│   │   └── sysfile.c  
│   ├── console.h  
│   ├── printf.h  
│   ├── printf.c   
│   ├── printk.h  
│   ├── console.c  
│   ├── fs  
│   │   ├── log.h  
│   │   ├── log.c  
│   │   ├── fs.h  
│   │   └── fs.c  
│   ├── include   
│   │   ├── build_bug.h   
│   │   ├── linkage.h   
│   │   ├── stdint.h   
│   │   ├── compiler_types.h   
│   │   ├── compiler_attributes.h   
│   │   ├── list.h   
│   │   ├── poison.h   
│   │   ├── kernel.h   
│   │   ├── util.h   
│   │   ├── types.h    
│   │   ├── stat.h  
│   │   ├── param.h  
│   │   └── dev_t.h  
│   └── main.c     
│   
├── user   
│   ├── src  
│   │   ├── cat  
│   │   │   └── cat.c  
│   │   ├── echo  
│   │   │   └── echo.c  
│   │   ├── hello  
│   │   │   └── hello.c  
│   │   ├── ls  
│   │   │   └── ls.c  
│   │   ├── mkdir  
│   │   │   └── mkdir.c  
│   │   ├── sh  
│   │   │   └── sh.c  
│   │   ├── stressfs   
│   │   │   └── stressfs.c  
│   │   ├── init  
│   │   │   └── init.c  
│   │   ├── forktest  
│   │   │   └── forktest.c  
│   │   └── lib  
│   │       ├── umalloc.c  
│   │       ├── ulib.c  
│   │       ├── printf.c  
│   │       └── usyscall.S  
│   ├── initcode.S  
│   ├── include    
│   │   └── user.h    
│   └── Makefile   
│    
├── tool  
│   ├── boot  
│   │   ├── config.txt  
│   │   ├── fixup.dat  
│   │   ├── LICENCE.broadcom  
│   │   ├── start.elf  
│   │   └── bootcode.bin  
│   ├── mkfs  
│   │   └── mkfs.c  
│   └── bak   
│       └── Makefile.bak 
│     
├── linker.ld   
├── Makefile  
├── mksd.mk  
└── README.md  
***
## Startup log  
\********************************************************************************    
kernel booing...  
Buddy system info:  
Managed_pages: 255025  Available pages: 255025  bytes: 1044582400    
Each order:  
Order: 0	 free: 1	 pages:1  
Order: 1	 free: 0	 pages:0  
Order: 2	 free: 0	 pages:0  
Order: 3	 free: 0	 pages:0  
Order: 4	 free: 1	 pages:16  
Order: 5	 free: 1	 pages:32  
Order: 6	 free: 0	 pages:0  
Order: 7	 free: 0	 pages:0  
Order: 8	 free: 0	 pages:0  
Order: 9	 free: 0	 pages:0  
Order: 10	 free: 249	 pages:254976  
Total free pages: 255025  
Memory manage system initialized.  
[cpu 0] timer frequency: 62500000  
binit: success.    
\- mbox write: 0x7FD18     
\- mbox read: 0x7FD18    
\- clock rate: 50000000    
\- SD base clock rate from mailbox: 50000000  
\- Reset the card.  
\- Divisor selected = 104, shift count = 6  
\- EMMC: Set clock, status 0x1FF0000 CONTROL1: 0xE6807  
\- Send IX_GO_IDLE_STATE command.  
\- Send command response: 0   
\- EMMC: Sending ACMD41 SEND_OP_COND status 1FF0000  
\- Divisor selected = 2, shift count = 0  
\- EMMC: Set clock, status 0x1FF0000 CONTROL1: 0xE0207  
\- EMMC: SD Card Type 2 SC 128Mb UHS-I 0 mfr 170 'XY:QEMU!' r0.1 2/2006, #FFFFFFFFDEADBEEF RCA 4567  
sd_init: Partition 1: 00 20 21 00 0C 49 01 08 00 08 00 00 00 00 02 00   
\- Status: 0  
\- CHS address of first absolute sector: head=32, sector=33, cylinder=0  
\- Partition type: 12  
\- CHS address of last absolute sector: head=73, sector=1, cylinder=8  
\- LBA of first absolute sector: 0x800  
\- Number of sectors: 131072  
sd_init: Partition 2: 00 49 02 08 83 51 01 10 00 08 02 00 00 F8 01 00   
\- Status: 0  
\- CHS address of first absolute sector: head=73, sector=2, cylinder=8  
\- Partition type: 131  
\- CHS address of last absolute sector: head=81, sector=1, cylinder=16  
\- LBA of first absolute sector: 0x20800  
\- Number of sectors: 129024  
sd_init: Partition 3: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   
\- Status: 0  
\- CHS address of first absolute sector: head=0, sector=0, cylinder=0  
\- Partition type: 0  
\- CHS address of last absolute sector: head=0, sector=0, cylinder=0  
\- LBA of first absolute sector: 0x0  
\- Number of sectors: 0  
sd_init: Partition 4: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   
\- Status: 0  
\- CHS address of first absolute sector: head=0, sector=0, cylinder=0  
\- Partition type: 0  
\- CHS address of last absolute sector: head=0, sector=0, cylinder=0  
\- LBA of first absolute sector: 0x0  
\- Number of sectors: 0  
sd_init: Boot signature: 55 AA  
sd_init: success.  
main: [CPU 0] started.  
[cpu 2] timer frequency: 62500000  
[cpu 3] timer frequency: 62500000  
  
\------------------------
Kernel boot success.
\------------------------  
  
main: [CPU 3] started.  
main: [CPU 2] started.  
[cpu 1] timer frequency: 62500000  
main: [CPU 1] started.  
\$ hello  
                    
                                  
            _|_|      _|_|_|      
          _|    _|  _|            
          _|    _|    _|_|        
          _|    _|        _|      
            _|_|     _|_|_|       
                                  
                                  
            Based on xv6-riscv      
              Hello, the World      
\$ echo hello,OS  
hello,OS  
\$ ls  
.              1 1 512   
..             1 1 512  
init            2 2 15440  
sh             2 3 23856  
echo            2 4 15048  
forktest        2 5 15376  
hello          2 6 15232   
cat            2 7 15584  
ls             2 8 16752  
mkdir        2 9 15072  
stressfs       2 10 15456  
console       3 11 0  
\$   
***
## Update log 
* Jun 6, 2022  
添加启动成功的提示           
添加hello文件   
  
* Jun 1, 2022  
fix fork and add some test      
  
* May 29, 2022   
添加cat的实现       
修复open系统调用的一处错误     
添加 echo          
  
* May 27, 2022  
添加用户态的一些基础架构    

* May 25, 2022  
添加sleep系统调用     
添加updtime逻辑     
分离用户态与内核的定义依赖    
将file与pipe联系起来     
实现pipe读写逻辑    
添加 link 与 unlink    
添加sbrk的实现    

* May 4, 2022  
添加mkdir系统调用    
实现fstat系统调用    
添加kill系统调用     
添加chdir功能    
添加yield逻辑        

* May 3, 2022  
为内存分配系统添加锁
完善fork,wait逻辑              

* May 1, 2022  
实现fork    
实现getpid调用    
实现dup系统调用    
  
* Apr 24, 2022  
完成proc中p->cwd的有关逻辑;  
修复trap环境恢复的错误;     
修复了文件系统的若干问题 todo vm walk      

* Apr 23, 2022  
未完成关于initcode的实现    
实现了open close的系统调用 以及exec的雏形    
整合makefile    
  
* Apr 22, 2022   
开始添加用户程序     
串口终端中断回显实现    

* Mar 20, 2022  
重构console驱动以及printf实现 未完成      
fix mkfs include冲突;     
fix fs.h 缺少iunlock函数声明      
完善sleeplock的实现      
更新主函数初始化      
实现sleep wakeup kill    
完善file.c    
完善file.h    
实现文件系统日志    

* Mar 19, 2022  
实现文件系统inode层面的 没有验证实现    
开始实现文件系统    
实现系统调用     

* Mar 17, 2022  
实现内核态用户态之间的转换     
修改context的存储位置到proc而非栈上; 加入main函数中进程管理,用户程序的初始化    
add trapframe_dump  
add proc_dump  
添加swtch和调度 未验证  

* Mar 16, 2022  
部分init_user函数   
添加了trap返回的逻辑 消除了一些编译的warning  
增加uvmdealloc函数  

* Mar 15, 2022  
remove useless file       
基本完成vm     

* Feb 16, 2022  
fix copyout    
添加vm的实现 未完成    
添加proc,file结构体定义    
为buffer系统实现 brelease    
添加memcmp    

* Feb 11, 2022  
加入mkfs    
添加buffer与sd系统的初始化  
开始添加文件系统 未完成  
  
* Jan 19, 2022    
完善uart (但没有完全完成)    
完善自旋锁    
 
* Jan 17, 2022  
support core timer  

* Jan 14, 2022  
add interupt_handler   

* Jan 8, 2022    
给串口输出以及printf添加自旋锁    

* Jan 6, 2022   
finish vm core algorithm  
在vm.c中添加一些类型转换  
add vm (unfinished...)  

* Jan 4, 2022  
Add vm (unfinish...)  
  
* Jan 3, 2022  
完成内存管理系统    
  
* Dec 30, 2021  
完成kalloc kfree  
add alloc free prototype    
内存管理系统初始化可以正常运行    
未完成 内存分配还有bug...    
  
* Dec 29, 2021  
将pages数组改为指针 (因为它本身太大,不宜放在bss区占用镜像体积)  
内存管理初始化  
添加通过链表访问元素的方法    
完成链表操作部分    

* Dec 28, 2021  
Joining a linked list (unfinished...)    

* Dec 25, 2021  
enable mmu  
add .gdbinit  
add sysreg.h
  
* Dec 23, 2021  
mmu.h  

* Dec 20, 2021  
Start my OS  
***
## Reference    
***
1、https://github.com/mit-pdos/xv6-riscv   
2、https://developer.arm.com/documentation   
3、https://www.kernel.org/doc/html/latest/#   
4、http://www.wowotech.net/sort/armv8a_arch     
5、https://pdos.csail.mit.edu/6.828/2021/schedule.html   
6、https://github.com/huihongxiao/MIT6.S081     
7、https://blog.csdn.net/u013577996/article/details/108679997     
8、https://www.ibm.com/docs/en/xl-c-aix/13.1.2?topic=extension-atomic-lock-release-synchronize-functions    
