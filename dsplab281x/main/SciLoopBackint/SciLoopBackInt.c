//******************************************************************************************
// 文件: Sci_LoopBack_Int.c
// 标题: 通过中断进行DSP281x器件SCI FIFO数字回送测试,将SCI通信次数以二进制形式进行显示
// 条件:不需要其它的硬件配置。//
//******************************************************************************************
#include "DSP281x_Device.h"     		// DSP281x 头文件包含文件。
#include "DSP281x_Examples.h"   		// DSP281x 示例包含文件。

	//本文件建立的函数原型声明。
interrupt void sciaTxFifoIsr(void);		// SCIA发送先进先出中断函数。
interrupt void sciaRxFifoIsr(void);		// SCIA接收先进先出中断函数。
interrupt void scibTxFifoIsr(void);		// SCIB发送先进先出中断函数。
interrupt void scibRxFifoIsr(void);		// SCIB接收先进先出中断函数。
void scia_fifo_init(void);				// SCIA先进先出初始化函数。
void scib_fifo_init(void);				// SCIB先进先出初始化函数。
void error(void);						// 通信出错处理函数。
void delay(Uint16 dly);
void LedOut(Uint16 led);	// 将SCI通信次数以二进制形式进行显示
void gpio_init();	       // 初始化GPIO

void WriteXram(Uint16 *StartAddr, Uint16 *EndAddr);

	// 全局变量
Uint16 count=0;							// 计数器，记录循环体循环的次数。

Uint16 sdataA[8];    					// SCI-A发送数据
Uint16 sdataB[8];    					// SCI-B发送数据
Uint16 rdataA[8];    					// SCI-A接收数据
Uint16 rdataB[8];    					// SCI-B接收数据
Uint16 rdata_pointA; 					// 用于检查接收的数据
Uint16 rdata_pointB;

Uint16 *Xaddr=(Uint16 *)0x13f000; 	// 指针变量定义。外存地址0x13f000为回送数据存入的首地址
Uint16 Xaddr_track=0;  				// 外存地址跟踪指针

