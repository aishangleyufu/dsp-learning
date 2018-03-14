#include "DSP281x_Device.h"
#include "DSP281x_Examples.h"   // DSP281x Examples 头文件
#include "math.h"

// 由本文件建立的函数原型声明
void scia_echoback_init(void);				// SCIA回传初始化函数
void scia_fifo_init(void);					// SCIA 先进先出(fifo)初始化
void scia_xmit(int a);						// SCIA发送函数
void scia_msg(char *msg);					// 邮箱信息发送函数

	// 本例用到的全局计数器Uint16 LoopCount;
Uint16 LoopCount;
Uint16 ErrorCount;

void main(void)
{
    char *msg;						// 邮箱指针变量定义为字符型
// 系统控制初始化: 锁相环(PLL),看门狗(WatchDog)及外设时钟初始化	 
   InitSysCtrl();

// 初始化GPIO
   EALLOW;
   GpioMuxRegs.GPFMUX.all=0x0030;	// 选择GPIOs SCIA引脚
   GpioMuxRegs.GPGMUX.all=0x0030;	// 选择GPIOs SCIB引脚
   EDIS;

// 关所有中断:
   DINT;
// 禁止CPU级中断并清除所有CPU中断标志。
   IER = 0x0000;
   IFR = 0x0000;
	
// 用户特殊代码
    LoopCount = 0;
    ErrorCount = 0;
    scia_fifo_init();	   				// SCIA 先进先出(fifo)初始化
    scia_echoback_init();  				// SCIA回传初始化函数


	msg = "\r\n学号：3130102475\0";
	scia_msg(msg);

	msg = "\r\n日期：2015-01-08\0";
	scia_msg(msg);

	msg = "\r\n桌号：教2楼116室17实验台\0";
	scia_msg(msg);

	msg = "\r\n请输入一个字符，dsp将回传学号、日期和桌号 \0";	
    scia_msg(msg);

	for(;;)										// 无限循环。
    {
       		// 等待一个字符输入
       if(SciaRegs.SCIFFRX.bit.RXFIFST >= 1) 
       { 
			SciaRegs.SCIRXBUF.all=SciaRegs.SCIRXBUF.all;
	       	msg = "\r\n学号：3130102475\0";
			scia_msg(msg);

			msg = "\r\n日期：2015-01-08\0";
			scia_msg(msg);

			msg = "\r\n桌号：教2楼116室17实验台\0";
			scia_msg(msg);

			msg = "\r\n请输入一个字符，dsp将回传学号、日期和桌号 \0";
    		scia_msg(msg);
       } 	

    }
}
//*******************************************************************************************
// 函数：  scia_echoback_init()
// 功能： SCIA回传初始化
// 注意： SCIA外设时钟通过InitSysCtrl()函数开启。
// DSP281x_SysCtrl.c文件中的锁相环函数InitPll(0xA),则系统输出时钟：
//SYSCLKOUT=(OSCCLK*DIV)/2=(30*10)/2=150MHz。本程序LOSPCP[2:0]取复位默认值2，
//故低速外设时钟 LSPCLK=150/4=37.5 MHz。
//*******************************************************************************************
void scia_echoback_init()
{   
    SciaRegs.SCICCR.all =0x0007;            // 1位停止位，无奇偶校验，禁止回送，8位字符，
                                            // 异步模式，空闲-线协议。
    SciaRegs.SCICTL1.all =0x0003;           // 使能TX，RX，内部SCI时钟(SCICLK)，
                                            // 禁止接收错误中断，禁止睡眠，禁止唤醒。
    SciaRegs.SCICTL2.all =0x0003;           // 启动RXRDY/BRKDT中断，启动TXRDY中断。
    SciaRegs.SCICTL2.bit.TXINTENA =1;       // 这以下两条指令用位域的方法重复上面一条指令。
    SciaRegs.SCICTL2.bit.RXBKINTENA =1;  
    SciaRegs.SCIHBAUD    =0x0001;  //波特率设置，当低速外设时钟LSPCLK = 37.5MHz时，波特率为9600 。
    SciaRegs.SCILBAUD    =0x00E7;       
    SciaRegs.SCICTL1.all =0x0023;           // 与上面一条SCICTL1设置指令相比
                                            // 增加了放弃SCI软件复位
}
//*******************************************************************************************
// 函数：  scia_xmit(int a)
// 功能： 从SCI传送一个字符。形式参数a为待发送的数据
//*******************************************************************************************
void scia_xmit(int a)
{
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
    SciaRegs.SCITXBUF=a;
}
//*******************************************************************************************
// 函数：  scia_msg(char * msg)
// 功能：  邮箱信息发送函数。形参为一指针变量。msg是信息邮箱的首地址，下面用到的msg[i]的含义
//          为：相对于首地址msg的偏移地址i中存储的数据。注意：msg+i是相对于msg的偏移量，可以
//          用来表示msg的偏移地址。要取出该地址中存储数据还可以用*(msg+i)。
// 注意：  用"//##..." 标注的指令与上面一段用"//$$..."标注的指令作用相同。该段指令不用变量
//          i跟踪邮箱数据，而是采用指针变量取增量的方法进行邮箱数据跟踪。当一个字符串结束时，
//          *msg 读出的是'\0'。因此可以用'\0'来检测字符串中的字符是否发送完毕。
//*******************************************************************************************
void scia_msg(char * msg)
{   
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
/*
    int i;
    i = 0;
    while(msg[i] != '\0')
    {   
        scib_xmit(msg[i]);
        i++;
    }
*/
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

//######################################################
/**/
    while(*msg != '\0')
    {
        scia_xmit(*msg++); 
    }
/**/
//######################################################
}

