/*******************************************************************************************
文件:   ECanSelf.c
功能:	采用自检方式进行DSP281x eCAN 数据的回送和接收。		
		本测试程序不停地高速回送数据，对接收到的数据进行校验，有错即予标识。
		MBX0传送到MBX16, MBX1传送到MBX17 ....  本程序阐明自检方式的应用。
		另外增加了led发光二极管显示(DS20～DS25)的接口。以便反映eCAN通信的次数
*******************************************************************************************/

#include "DSP281x_Device.h"     // DSP281x 头文件包含文件
#include "DSP281x_Examples.h"   // DSP281x 示例包含文件

	// 本文件建立的函数原型声明
void mailbox_check(int32 T1, int32 T2, int32 T3);	
	// 检查邮箱接收的数据
void mailbox_read(int16 i); 						
	// 读出由参数MBXnbr指出的邮箱的内容。
void WriteXram(Uint16 *StartAddr, Uint16 *EndAddr, Uint16 number);
	// 从指定的外存地址StartAddr开始到EndAddr结束，顺序写入number数据。
void WriteMDLH(Uint16 *StartAddr, Uint16 *EndAddr);
	// 以跟踪计数器Trackcount为基数，从第一个发送邮箱开始以Trackcount加1递增数存入
	// 发送邮箱的各个单元(总共64个字单元)。
void CompMDLH(Uint16 *Comp1, Uint16 *Comp2);
	// 在eCAN回送的条件下，将前16个发送邮箱的数据与后16个接收邮箱的数据一一比较，
	// 若不等由错误计数器ErrorCount累计计数。
void delay(Uint16 dly);

	// 本例的全局变量
Uint32  ErrorCount=0;			// 错误计数器
Uint32  MessageReceivedCount;	// 报文接收计数器

Uint32  TestMbox1 = 0;
Uint32  TestMbox2 = 0;
Uint32  TestMbox3 = 0;

Uint32	Trackcount=0;			// 发送邮箱动态数据更新跟踪计数器。采用Trackcount加1
								// 递增形式，依次存入16个发送邮箱共64个字。下一次发
								// 送邮箱数据更新，以前一个动态的Trackcount为基数。