void main(void)
{ 
   	Uint16 i;
	// 系统控制初始化: PLL, WatchDog, 使能外设时钟	
   	InitSysCtrl();					
	// 初始化GPIO,用于led显示
 	gpio_init();
	// 将GP I/O 设置成SCI-A 及 SCI-B 功能									
   	EALLOW;	  		// 允许访问受保护的寄存器
	GpioMuxRegs.GPFMUX.all = 0x0030;
	GpioMuxRegs.GPGMUX.all = 0x0030;			 
	EDIS;			// 禁止访问受保护的寄存器。
		//清除所有中断和初始化PIE向量标			
	DINT;			// 关CPU中断

   	InitPieCtrl();		// 将PIE控制寄存器初始化为其默认状态
   	IER = 0x0000;		// 关CPU中断。	
   	IFR = 0x0000;		// 清除CPU中断标志位。	
   	InitPieVectTable();	// 初始化PIE向量表  				

   	EALLOW;		// 允许写入受保护的寄存器
   	PieVectTable.RXAINT = &sciaRxFifoIsr;  //初始化外设中断扩展(PIE)向量表
   				// RXAINT位于第9组中的第1个，TXAINT位于第9组中的第2个，
   				// RXBINT位于第9组中的第3个，TXBINT位于第9组中的第4个。
   	PieVectTable.TXAINT = &sciaTxFifoIsr;
   	PieVectTable.RXBINT = &scibRxFifoIsr;
   	PieVectTable.TXBINT = &scibTxFifoIsr;
   	EDIS;   	// 禁止写入受保护的寄存器
		
   	scia_fifo_init();  			// Init SCI-A 初始化SCI-A
   	scib_fifo_init();  			// Init SCI-B 初始化SCI-B
	
		//  用户绝对代码，使能中断
		// 发送数据初始化。在每次传送数据之后，该数据将更新为下一个要传送的数据。
   	for(i = 0; i<8; i++)	{ sdataA[i] = i;	}  
   								// sdataA[0]=0，sdataA[1]=1，... sdataA[7]=7。
   	for(i = 0; i<8; i++)	{ sdataB[i] = 0xFF - i;	 }
								// sdataB[0]=0xff，sdataB[1]=0xfe，... sdataB[7]=0xf8。
   	rdata_pointA = sdataA[0];	// rdata_pointA是一个动态变量，指向发送数组A 
//  sdataA[]中的第一个元素。	
   	rdata_pointB = sdataB[0]; 	// rdata_pointB是一个动态变量，指向发送数组B 
   								// sdataB[]中的第一个元素。	这两个变量用于内部回送测试。

		/********* 使能本示例所需的中断 **********/
   	PieCtrlRegs.PIECRTL.bit.ENPIE = 1;  	// 使能PIE向量表   		
   	PieCtrlRegs.PIEIER9.bit.INTx1=1;    	// 使能PIE 第9组 INT1(RXAINT)中断 
   	PieCtrlRegs.PIEIER9.bit.INTx2=1;    	// 使能PIE 第9组 INT2(TXAINT)中断
   	PieCtrlRegs.PIEIER9.bit.INTx3=1;    	// 使能PIE 第9组 INT3(RXBINT)中断
   	PieCtrlRegs.PIEIER9.bit.INTx4=1;    	// 使能PIE 第9组 INT4(TXBINT)中断
   	IER |= M_INT9;	//IER = 0x100;			// 使能第9组中断
   	EINT; 									// 开放可屏蔽中断  
      
		//  等中断,无限循环	
	for(;;){;}	
} 	
//*******************************************************************************************
// 函数： sciaTxFifoIsr
// 功能： SCI_A 发送先进先出(FIFO)中断服务子程序(ISR)
//*******************************************************************************************
void error(void)
{
	asm("     ESTOP0"); 	// 测试出错，终止。 C语言中插入汇编指令的格式: asm("  ESTOP0");
							// 意为 仿真停止0(ESTOP0)。第一个双引号与指令之间要空格。
    for (;;);				// 无限空循环。
}

//******************************************************************************************
// 函数： sciaTxFifoIsr
// 功能： SCI_A 发送先进先出(FIFO)中断服务子程序(ISR)
//******************************************************************************************
interrupt void sciaTxFifoIsr(void)
{   
    Uint16 i;
    for(i=0; i< 8; i++)	{ SciaRegs.SCITXBUF=sdataA[i]; }
 	      // 将发送数组A第i个数据sdataA[i]放入发送缓冲器SCITXBUF，发送8个数据
    for(i=0; i< 8; i++)                 	// 下一个周期，发送数据取增量。
    {
 	   sdataA[i] = (sdataA[i]+1) & 0x00FF; 
 	   		// 这里有2个sdataA[i]，后一个sdataA[i]是从先皊dataA[]数组中取
 	   		// 第i个元素，在加1并屏蔽高8位后重新赋值给第i个元素。
 	   		// 假如先前一个数组的8个元素为:		2,3,4,5,6,7,8,9
 	   		// 则后一个数组的8个元素为:		3,4,5,6,7,8,9,10...
	}	
	SciaRegs.SCIFFTX.bit.TXINTCLR=1;		// SCIFFTX[6]为TXINTCLR位，该位置1
											// 清除SCI中断标志位TXFFIN
	PieCtrlRegs.PIEACK.all |= 0x100;    	// 发出PIE应答，以示响应INT9中断
}