//*******************************************************************************************
// 函数：  scib_fifo_init()
// 功能：  SCIB FIFO先进先出(FIFO)初始化子程序
//*******************************************************************************************
void scia_fifo_init()
{
    SciaRegs.SCIFFTX.all=0xE040;
        // SCIFFTX[15]=SCIRST=1 ，       SCI FOFO重新开始发送和接收。
        // SCIFFTX[14]=SCIFFENA=1，      使能SCI FOFO增强型功能。
        // SCIFFTX[13]=TXFIFO Reset=1，  重新使能发送FIFO的操作。
        // SCIFFTX[12:8]=TXFFST，        只读位，为0时，发送FIFO空。
        // SCIFFTX[7]=TXFFINT Flag， 只读位，为0时，不产生TXFIFO中断。
        // SCIFFTX[6]= TXFFINT CLR=1，   清除第7位TXFFINT的标志。
        // SCIFFTX[5]=TXFFIENA=0，       禁止基于TXFFIL匹配的TXFIFO中断。
        // SCIFFTX[4:0]=TXFFIL=00000b，  TXFFIL4- TXFFIL0发送FIFO中断级位清0。
    SciaRegs.SCIFFRX.all=0x204f;
        // SciaRegs[15]=RXFFOVF，        只读位，为0时，接收FIFO无溢出。
        // SciaRegs[14]= RXFFOVF CLR=0，写0时，对RXFFOVF无影响；
        //                              写1时，清RXFFOVF标志位
        // SciaRegs[13]=RXFIFO Reset=1，重新使能接收FIFO的操作。
        // SciaRegs[12:8]=RXFIFST，      只读位，为0时，接收FIFO空。写无影响。
        // SciaRegs[7]=RXFFINT Flag，    只读位，为0时，没有产生RXFIFO中断；
        //                              为1时，产生RXFIFO中断。
        // SciaRegs[6]= RXFFINT CLR=1，  清第7位RXFFINT Flag标志位。
        // SciaRegs[5]=RXFFIENA=0，      禁止基于RXFFIL匹配的RXFIFO中断。
        // SciaRegs[4:0]=RXFFIL=01111b，由于已经禁止基于RXFFIL匹配的RXFIFO中断，所以原
        //                              程序对RXFFIL位的设置是无意义的。
    SciaRegs.SCIFFCT.all=0x0;
        // SCIFFCT[13]=CDC=0，           禁止自动检测校准。
        // SCIFFCT[7:0]=FFTXDLY=0，      从FIFO发送缓冲器到发送移位寄存器之间的延时为0。
}


//===========================================================================================
// No more.
//===========================================================================================