void main(void)
{
		//	eCAN控制寄存器需用32位数据进行读写访问，因此在本例中构建了一组影子寄存器，
		//	这种影子寄存器采用32位数据访问方式而非16位方式。
	struct ECAN_REGS ECanaShadow; 	
					// 定义ECanaShadow为具有ECAN_REGS相同类型的结构体变量.

		//  系统控制初始化:
   	InitSysCtrl();	// 锁相环(PLL),看门狗(WatchDog)及外设时钟(PeripheralClocks)初始化。
					// InitSysCtrl()函数由DSP281x_SysCtrl.c文件建立。
  	EALLOW;			// 允许访问受保护的空间。

		//	本程序采用GPIO多路复用器配置CAN引脚
   	EALLOW;
   	GpioMuxRegs.GPFMUX.bit.CANTXA_GPIOF6 = 1;		
   					// 当CANTXA_GPIOF6=0，将GPIOF6配置成普通I/O 口
					// 当CANTXA_GPIOF6=1，将GPIOF6配置成CANTXA引脚
   	GpioMuxRegs.GPFMUX.bit.CANRXA_GPIOF7 = 1;
					// 将GPIOF7配置成CANRXA引脚
   	EDIS;
	//初始化GPIO:
	EALLOW;	// 允许访问受保护的寄存器
		//将GPIOF8~GPIOF13配置为一般I/O口，数字量输出发光二极管显示(DS20～DS25)
 	GpioMuxRegs.GPFMUX.all = 0xc0ff;
    GpioMuxRegs.GPFDIR.all = 0x3f00; 		 
	EDIS;								// 禁止访问受保护的寄存器
    GpioDataRegs.GPFDAT.all=0x5555;		// 间隔一个点亮LED发光二极管。
		
   	DINT;	//	关CPU中断 
 
	IER = 0x0000;
	IFR = 0x0000;	// 关CPU中断并清所有CPU中断标识符。 
	
    MessageReceivedCount = 0;
    ErrorCount = 0;

/*******************************************************************************************
	这里对eCAN邮箱RAM区域0x6100-0x61FF设置初值0，其作用为配合接收邮箱赋值试验。参见
下面接收邮箱赋值程序段说明。
	实参"(Uint16 *)0x6100" 为WriteXram函数形参"Uint16 *StartAddr" 确定起始地址指针
	实参"(Uint16 *)0x6200" 为WriteXram函数形参"Uint16 *EndAddr" 确定结束地址指针
*******************************************************************************************/
	WriteXram((Uint16 *)0x6100,(Uint16 *)0x6200,0x000000);

    
/*******************************************************************************************
32位访问方法介绍:

		eCAN控制和状态寄存器需用32位数据进行读写访问，如果需要写单独的一位或几位，可通
	过以下软件步骤完成:

		(1) 先建立一个具有32位ECAN_REGS结构体类型的影子寄存器变量ECanaShadow(见本文件
			的第一条指令 "struct ECAN_REGS ECanaShadow;" )。由于在DSP281x_ECan.h头文件
			中已经有指令 "extern volatile struct ECAN_REGS ECanaRegs;",故称之为影子寄
			存器。
			如果需要写单独的一位，例如对CAN发送IO控制寄存器 CANTIOC 的 TXFUNC 位置1，
			可通过以下3步完成。
		(2) 以32位形式将整个寄存器读入影子寄存器:
				ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all;
		(3) 改变影子寄存器中要操作的位:
    			ECanaShadow.CANTIOC.bit.TXFUNC = 1;
		(4) 将影子寄存器回拷到带32位写的eCAN寄存器中
			    ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all;

		下面的8条指令配置eCAN的RX及TX引脚为eCAN传送方式
*******************************************************************************************/ 
    EALLOW;
    ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all;	// CANTIOC: CAN发送IO控制寄存器
    ECanaShadow.CANTIOC.bit.TXFUNC = 1;					// CANTX引脚用于CAN发送操作	 (P590)
    ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all;

    ECanaShadow.CANRIOC.all = ECanaRegs.CANRIOC.all;	// CANRIOC: CAN接收IO控制寄存器
    ECanaShadow.CANRIOC.bit.RXFUNC = 1;					// CANRX引脚用于CAN接收操作
    ECanaRegs.CANRIOC.all = ECanaShadow.CANRIOC.all;
    EDIS;
     
    ECanaRegs.CANME.all = 0;							// CANME:邮箱使能寄存器。	 (P571)
    		// 关所有邮箱。由于是写入整个寄存器(而不是位域)，故不需要影子寄存器。

/*******************************************************************************************	
发送邮箱消息标识符寄存器(MSGID)赋值说明：

	(1) 本例通过后面的指令"ECanaShadow.CANMC.bit.SCB = 1;"设置为eCAN方式，使用32个邮箱。
		并通过指令"ECanaRegs.CANMD.all = 0xFFFF0000;"将前16个邮箱设置为输出(与低位对应)，
		后16个邮箱设置为输入(与高位对应)。
	(2) 一个有用的发送邮箱的标识符的赋值必须与某一个接收邮箱的标识符的赋值相等(匹配)，数
		据通信时，这对邮箱才能够握手联络。因此可把标识符视作通信联络码。
	(3) MSGID是一个32位的寄存器，从最高位MSGID.31开始，顺序3位分别是：标识符扩展位 IDE,
		接收屏蔽使能位 AME 及自动应答模式位AAM。MSGID.28-MSGID.0 为存放通信联络码的消息
		标识符位。
	(4) 标识符有标准和扩展两种模式，标准占用MSGID.28-18，扩展占用MSGID.28-0。当
		IDE=0时，选用标准模式。当IDE=1时，选用扩展模式。本例为扩展模式。
	(5) 接收屏蔽使能位 AME 只用于接收箱，在AME=1时与全局接收屏蔽寄存器(CANGAM)联合使用。
		CANGAM.28-0是对应于MSGID.28-0的全局接收屏蔽位，若某位置1，则对应屏蔽MSGID.28-0的
		某位。本例取AME=0，不采用接收屏蔽。
	(6) 自动应答模式位 AAM 仅仅对配置为发送操作的消息邮箱有效，对于接收邮箱，该位无效。
		当 AAM=1时，为发送自动应答模式。此时，若发送端收到一个匹配的远端请求，CAN模块通
		过发送该邮箱的内容来应答远端请求。当 AAM=0时，为正常发送模式。发送端邮箱不对远端
		请求进行应答。本例取AAM=0，设置为正常发送模式。
	(7) 发送邮箱占用内存区域:0x006100-0x00617F
*******************************************************************************************/
    ECanaMboxes.MBOX0.MSGID.all = 0x9555AAA0; 							//			(P595)
    ECanaMboxes.MBOX1.MSGID.all = 0x9555AAA1; 
    ECanaMboxes.MBOX2.MSGID.all = 0x9555AAA2; 
    ECanaMboxes.MBOX3.MSGID.all = 0x9555AAA3; 
    ECanaMboxes.MBOX4.MSGID.all = 0x9555AAA4; 
    ECanaMboxes.MBOX5.MSGID.all = 0x9555AAA5; 
    ECanaMboxes.MBOX6.MSGID.all = 0x9555AAA6; 
    ECanaMboxes.MBOX7.MSGID.all = 0x9555AAA7; 
    ECanaMboxes.MBOX8.MSGID.all = 0x9555AAA8; 
    ECanaMboxes.MBOX9.MSGID.all = 0x9555AAA9; 
    ECanaMboxes.MBOX10.MSGID.all = 0x9555AAAA; 
    ECanaMboxes.MBOX11.MSGID.all = 0x9555AAAB; 
    ECanaMboxes.MBOX12.MSGID.all = 0x9555AAAC; 
    ECanaMboxes.MBOX13.MSGID.all = 0x9555AAAD; 
    ECanaMboxes.MBOX14.MSGID.all = 0x9555AAAE; 
    ECanaMboxes.MBOX15.MSGID.all = 0x9555AAAF; 
    
/*******************************************************************************************
接收邮箱消息标识符寄存器(MSGID)赋值说明：

	(1) 为了便于数据观察,这里采用了接收邮箱消息与发送邮箱消息上下对应相等的做法。
	(2) 一个有效的接收邮箱消息标识符寄存器MSGID所存储的消息，必须与某一个发送邮箱消息
	    标识符寄存器的消息相等。这样，当发送邮箱的一帧数据(包括标识符寄存器MSGID所存储
		的消息)被发送之后，接收端将接收到的每一个发送邮箱的消息标识符数据与接收邮箱消
		息标识符数据进行比较，如果相等，则接收到的标识符、控制位及数据字节写入对应的接
		收邮箱,即某一RAM区域。如果不等，则这一帧数据丢弃，不予存储。
	(3) 若存在一个接收邮箱，其消息标识符的数据与各个发送邮箱消息标识符的数据没有一个相
		等，则这个接收邮箱将收不到任何数据。 
	(4) 接收邮箱消息标识符寄存器MSGID的初始化没有次序上的要求。
	(5) 接收邮箱占用内存区域:0x006180-0x0061FF

		下面的16条指令对16个接收邮箱（MBOX16 - 31）的消息标识符寄存器MSGID赋值。为了对上
	面的概念有更直观的理解，可做下面2个试验：

	(1) 接收邮箱与发送邮箱的匹配试验：
		将MBOX16的消息改为0x9555AAAF, MBOX31的消息改为0x9555AAA0.编译运行程序之后暂停，观
		察邮箱RAM区域(View->Memory->0x00006100)，可以发现原来存入第17号邮箱MBOX16及第32号
		邮箱的数据作了调换。
	(2) 接收邮箱与发送邮箱的不匹配试验：
		将若干接收邮箱的消息设置成与发送邮箱的消息不匹配，编译运行程序之后暂停。可以发现这
		些邮箱保留前面的初始化数据。
*******************************************************************************************/
    ECanaMboxes.MBOX16.MSGID.all = 0x9555AAA0; 
    ECanaMboxes.MBOX17.MSGID.all = 0x9555AAA1; 
    ECanaMboxes.MBOX18.MSGID.all = 0x9555AAA2; 
    ECanaMboxes.MBOX19.MSGID.all = 0x9555AAA3; 
    ECanaMboxes.MBOX20.MSGID.all = 0x9555AAA4; 
    ECanaMboxes.MBOX21.MSGID.all = 0x9555AAA5; 
    ECanaMboxes.MBOX22.MSGID.all = 0x9555AAA6; 
    ECanaMboxes.MBOX23.MSGID.all = 0x9555AAA7; 
    ECanaMboxes.MBOX24.MSGID.all = 0x9555AAA8; 
    ECanaMboxes.MBOX25.MSGID.all = 0x9555AAA9; 
    ECanaMboxes.MBOX26.MSGID.all = 0x9555AAAA; 
    ECanaMboxes.MBOX27.MSGID.all = 0x9555AAAB; 
    ECanaMboxes.MBOX28.MSGID.all = 0x9555AAAC; 
    ECanaMboxes.MBOX29.MSGID.all = 0x9555AAAD; 
    ECanaMboxes.MBOX30.MSGID.all = 0x9555AAAE; 
    ECanaMboxes.MBOX31.MSGID.all = 0x9555AAAF; 

	//ECanaRegs.CANMD.all = 0x00000000;				// 将32个邮箱设置成发送邮箱
    ECanaRegs.CANMD.all = 0xFFFF0000; 			// CANMD: 邮箱指向寄存器。			(P571)		
			// 当CANMD.x=1，对应邮箱定义为接收邮箱，当CANMD.x=0，对应邮箱定义为发送邮箱。
			// 配置0-15邮箱为发送，16-31邮箱为接收。
    
    ECanaRegs.CANME.all = 0xFFFFFFFF;	// CANME: 邮箱使能寄存器					(P571)	
    		// 对邮箱使能后，相应邮箱的标识符写入操作被禁止。因此标识符写入必须
    		// 在邮箱使能之前	

/*******************************************************************************************
消息控制寄存器(MSGCTRL)使用说明：

	(1) 邮箱中32位消息控制寄存器(MSGCTRL)有3种用途：定义字节数，发送优先级及远程帧管理。
	(2) MSGCTRL.12-MSGCTRL.8为发送优先级位TPL4:0。
		这4位的位值，从大到小决定了该邮箱发送的优先级。当优先级相同时，具有较大序号的邮箱
		先进行发送操作。TPL只用于发送邮箱，而且不在SCC(16邮箱)模式中使用。本例没有对这4位
		进行设置，采用随机配置，不影响16邮箱消息的发送。
	(3) MSGCTRL.3-MSGCTRL.0为数据长度代码管理位DLC3:0。
		它决定进行发送或接收的数据字节数，最大只能设置8个字节。本例设置8个字节。
	(4) MSGCTRL.4为远端发送请求位 RTR。
		当RTR=1时，对于接收邮箱，如果TRS标志被置位，则会发送一个远程帧并且用同一个邮箱接收
		相应的数据帧。一旦远程帧被发送出去，邮箱的TRS位就回被CAN模块清0。对于发送邮箱，如果
		TRS标志被置位，则会发送一个远程帧，但是会用另一个邮箱接收相应的数据帧。由于系统复位
		时，RTR 为一随机状态，而本例没有远程帧请求，故设置RTR=0。
*******************************************************************************************/
    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;				// 设置数据长度为8个字节，
    ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;				// 最高只能为8个字节。
    ECanaMboxes.MBOX2.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX3.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX4.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX5.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX6.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX7.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX8.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX9.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX10.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX11.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX12.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX13.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX14.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX15.MSGCTRL.bit.DLC = 8;
    
		
    ECanaMboxes.MBOX0.MSGCTRL.bit.RTR = 0;  			// 设置无远程帧请求。			   
    ECanaMboxes.MBOX1.MSGCTRL.bit.RTR = 0;  
    ECanaMboxes.MBOX2.MSGCTRL.bit.RTR = 0;  
    ECanaMboxes.MBOX3.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX4.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX5.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX6.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX7.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX8.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX9.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX10.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX11.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX12.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX13.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX14.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX15.MSGCTRL.bit.RTR = 0;
    
/*	用WriteMDLH()动态数据代替
    // Write to the mailbox RAM field of MBOX0 - 15
    ECanaMboxes.MBOX0.MDL.all = 0x9555AAA0;
    ECanaMboxes.MBOX0.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX1.MDL.all = 0x9555AAA1;
    ECanaMboxes.MBOX1.MDH.all = 0x89ABCDEF;
   
    ECanaMboxes.MBOX2.MDL.all = 0x9555AAA2;
    ECanaMboxes.MBOX2.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX3.MDL.all = 0x9555AAA3;
    ECanaMboxes.MBOX3.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX4.MDL.all = 0x9555AAA4;
    ECanaMboxes.MBOX4.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX5.MDL.all = 0x9555AAA5;
    ECanaMboxes.MBOX5.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX6.MDL.all = 0x9555AAA6;
    ECanaMboxes.MBOX6.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX7.MDL.all = 0x9555AAA7;
    ECanaMboxes.MBOX7.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX8.MDL.all = 0x9555AAA8;
    ECanaMboxes.MBOX8.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX9.MDL.all = 0x9555AAA9;
    ECanaMboxes.MBOX9.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX10.MDL.all = 0x9555AAAA;
    ECanaMboxes.MBOX10.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX11.MDL.all = 0x9555AAAB;
    ECanaMboxes.MBOX11.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX12.MDL.all = 0x9555AAAC;
    ECanaMboxes.MBOX12.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX13.MDL.all = 0x9555AAAD;
    ECanaMboxes.MBOX13.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX14.MDL.all = 0x9555AAAE;
    ECanaMboxes.MBOX14.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX15.MDL.all = 0x9555AAAF;
    ECanaMboxes.MBOX15.MDH.all = 0x89ABCDEF;
*/

    EALLOW;
    ECanaRegs.CANMIM.all = 0xFFFFFFFF;		// CANMIM：邮箱中断屏蔽寄存器。			(P589)			
			// 邮箱中断使能。
			// 由于是写入整个寄存器(而不是位域)，故不需要影子寄存器。							  

 			// 要求允许改变配置寄存器:
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;   // CANMC,主控制寄存器			(P578)
    ECanaShadow.CANMC.bit.CCR = 1;            
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
    EDIS;
    
			// 通过等候CCE被置位，等待CPU准予更改配置寄存器的值
    do 
    {
      ECanaShadow.CANES.all = ECanaRegs.CANES.all;	// CANES：错误和状态寄存器		(P582)
    } while(ECanaShadow.CANES.bit.CCE != 1 );  
			// 当CPU对配置寄存器进行了写操作时，CCE=1，否则CCE=0。上面3条指令的
			// 含义为：当CCE=1时，执行下面的指令,否则等待CPU更改配置寄存器的值。    
    	
    		/******* Configure the eCAN timing  配置eCAN时钟。********/
    EALLOW;
    ECanaShadow.CANBTC.all = ECanaRegs.CANBTC.all;	// CANBTC: 位定时配置寄存器		(P580)
    ECanaShadow.CANBTC.bit.BRPREG = 9;
			// 波特率预定标器（BRPREG）与 CAN 时钟频率有以下关系：
			// 		CAN clock = SYSCLKOUT/(BRPREG + 1)
			// CAN clock 的倒数为时间量（TQ）长度(周期)，其关系为：
			// 		TQ = （1/SYSCLKOUT）*BRP  
			// 其中 SYSCLKOUT 为 CPU 时钟，BRP=BRPREG+1
			// 当 SYSCLKOUT=150MHz, BRPREG=9时，CAN 时钟频率=150/10=15 MHz.
    ECanaShadow.CANBTC.bit.TSEG2REG = 5 ;
    ECanaShadow.CANBTC.bit.TSEG1REG = 7; 
			// CAN 波特率由下式确定：
			// 波特率（bit rate）= SYSCLKOUT/(BRP*(bit-time))
			// 其中 SYSCLKOUT 为 CPU 时钟，BRP=BRPREG+1，bit-time=(TSEG1REG+1)+(TSEG2REG+1)+1
			// 当 SYSCLKOUT=150MHz, BRPREG=9, TSEG1REG = 7, TSEG2REG = 5 时，
			// CAN 波特率=150/(10*15)=1 MHz.										(P601)
    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;

			// 上面	EALLOW 下的指令更改了位定时配置寄存器配置。下面3条指令向CPU发
			// 出变换配置请求。	
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.CCR = 0;            
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

    EDIS;

			// 下面3条指令为等待CPU对位定时配置寄存器（CANBTC）配置的更改。
    do 
    {
      ECanaShadow.CANES.all = ECanaRegs.CANES.all;	// CANES：错误和状态寄存器		(P582)
    } while(ECanaShadow.CANES.bit.CCE != 1 );  
			// 当CPU对配置寄存器进行了写操作时，CCE=1，否则CCE=0。上面3条指令的
			// 含义为: 当CCE=0时，等待CPU更改配置寄存器的值;
			//         当CCE=1时，执行下面的指令。   


			// 配置 eCAN 为自检模式，使能 eCAN 的增强功能。
    EALLOW;
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;			// CANMC 主控制寄存器
/****************************************************************************************
注意：	屏蔽 eCAN 自检模式之后，示波器可观察到输出波形，但是程序在下面
		while(ECanaRegs.CANTA.all != 0x0000FFFF ) {}  指令处踏步。不屏蔽这条指令则
		示波器观察不到输出波形。

		根据实际使用情况有以下判断:
	(1)	当STM=1，即配置 eCAN 为自检模式的条件下，eCAN屏蔽对外发送信号。此时，eCAN的发送
		和接收在系统内部完成。
	(2)	当STM=0，在关闭自检模式的条件下，eCAN对外发送信号，示波器可观察到数据输出波形。
		此时，内部禁止接收，CANTA功能失效。外部接收机的情况如何待实验之后再看。
****************************************************************************************/
  	ECanaShadow.CANMC.bit.STM = 1;    	// 配置 eCAN 为自检模式 
    ECanaShadow.CANMC.bit.SCB = 1;    	// eCAN 模式（允许访问32位邮箱）。

    ECanaRegs.CANMC.all= ECanaShadow.CANMC.all;
    EDIS;
    
			// 开始传送
//    while(1)
	for(;;) 	// 若用while(1)作无限循环，编译通过,但会出现编译警告。故改用for(;;)                               
    {  
		WriteMDLH((Uint16 *)0x6104,(Uint16 *)0x617F);	// 改变发送邮箱待发送的数据。
				// 每个邮箱由两部分组成:它们是位于邮箱前半部的4字8字节的邮箱标识符，
				// 及位于邮箱后半部的4字8字节的数据。这条指令仅改变每个邮箱数据区的
				// 数据。
		
       	ECanaRegs.CANTRS.all = 0x0000FFFF;  			// CANTRS：发送请求置位寄存器
				// 对所有发送邮箱设置TRS位,即置TRS[15:0]全为1。16个邮箱开始发送。	(P572)

     	while(ECanaRegs.CANTA.all != 0x0000FFFF ) {}  // Wait for all TAn bits to be set..
				// 如果 CANTA 的某位被置1，则对应邮箱的消息被成功发送。这两条指令的含义为
				// 等待TA[15:0]均变为1，即等待所有邮箱的信息发送出去。

       	ECanaRegs.CANTA.all = 0x0000FFFF;   
       			// 通过置位清TAn，准备判别下一次发送与否。

       	MessageReceivedCount++;

		CompMDLH((Uint16 *)0x6104,(Uint16 *)0x6184);
				// 在eCAN回送的条件下，将前16个发送邮箱的数据与后16个接收邮箱的数据一一比较，
		  		// 若不等由错误计数器ErrorCount累计计数。

		GpioDataRegs.GPFTOGGLE.all = 0xffff;	// F端口取反

		delay(6);								// 延时 ,此处可设探点

				// 用上面的"CompMDLH((Uint16 *)0x6104,(Uint16 *)0x6184);"
				// 指令检查整个接收数据的正确性。代替下面屏蔽的检查程序。
/*
       	for(j=0; j<16; j++)          						// 读取16个接收邮箱信息并检查数据
       	{
          	mailbox_read(j); 								// 读出由实参j指出的邮箱的内容。
          	mailbox_check(TestMbox1,TestMbox2,TestMbox3); 	// 检查邮箱接收的数据
       	}
*/
    }
}