//******************************************************************************************
// 函数： sciaRxFifoIsr()
// 功能： SCI_A 接收先进先出(FIFO)中断服务子程序(ISR)
//******************************************************************************************
interrupt void sciaRxFifoIsr(void)
{   
    Uint16 i;
	for(i=0;i<8;i++)	{ rdataA[i]=SciaRegs.SCIRXBUF.all; }
	   	 	// 读出接收缓冲器SCIRXBUF中的数据，放入接收数组第i位，读出8个数据
	for(i=0;i<8;i++)                     // 检查接收的数据
	{
	   if(rdataA[i] != ( (rdata_pointA+i) & 0x00FF) ) error();
// 检测rdataA[i]与rdata_pointA+i对应8个数据相等否，不等转出错处理出，终止仿真调试; 相等顺执。
	   		// 由于rdata_pointA 跟踪发送数组A sdataA[]中的第一个元素，
// 因此(rdata_pointA+i)即为发送数组的对应元素。
// 接收数组第i个元素rdataA[i]为发送数组A sdataA[]中的第i个元素，
	   		// rdataA[i]与(rdata_pointA+i)两者相等，传送无误，否则出错。
	   		// 这里用到DSP281x串行传送的内部回送功能。
	}
	rdata_pointA = (rdata_pointA+1) & 0x00FF;  
												// 指向下一次发送数组第一个元素
	if(count>0xFFFF) count=0;
		else
		count++;//记录串行通信的次数
	LedOut(count);//显示串行通信SCI-A的次数,可在此处设探针连接视窗进行观察
	SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;  		// 清除溢出标志位
	SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;  		// 清除中断标志位
	PieCtrlRegs.PIEACK.all|=0x100;      		// 发出PIE应答，以示响应INT9中断		
												// PIEACK与0x100按位或后再对PIEACK赋值
}

//******************************************************************************************
// 函数： scia_fifo_init():
// 功能： SCI_A 先进先出(FIFO)初始化子程序
// 注意:  DSP281x_SysCtrl.c文件中的锁相环函数InitPll(0x5)的实参原值为0xA,现改为0x5。
// 在外接晶振30MHz的条件下,当锁相环函数实参为0xA时，系统时钟  SYSCLKOUT=(OSCCLK*DIV)/2=(30*10)/2=150MHz
// (LOSPCP)寄存器的默认值(默认值为2)。故低速外设时钟 LSPCLK=150/4=37.5 MHz
//******************************************************************************************
void scia_fifo_init()										
{
   	SciaRegs.SCICCR.all =0x0007;   
		// 对SCI通信控制寄存器(SCICCR)进行配置。一个结束位，禁止回送，
		// 禁止极性功能，8位字符，异步方式，选择空闲线协议。
   	SciaRegs.SCICTL1.all =0x0003;  
         // 使能TX, RX, 内部SCICLK,禁止接收错误中断，禁止睡眠，禁止TXWAKE
    	SciaRegs.SCICTL2.bit.TXINTENA =1;
   		// SCITXBUF中断使能，表示SCITXBUF寄存器准备接收下一个字符。
   	SciaRegs.SCICTL2.bit.RXBKINTENA =1;
   		// 启动接收缓冲器/中断。该位对SCI接收状态寄存器(SCIRXST)的RXRDY和
   		// BRKDT标志(SCIRXST.6及.5位)位进行控制
	ScibRegs.SCIHBAUD   =0x0001; 
   		// SCIHBAUD: 波特率选择最高有效字节寄存器
    ScibRegs.SCILBAUD   =0x00E7; 
   		// SCILBAUD: 波特率选择最低有效字节寄存器
	
   	SciaRegs.SCICCR.bit.LOOPBKENA =1;		// 启动自测试模式,Tx引脚在内部连接到Rx引脚
   	SciaRegs.SCIFFTX.all=0xC028;			// SCI FIFO 发送寄存器(SCIFFTX)
   											// SCI FIFO重新开始和接收，使能
   											// SCI FIFO的增强型功能
   											// 使能匹配中断，FIFO级位为8		
   	SciaRegs.SCIFFRX.all=0x0028;			// 使能匹配中断，FIFO级位为8
//   SciaRegs.SCIFFCT.all=0x00;				// SCI FIFO 控制寄存器(SCIFFCT)
   	SciaRegs.SCICTL1.all =0x0023;     		// 重新使能SCI
   	SciaRegs.SCIFFTX.bit.TXFIFOXRESET=1;	//重新使能FIFO发送
   	SciaRegs.SCIFFRX.bit.RXFIFORESET=1;	 	//重新使能FIFO接收
}

//******************************************************************************************
// 函数： scibTxFifoIsr
// 功能： SCI_B 发送先进先出(FIFO)中断服务子程序(ISR)
//******************************************************************************************
interrupt void scibTxFifoIsr(void)
{       
    Uint16 i;
    for(i=0; i< 8; i++)		{ ScibRegs.SCITXBUF=sdataB[i]; }
 	   		// 将发送数组B第i个数据sdataB[i]放入发送缓冲器SCITXBUF，发送8个数据
    for(i=0; i< 8; i++)                 		// 下一个周期，发送数据取增量。
    {
 	   sdataB[i] = (sdataB[i]-1) & 0x00FF; 
 	   		// 这里有2个sdataB[i]，后一个sdataB[i]是从先前sdataB[]数组中取
 	   		// 第i个元素，在减1并屏蔽高8位后重新赋值给第i个元素。
 	   		// 假如先前一个数组的8个元素为:		DC,DD,DE,DF,E0,E1,E2,E3
 	   		// 则后一个数组的8个元素为:			DB,DC,DD,DE,DF,E0,E1,E2
	}	
	ScibRegs.SCIFFTX.bit.TXINTCLR=1;    		// SCIFFTX[6]为TXINTCLR位，该位置1
												// 清除SCI中断标志位TXFFIN
	PieCtrlRegs.PIEACK.all|=0x100;      		// 发出PIE应答，以示响应INT9中断
}

//*******************************************************************************************
// 函数： scibRxFifoIsr()
// 功能： SCI_B 接收先进先出(FIFO)中断服务子程序(ISR)
//*******************************************************************************************
interrupt void scibRxFifoIsr(void)
{
    Uint16 i;
	for(i=0;i<8;i++) {rdataB[i]=ScibRegs.SCIRXBUF.all; }
	   	 	// 读出接收缓冲器SCIRXBUF中的数据，放入接收数组第i位，读出8个数据
	for(i=0;i<8;i++)                     		// 检查接收的数据
	{
	   if(rdataB[i] != ( (rdata_pointB-i) & 0x00FF) ) error();
	   		// 检测rdataB[i]与(rdata_pointB-i)对应8个数据相等否，不等转出错处理，终止仿真调试;
			// 相等顺执。 由于rdata_pointB 跟踪发送数组B sdataB[]中的第一个元素，因此
			// (rdata_pointB-i) 即为发送数组的对应元素。接收数组第i个元素rdataB[i]为发送数组B 
			// sdataB[]中的第i个元素，rdataB[i]与(rdata_pointB-i)两者相等，传送无误，否则出错。
	   		// 这里用到DSP281x串行传送的内部回送功能。
	}

//******************************************************************************************
//  以下3行指令作用为:
//		(1) 将回送值按顺序存入外存地址0x13f000开始的对应的44个单元。 
//		(2) Xaddr_track为地址跟踪指针，当Xaddr_track同步递增到44时复位地址指针
//			及地址跟踪指针。
//******************************************************************************************
   	for(i=0; i<8; i++)
   	{
		if(Xaddr_track>43) {Xaddr=(Uint16 *)0x13f000;Xaddr_track=0;}
		*Xaddr++ = rdataB[i];		//可在此处设探针连接外存地址0x13f000视窗进行观察
		Xaddr_track++;
	}

	rdata_pointB = (rdata_pointB-1) & 0x00FF; 	// 指向下一次发送数组第一个元素  
	ScibRegs.SCIFFRX.bit.RXFFOVRCLR=1;  		// 清除溢出标志位
	ScibRegs.SCIFFRX.bit.RXFFINTCLR=1; 			// 清除中断标志位
	PieCtrlRegs.PIEACK.all|=0x100;  			// 发出PIE应答，以示响应INT9中断	
												// PIEACK与0x100按位或后再对PIEACK赋值
}