/*******************************************************************************************
函数名称: WriteXram(Uint16 *StartAddr, Uint16 *EndAddr, Uint16 number)
函数功能: 从指定的外存地址StartAddr开始到EndAddr结束，顺序写入number数据。调用该函数
		  时，可在*StartAddr++ =number;指令前设置断点，打开对应的Memory窗口，按F10键，
		  可看出 number 数据(递增)写入对应单元。
输入参数: 第一形参StartAddr为一指针变量，要写入数据的首地址。
		  第二形参EndAddr为一指针变量，要写入数据的结束地址。
		  第三形参number为写入的数据。
*******************************************************************************************/
void WriteXram(Uint16 *StartAddr, Uint16 *EndAddr, Uint16 number)
{
	while(StartAddr < EndAddr)
    { 
       	*StartAddr++ =number;
       	//*StartAddr++ =number++;
    }
    return;
}

/*******************************************************************************************
函数名称: WriteMDLH(Uint16 *StartAddr, Uint16 *EndAddr)
函数功能: 以跟踪计数器Trackcount为基数，从第一个发送邮箱开始以Trackcount加1递增数存入
		  发送邮箱的各个单元(总共64个字单元)。
输入参数: 第一形参为一指针变量StartAddr，实参地址指针指向数据发送区第一个发送邮箱。
		  第二形参为一指针变量EndAddr，实参地址指针指向数据接发送区最后一个发送邮箱。
*******************************************************************************************/
void WriteMDLH(Uint16 *StartAddr, Uint16 *EndAddr)
{
	Uint16 i;
	while(StartAddr < EndAddr)
    { 
       	for(i=0;i<4;i++) {*StartAddr++ =Trackcount++;}		// 每组邮箱4个字。
		StartAddr=StartAddr+4;								// 指向下一组邮箱数据首地址。
    }
    return;
}