//******************************************************************************************
// 函数： scib_fifo_init()	
// 功能： SCI_B 先进先出(FIFO)初始化子程序
//******************************************************************************************
void scib_fifo_init()										
{
   ScibRegs.SCICCR.all =0x0007;
			// 对SCI通信控制寄存器(SCICCR)进行配置。一个结束位，禁止回送，
			// 禁止极性功能，8位字符，异步方式，选择空闲线协议。
   ScibRegs.SCICTL1.all =0x0003;   
         	// 使能TX, RX, 内部SCICLK,禁止接收错误中断，禁止睡眠，禁止TXWAKE
   ScibRegs.SCICTL2.bit.TXINTENA =1;
			// SCITXBUF中断使能，表示SCITXBUF寄存器准备接收下一个字符。
   ScibRegs.SCICTL2.bit.RXBKINTENA =1;
   			// 启动接收缓冲器/中断。该位对SCI接收状态寄存器(SCIRXST)的RXRDY和
   			// BRKDT标志(SCIRXST.6及.5位)位进行控制

   ScibRegs.SCIHBAUD    =0x0001;		// 波特率=9600，与串行调试助手9600 
   ScibRegs.SCILBAUD    =0xE7;			// 波特率匹配，通过
   ScibRegs.SCICCR.bit.LOOPBKENA =1; 	// 启动自测试模式,Tx引脚在内部连接到Rx引脚
   ScibRegs.SCIFFTX.all=0xC028;			// SCI FIFO 发送寄存器(SCIFFTX)	
   										// SCI FIFO重新开始和接收，使能
   										// SCI FIFO的增强型功能
   										// 使能匹配中断，FIFO级位为8		  
   ScibRegs.SCIFFRX.all=0x0028;			// 使能匹配中断，FIFO级位为8

   ScibRegs.SCICTL1.all =0x0023;     	// 重新使能SCI
   ScibRegs.SCIFFTX.bit.TXFIFOXRESET=1;	//重新使能FIFO发送
   ScibRegs.SCIFFRX.bit.RXFIFORESET=1;	 //重新使能FIFO接收
} 

 //IO初始化子程序
void gpio_init()
{ 
	EALLOW;
	GpioMuxRegs.GPAMUX.bit.TDIRA_GPIOA11=0; //GPIOA11设置为一般I/O口
	GpioMuxRegs.GPADIR.bit.GPIOA11=1;	//把GPIOA11设置为输出	
    //将GPIOE0~GPIOE2配置为一般I/O口输出,作138译码  
 	GpioMuxRegs.GPEMUX.all = GpioMuxRegs.GPEMUX.all&0xfff8; 
	GpioMuxRegs.GPEDIR.all = GpioMuxRegs.GPEDIR.all|0x0007;
    //将GPIOB8~GPIOB15配置为一般I/O口,D0~D7
	GpioMuxRegs.GPBMUX.all = GpioMuxRegs.GPBMUX.all&0x00ff; 					    
    EDIS;
}

//记录SCI-A通信次数，显示在16个LED上
void LedOut(Uint16 led)
{
 	Uint16 i;
 	EALLOW;  
    //将GPIOB8~GPIOB15配置为输出,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all|0xff00;
	EDIS;	
	GpioDataRegs.GPEDAT.all = 0xfffb;    //LEDB置0
	GpioDataRegs.GPBDAT.all = ~led;
	for (i=0; i<100; i++){}              //延时
    GpioDataRegs.GPEDAT.all = 0xffff;    //锁存高8位	
	GpioDataRegs.GPEDAT.all = 0xfffa;    //LEDA置0
	GpioDataRegs.GPBDAT.all = ~(led<<8);
	for (i=0; i<100; i++){}
    GpioDataRegs.GPEDAT.all = 0xffff;    //锁存低8位			
}
//==========================================================================================
// No more.
//==========================================================================================