/*******************************************************************************************
函数名称: CompMDLH(Uint16 *Comp1, Uint16 *Comp2)
函数功能: 在eCAN回送的条件下，将前16个发送邮箱的数据与后16个接收邮箱的数据一一比较，
		  若不等由错误计数器ErrorCount累计计数。
输入参数: 第一形参为一指针变量Comp1，实参地址指针指向数据发送区。
		  第二形参为一指针变量Comp2，实参址指针指向数据接收区。
*******************************************************************************************/
void CompMDLH(Uint16 *Comp1, Uint16 *Comp2)
{
	Uint16 i,j;
	for(i=0;i<16;i++)		// 大循环，取16组邮箱
	{
		for(j=0;j<4;j++)	// 小循环，每组含4个字，8个字节。
		{	
			if(*Comp1++ != *Comp2++)  { ErrorCount++;}		
							// 有兴趣可测试一下相等，打开观察窗输入ErrorCount进行观察。
		}
		Comp1=Comp1+4;		// 确定下一组起始地址
		Comp2=Comp2+4;
	}
}

/****************************************************************************************
函数名称: 	mailbox_read(int16 MBXnbr)
函数功能: 	读出由参数MBXnbr指出的邮箱的内容。
输入形参: 	MBXnbr 表示选择的邮箱号。

注	  意:	下面两个函数为TI原配函数。由于现在的文件在发送邮箱的配值上采取比较贴合
			际的动态数据，另外采取了对全部通信数据进行检测的方法。因此，不用这两
			个函数。但是，mailbox_read函数的构思是值得学习的。它是一个关于指针变量
			的简单明晰的范例。			
****************************************************************************************/
void mailbox_read(int16 MBXnbr)
{
	volatile struct MBOX *Mailbox;	
   		// MBOX 是一个结构体类型(见DSP281x_Ecan.h 头文件)，它由表明邮箱4个32位寄存器
   		// 特性的4个联合体成员变量组成。这里定义Mailbox是一个具蠱BOX结构体类型的结
   		// 构体指针变量。
   	Mailbox = &ECanaMboxes.MBOX0 + MBXnbr;
		// 指针变量Mailbox 取第一个发送邮涞牡刂� ECanaMboxes.MBOX0 作为基址再加上
		// 偏移量MBXnbr。
   	TestMbox1 = Mailbox->MDL.all; 	// = 0x9555AAAn (n is the MBX number)
   	TestMbox2 = Mailbox->MDH.all; 	// = 0x89ABCDEF (a constant)
   	TestMbox3 = Mailbox->MSGID.all;	// = 0x9555AAAn (n is the MBX number)

} // MSGID of a rcv MBX is transmitted as the MDL data.

/****************************************************************************************
函数称: 	mailbox_check(int32 T1, int32 T2, int32 T3)
函数功能: 	检查邮箱接收的数据
			如果 T1!=T3 或者 T2!=0x89ABCDEF，即发送数据与接收数据不等，执行循环体:
			错误计数器加1; 若都相等则跳过循环体。
输入形参: 	 
****************************************************************************************/
void mailbox_check(int32 T1, int32 T2, int32 T3)
{
	if((T1 != T3) || ( T2 != 0x89ABCDEF))	
    {
    	
       	ErrorCount++;
       	
    }
}

/*******************************************************************************************
函数名称: delay(Uint16 dly)
函数功能: 延时函数
输入参数: 形参dly，dly越大延时越久
输出参数: 无
*******************************************************************************************/
void delay(Uint16 dly)
{
	Uint32 i;
	for(;dly>0;dly--)
	{
		for(i=0;i<100000;i++);
	}
} 

//=========================================================================================
// No more.
//=========================================================================================
