// Dated 2010-01-08
// For 60 marks



/******************************************************************************************
**描述:PWM1-6控制逆变桥6个开关管，开关频率60KHz
***ADC中断由T2启动，采样频率10K(T2)
**中断T1PINT更新PWM1～6的占空比(60k)，用全比较器产生3对PWM波，GP定时器1作全比较单元时基
**中断T3PINT更新PWM7～12的占空比(60k)，用全比较器产生3对PWM波，GP定时器3作全比较单元时基
**中断T4PINT扫描按键(10.24 ms)，实现四路DA转换,按键的值(0-9)置SCI发送数据*
**PDPINTA中断故障保护输入
**SCI以中断方式接收,用上位机串口调试程序实现与位机通讯
**Ecan以中断方式实现双机通讯,功能B键用于启动发送,将相应的频率值发送对方
**可中断嵌套
**功能C键:用键盘设定deaout频率;功能D键:用ADCINB7通道设定deaout频率;
**功能键E/F,按E键输出频率debout由键盘0-D设定,按F键输出频率debout由ADCINB7设定;
// sin表长8192(8K)幅值0～1250, 频率调节4Hz～2KHz
******************************************************************************************/

#include"DSP281x_Device.h"
#include   "math.h"           //用到正弦值的计算

static  long  x1=0;
static  long  x2=0;
static  long  x3=0;	
static  long  x4=0;
static  long  x5=0;
static  long  x6=0;	
unsigned int LEDReg;
unsigned int LED4=0;    //要显示的四位频率值
unsigned int LED3=0;
unsigned int LED2=0;
unsigned int LED1=0;
unsigned int KeyReg1;
unsigned int KeyReg2;
unsigned int flag=0;
unsigned long int i = 0;
unsigned long int q = 0;
void  IOinit(void);
void  LedOut(Uint16 led);
int   KeyIn1(void);
void  KeyFunction1(unsigned int KeyReg1);
void  KeyFunction2(unsigned int KeyReg2);
void   DAC(void);
void Write_LED (int LEDReg);//通过SPI向LED发送显示数据
Uint16 keyNum = 0x0000;  //按键次数
Uint16 RecieveChar; 
unsigned long int  AD1;  
unsigned long int AD2;  ////实际AD值*1000

Uint32  TestMbox1 = 0;
Uint32  TestMbox2 = 0;
Uint32  TestMbox3 = 0;

#define	  K1		0xfeff
#define	  K2		0xfdff
#define	  K3		0xfbff
#define	  K4		0xf7ff
#define	  K5		0xefff
#define	  K6		0xdfff
#define	  K7		0xbfff
#define	  K8		0x7fff
#define	  K9		0xfeff
#define	  K10		0xfdff
#define	  K11		0xfbff
#define	  K12		0xf7ff
#define	  K13		0xefff
#define	  K14		0xdfff
#define	  K15		0xbfff
#define	  K16		0x7fff

Uint16  sindata[8192];     //存正弦表
interrupt void  T1pint_isr(void);//中断子程序的声明
interrupt void  T3pint_isr (void);
interrupt void  T4pint_isr(void);
interrupt void  adc_isr(void);
interrupt void  Scirxinta_isr(void);
interrupt void  Ecan0inta_isr(void);
interrupt void  pdpaint_isr(void);

void  stop_pwm(void);       //停止PWM子程序
void  starlpwm(void);       //启动PWM子程序

#define   PI2   3.1416* 2 
							//2π
#define   FC    60000     	
							//保存载波频率60KHz(PWM1～6)
#define   FC2   10000     
							//A／D采样频率10KHz
#define   sinlen 8192     
							//8K的正弦表长度


long     Tlpr;            //保存Tl定时器的周期寄存器的值
long     T2pr;
long     T3pr;           //保存T2，T3定时器的周期寄存器的值

unsigned long int    deaout=0;   //EVA输出频率，在T4中断程序中由按键改变
unsigned long int    dea=0;   //EVA输出频率对应查表等分值
unsigned long int    dea1=0;   //对应查表等分值中间变量
unsigned long int    debout=0;   //EVB输出频率，在T4中断程序中由按键改变
unsigned long int    deb=0;   //EVB输出频率对应查表等分值
unsigned long int    deb1=0;   //对应查表等分值中间变量
int  j;

void EVA_PWM()
{
   	EvaRegs.EXTCONA.bit.INDCOE = 1;  //单独使能比较输出模式
    EvaRegs.ACTRA.all = 0x0aaa;     //空间矢量不动作
    EvaRegs.DBTCONA.all = 0x08ec;   //死区定时器启动
    EvaRegs.CMPR1 = 0x0006;
    EvaRegs.CMPR2 = 0x0005;
    EvaRegs.CMPR3 = 0x0004;
    EvaRegs.COMCONA.all = 0xa6e0;   //空间向量禁止，全比较使能，陷阱禁止
}

void EVA_Timer1()
{
   	EvaRegs.EXTCONA.bit.INDCOE = 1;  //单独使能比较输出模式
    EvaRegs.GPTCONA.all = 0x0000;   //GP定时器1比较输出低有效
  	Tlpr=75000000/FC;
	EvaRegs.T1PR=Tlpr;
    EvaRegs.T1CNT = 0x0000;     // 定时器初值
    EvaRegs.T1CON.all = 0x0846;//.连续增减计数,1分频,使能比较,内部时钟,打开定时器
}

void EVB_PWM()
{
   EvbRegs.EXTCONB.bit.INDCOE = 1;  //单独使能比较输出模式
    EvbRegs.ACTRB.all = 0x0aaa;     //空间矢量不动作
    EvbRegs.DBTCONB.all = 0x08ec;   //死区定时器启动
    EvbRegs.CMPR4 = 0x0010;
    EvbRegs.CMPR5 = 0x0005;
    EvbRegs.CMPR6 = 0x0004;
    EvbRegs.COMCONB.all = 0xa6e0;   //空间向量禁止，全比较使能，陷阱禁止
}

void EVB_Timer3()
{
   EvbRegs.EXTCONB.bit.INDCOE = 1;  //单独使能比较输出模式
    EvbRegs.GPTCONB.all = 0x0000;   //GP定时器1比较输出低有效
 	T3pr=75000000/FC;
	EvbRegs.T3PR=T3pr;   
    EvbRegs.T3CNT = 0x0000;     // 定时器初值
    EvbRegs.T3CON.all = 0x0846;////连续增减计数,1分频,使能比较,内部时钟,打开定时器
}

void IOinit()
{
 	EALLOW;  
 	//将GPIOA配置为外设口
 	GpioMuxRegs.GPAMUX.all = 0xffff;

    //将GPIOE0~GPIOE2配置为一般I/O口输出,作138译码  
 	GpioMuxRegs.GPEMUX.all = GpioMuxRegs.GPEMUX.all&0xfff8; 
	GpioMuxRegs.GPEDIR.all = GpioMuxRegs.GPEDIR.all|0x0007;
    //将GPIOB8~GPIOB15配置为一般I/O口,D0~D7
	GpioMuxRegs.GPBMUX.all = GpioMuxRegs.GPBMUX.all&0x00ff; 
	EDIS;				
}

//SPI初始化子程序
void spi_intial()
 {
 	SpiaRegs.SPICCR.all =0x0047;   // 使SPI处于复位方式, 下降沿, 八位数据  
	SpiaRegs.SPICTL.all =0x0006;   // 主控模式, 一般时钟模式,使能talk, 关闭SPI中断.
	SpiaRegs.SPIBRR =0x007F;       //配置波特率
	SpiaRegs.SPICCR.all =SpiaRegs.SPICCR.all|0x0080;  // 退出复位状态	 
	EALLOW;	
    GpioMuxRegs.GPFMUX.all=0x000F;	// 设置通用引脚为SPI引脚	 	
    EDIS;
  }

void gpio_init()
{ 
	EALLOW;
	GpioMuxRegs.GPAMUX.bit.TDIRA_GPIOA11=0; //GPIOA11设置为一般I/O口
	GpioMuxRegs.GPADIR.bit.GPIOA11=1;		//把GPIOA11设置为输出						    
   
	GpioMuxRegs.GPFMUX.bit.XF_GPIOF14 = 0;  //  GPIOF14设置为一般I/O端口
	GpioMuxRegs.GPFDIR.bit.GPIOF14 = 1;	   //  把GPIOF14设置为输出			
    EDIS;
    GpioDataRegs.GPADAT.bit.GPIOA11=0; 	//GPIOA11=0;该端口为74HC595锁存信号
    GpioDataRegs.GPFDAT.bit.GPIOF14 = 1; //  GPIOF14＝1;该端口为使能信号
}

void Scia_init()
{    
	EALLOW;
 	GpioMuxRegs.GPFMUX.bit.SCITXDA_GPIOF4 = 1;	 //  设置F4和F5为通信端口
 	GpioMuxRegs.GPFMUX.bit.SCIRXDA_GPIOF5 = 1;	 
    EDIS;
	SciaRegs.SCICTL2.all = 0x0002;  //  允许RX中断
 	SciaRegs.SCILBAUD = 0x00E7;     //  波特率＝9600
	SciaRegs.SCIHBAUD = 0x0001;  	     
  SciaRegs.SCICCR.all = 0x0007;  	//  1个停止位，禁止校验，8位字符 
                             		//  禁止自测试，异步空闲线协议
	SciaRegs.SCICTL1.all = 0x0023;  //  脱离复位状态，使能接收发送
}

void  CAN_INIT()
{
	struct ECAN_REGS ECanaShadow;  
	EALLOW;
  	GpioMuxRegs.GPFMUX.bit.CANTXA_GPIOF6 = 1; //  设置GPIOF6为CANTX 
  	GpioMuxRegs.GPFMUX.bit.CANRXA_GPIOF7 = 1; //  设置GPIOF7为CANRX    	
	EDIS;
/*eCAN 控制寄存器需要32位访问。如果想向一个单独位进行写操作，编译器可能会使其进入16位访问。这儿引用了一种解决方法，就是用影子寄存器迫使进行32位访问。 把整个寄存器读入一个影子寄存器。 这个访问将是32位的。用32位写操作改变需要改的位，然后把该值拷贝回eCAN寄存器*/
    EALLOW; 
   	ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all; //  把CANTIOC读入影子寄存器
   	ECanaShadow.CANTIOC.bit.TXFUNC = 1;              //  外部引脚I/O使能标志位。
//  TXFUNC＝1  CANTX引脚被用于CAN发送功能。
//  TXFUNC＝0  CANTX引脚被作为通用I/O引脚被使用 
	ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all; //  把配置好的寄存器值回写
    ECanaShadow.CANRIOC.all = ECanaRegs.CANRIOC.all;  //  把CANRIOC读影子寄存器
	ECanaShadow.CANRIOC.bit.RXFUNC = 1;              //  外部引脚I/O使能标志位。
//  RXFUNC＝1  CANRX引脚被用于CAN接收功能。
//  RXFUNC＝0  CANRX引脚被作为通用I/O引脚被使用。
    ECanaRegs.CANRIOC.all = ECanaShadow.CANRIOC.all;  //  把配置好的寄存器值回写
    EDIS;
//  在配置邮箱ID值之前，CANME对应的位必须复位，
//  如果CANME寄存器中对应的位被置位，则ID写入操作无效。
	ECanaRegs.CANME.all = 0;                 //  复位所有的邮箱
   	ECanaMboxes.MBOX0.MSGID.all = 0x15100000;  //  配置发送邮箱0的ID：扩展标识符29位  
    ECanaMboxes.MBOX16.MSGID.all = 0x15200000; //  确定接收邮箱16ID 
    //  把邮0～15 配置为发送邮箱 ， 把邮箱16～31 配为接收邮箱
    ECanaRegs.CANMD.all = 0xFFFF0000; 
    ECanaRegs.CANME.all = 0xFFFFFFFF;         //  CAN模块使能对应的邮箱，
        
    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;       //  把发送，接收数据的长度定义?位
    ECanaMboxes.MBOX0.MSGCTRL.bit.RTR = 0;       //  无远程帧请? 
//  因为RTR位在复位后状态不定，因此在程序进行初始化的时候必须对该位赋值。
    ECanaMboxes.MBOX1.MSGCTRL.bit.RTR = 0;
//  把待发送的数据写入发送邮箱 
 	ECanaMboxes.MBOX0.MDL.all = 0x55555555;
	ECanaMboxes.MBOX0.MDH.all = 0x55555501;	 
	EALLOW;
//  邮箱中断屏蔽寄存器。上电后所有的中断屏蔽位都清零且停止中断使能。
//  这些位允许独立屏蔽任何邮箱中断。
	ECanaRegs.CANGIM.all = 0x0001;     //中断0使能
	ECanaRegs.CANMIM.all = 0xFFFFFFFF;  
//  CANMIM .BIT.X＝1  邮箱中断被使能（X＝1～31）
//  CANMIM .BIT.X＝0  邮箱中断被禁止（X＝1～31）
	ECanaShadow.CANMC.all = ECanaRegs.CANMC.all; //  把CANMC读入影子寄存器
	ECanaShadow.CANMC.bit.CCR = 1;               //  改变配置请求位
	ECanaShadow.CANMC.bit.SCB = 1;  			//eCAN模式,所有邮箱使能
	ECanaRegs.CANMC.all = ECanaShadow.CANMC.all; //  把配置好的寄存器值回写
  	EDIS;
/*CPU要求对配置寄存器CANBTC和SCC的接收屏蔽寄存器(CANGAM，LAM[0]和LAM[3])进行写操作。对该位置位后，CPU必须等待，直到CANES寄存器的CCE标志位在送入CANBTC寄存器之前为1 */
do 
    {
      ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    } while(ECanaShadow.CANES.bit.CCE != 1 );  //  当CCE＝1时可以对CANBTC进行操作。
    //  配置波特率
    EALLOW;
    ECanaShadow.CANBTC.all = ECanaRegs.CANBTC.all; //  把CANBTC读入影子寄存器
    ECanaShadow.CANBTC.bit.BRPREG = 149;   //  (BRP+1)＝150， 最小时间单位TQ＝1us
    ECanaShadow.CANBTC.bit.TSEG2REG = 2;   //  位定时bit-time＝(TSEG1+1)+(TSEG1+1)+1
    ECanaShadow.CANBTC.bit.TSEG1REG = 3;   //  bit-time＝8us， 所以波特率为125Kpbs       
    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;  //  把配置好的寄存器值回写
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;   //  把CANMC读入影子寄存器
    ECanaShadow.CANMC.bit.CCR = 0 ;       //  设置CCR＝0， CPU请求正常模式
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;   //  把配置好的寄存器值回写
    EDIS;
    do
    {
      ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    } while(ECanaShadow.CANES.bit.CCE != 0 );  //  等待 CCE 位被清零
//  配置eCAN为自测试模式，使能eCAN的增强特性
    EALLOW;
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.STM = 0;        //  配置CAN 为正常模式
  	// CANMC.bit.STM＝0，正常模式，CANMC.bit.STM＝1，自测试模式
    ECanaShadow.CANMC.bit.SCB = 1;        // 选择HECC工作模式
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
    EDIS;
}

void mailbox_read(int16 MBXnbr)
{
  	volatile struct MBOX *Mailbox;
	Mailbox = &ECanaMboxes.MBOX0 + MBXnbr;
	TestMbox1 = Mailbox->MDL.all;    	// 读出当前邮箱数据低4字节
	TestMbox2 = Mailbox->MDH.all;       //  读出当前邮箱数据高4字节
	TestMbox3 = Mailbox->MSGID.all;     //  读出当前邮箱ID
} 

//ADC模块上电延时
void Adc_PowerUP() 
{ 	
  	AdcRegs.ADCTRL3.bit.ADCBGRFDN = 0x3;  //ADC带隙和参考电路加电
  	for (i=0; i<1000000; i++){}      //至少5ms延时
  	AdcRegs.ADCTRL3.bit.ADCPWDN = 1; //ADC核模拟电路加电
  	for (i=0; i<10000; i++){}        //至少20us延时
}

void Adc_Init()
{
    AdcRegs.ADCTRL1.bit.CONT_RUN = 0;      //启动-停止/连续转换选择:启动-停止方式
    AdcRegs.ADCTRL1.bit.CPS = 1;           //核时钟预定标器:ADC_CLK=ADCLKPS/2=3.125M
  	AdcRegs.ADCTRL1.bit.ACQ_PS = 0xf;      //采集窗口大小:SH pulse/clock=16
   	AdcRegs.ADCTRL3.bit.ADCCLKPS = 0x5;    //核时钟分频:ADCLKPS=HSPCLK/4=6.25M
	AdcRegs.ADCMAXCONV.all = 0x0000;       //转换通道数:SEQ1序列的通道数为1
  	AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0xf; //转换通道选择:ADCINB7
  	 
	AdcRegs.ADCTRL2.bit.EVA_SOC_SEQ1=1;  //SEQl被EVA的触发源启动
  	AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1=1;  //使能中断		
}

void LEDdisplay(int LED8,int LED7,int LED6,int LED5,int LED4,int LED3,int LED2,int LED1)
{
         GpioDataRegs.GPADAT.bit.GPIOA11=0; //给LACK信号一个低电平
	   	Write_LED (LED8);//向LED最高位写数据,显示LED8
		Write_LED (LED7);
		Write_LED (LED6);
		Write_LED (LED5);
		Write_LED (LED4);
		Write_LED (LED3);
		Write_LED (LED2);
		Write_LED (LED1);//向LED最低位写数据,显示LED1

     	GpioDataRegs.GPADAT.bit.GPIOA11=1; //给LACK信号一个高电平为锁存74HC595     	
} 

void main(void)
{		
   	asm (" NOP ");
   	asm (" NOP ");
   	InitSysCtrl();      //初始化系统控制寄存器, 时钟频率150M
	EALLOW;				
	SysCtrlRegs.HISPCP.all = 0x0001;//高速时钟的工作频率＝SYSCLKOUT/2=75M
	EDIS;
	DINT;	        //关闭总中断，清除中断标志
	IER = 0x0000;   //关闭外围中断
	IFR = 0x0000;	//清中断标志
	spi_intial();   //SPI初始化子程序
	gpio_init();   
	IOinit();	    // I/O初始化子程序	
	EVA_PWM();	
	EVA_Timer1();
	EVB_PWM();	
	EVB_Timer3 ();
  	Scia_init();//初始化SCl
   	CAN_INIT();

	EALLOW;
	GpioMuxRegs.GPAMUX.all=0x003F;    //EVAPWMl-6
	GpioMuxRegs.GPBMUX.all=0x003F;    //EVBPWM7-12
	EDIS;

//初始化PIE控制寄存器
	InitPieCtrl();       //DSP281x_PieCtrl．C
//初始化PIE中断向量表
	InitPieVectTable();  //DSP281x_PieVect．C
	LEDdisplay(0,0,0,0,0,0,0,0);

//．初始化ADC寄存器
 	Adc_PowerUP();
 	Adc_Init();

//使能Eva、Evb相应位
	EvaRegs.COMCONA.bit.FCOMPOE=1;	//比较单元使能
	EvaRegs.EVAIMRA.bit.T1PINT=1;	//使能T1PINT中断
	EvaRegs.EVAIFRA.bit.T1PINT=1;	//先清

	EvbRegs.COMCONB.bit.FCOMPOE=1;	//比较单元使能
	EvbRegs.EVBIMRA.bit.T3PINT=1;	//使能T3PINT中断
	EvbRegs.EVBIFRA.bit.T3PINT=1;	//先清

//T2：启动AD转换
	T2pr=75000000/FC2;
	EvaRegs.T2PR=T2pr;	//T2定时器周期
	EvaRegs.GPTCONA.bit.T2TOADC=2;	//由定时器2周期中断启动A／D转换
	EvaRegs.T2CON.all=0x084C;
	EvaRegs.T2CNT=0x0000;	//T2计数器清

//T4：按键 ，初始化按键检测中断
	/*EvbRegs.T4PR=12000;	//T4定时器周期
	EvbRegs.T4CON.all=0x1744;	//x／128，增计数10．24 ms
	EvbRegs.T4CNT=0x0000;//T4计数器清

	EvbRegs.EVBIMRB.bit.T4PINT=1;	//使能T4PINT中断
	EvbRegs.EVBIFRB.bit.T4PINT=1;	//先清*/

//．初始化占空比列表
	for(j=0;j<8192;j++)	//在线计算正弦表
	{
	sindata[j]=625-sin(j*PI2/8192)*625*0.85;
	}

//####．初始化Xintf
	InitXintf();	//初始化XINTF

//####中断使能
	EALLOW;
	PieVectTable.T1PINT=&T1pint_isr;//PIE中断矢量表
	PieVectTable.T3PINT=&T3pint_isr;
	PieVectTable.T4PINT=&T4pint_isr;
	PieVectTable.ADCINT=&adc_isr;
	PieVectTable.RXAINT=&Scirxinta_isr;
	PieVectTable.PDPINTA=&pdpaint_isr;	
	PieVectTable.ECAN0INTA=&Ecan0inta_isr;
	EDIS;

	PieCtrlRegs.PIEIER2.bit.INTx4=1;	//使能T1PINT
	PieCtrlRegs.PIEIER4.bit.INTx4=1;	//使能T3PINT
	PieCtrlRegs.PIEIER5.bit.INTx1=1;	//使能T4PINT
	PieCtrlRegs.PIEIER1.bit.INTx6=1;	//使能ADCINT
	PieCtrlRegs.PIEIER9.bit.INTx1=1;	//使能SCIRXlNTA
	PieCtrlRegs.PIEIER9.bit.INTx5=1;	//使能ECAN0lNT
	PieCtrlRegs.PIEIER1.bit.INTx1=1;	//使能PDPAINT

	IER |=(M_INT1 | M_INT2 | M_INT5 |M_INT9 |M_INT4);//使能INTl，INT2，INT5，INT9

	EINT;	//开放全局中断
	ERTM;	//EnableGlobal realtime interrupt DBGM
	deaout = 1000;		// for fixed 1000Hz Output
//主循环
	while (1)
 	{


//	dea1=8192*deaout*2/1000;//中间变量，防计算溢出
	dea1=8192*deaout*1/1000;

	dea=dea1*Tlpr/75000;//计算查表等分值
	deb1=8192*debout*2/1000;
	deb=deb1*T3pr/75000;
	 //四路DA转换
 	    if (KeyIn1() == 1)     // 调用查键K1~K8子程序
 	    {
 	    keyNum=keyNum+1; 	 
	    KeyFunction1(KeyReg1);
		SciaRegs.SCITXBUF=LEDReg+48;  //发送键值,转换成ASCII码0～9.作用：SCI
		if(keyNum>4)
		{
		keyNum=keyNum-4;
		LED4=LEDReg;
		}
 	    if(keyNum==1)
		{
		LED4=LEDReg;
		}
		 if(keyNum==2)
		{
		LED3=LEDReg;
		}
		 if(keyNum==3)
		{
		LED2=LEDReg;
		}
 		if(keyNum==4)
		{
		LED1=LEDReg;
		}

 		if(flag==1)
		{
		debout=LED4*1000+LED3*100+LED2*10+LED1; 
 		LEDdisplay(15,11,17,19,LED4,LED3,LED2,LED1); //显示FB-灭****

		/*EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;*/
		}
		if(flag==0)
		{
		deaout=LED4*1000+LED3*100+LED2*10+LED1;
 		LEDdisplay(15,10,17,19,LED4,LED3,LED2,LED1); //显示FA-灭****
	    }
		/*EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;*/
 	    }

//功能E键:用键盘设定deaout频率;功能F键:用ADCINB7通道设定deaout频率;
//功能C键:用键盘设定debout频率;功能D键:用ADCINB7通道设定debout频率;

		if (KeyIn2() == 1)     // 调用查键K8~K16子程序
 	    {
 	    KeyFunction2(KeyReg2);
		SciaRegs.SCITXBUF=LEDReg+48;  //发送键值，转换成ASCII码0～9
		if(LEDReg==0x0E) 		//判是否为功能E键
		{
		flag=0;
		LEDdisplay(15,10,17,19,LED4,LED3,LED2,LED1); //显示FA-灭****
		deaout=LED4*1000+LED3*100+LED2*10+LED1;   //频率设定值
		/*EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;*/
		}

		if(LEDReg==0x0F)		////判是否为功能F键
		{
	 	flag=3;
		//功能F键:用ADCINB7通道设定deaout频率				
		deaout=AD1;
		
			
		/*EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;*/	
		}	
	
	if(LEDReg==0x0C) 	//判是否为功能C键
		{
		flag=1;          //设置功能C键标志位
		LEDdisplay(15,11,17,19,LED4,LED3,LED2,LED1); //显示FB-灭****
		debout=LED4*1000+LED3*100+LED2*10+LED1;   //频率设定值

		/*EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;*/
		}

		if(LEDReg==0x0D)		//判是否为功能D键
		{
		//功能D键:用ADCINB7通道设定debout频率;			
		debout=AD1;
		flag = 2;
			
		/*EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;*/	
		}

		/*if(LEDReg==0x0B)		////判是否为功能B键 CAN发送
		{	 
	 	if	(flag==1)
		{
		ECanaMboxes.MBOX0.MDL.all = 0x55555502;//发送标志
		ECanaMboxes.MBOX0.MDH.all = debout;
		ECanaRegs.CANTRS.all=0x0001; //发送0邮箱 
		while(ECanaRegs.CANTA.all!=0x0001) {};
		ECanaRegs.CANTA.all=0xFFFF;		//写1清.	
		}
		if	(flag==0)
		{
		ECanaMboxes.MBOX0.MDL.all = 0x55555501;//发送标志
		ECanaMboxes.MBOX0.MDH.all = deaout;
		ECanaRegs.CANTRS.all=0x0001;//发送0邮箱 
		while(ECanaRegs.CANTA.all!=0x0001) {};
		ECanaRegs.CANTA.all=0xFFFF;		//写1清
		}	
	 
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;
		}*/
 	   	keyNum=keyNum+1;		
		if(keyNum>4)
		{
		keyNum=keyNum-4;
		LED4=LEDReg;
		}
 	    if(keyNum==1)
		{
		LED4=LEDReg;
		}
		 if(keyNum==2)
		{
		LED3=LEDReg;
		}
		 if(keyNum==3)
		{
		LED2=LEDReg;
		}
 		if(keyNum==4)
		{
		LED1=LEDReg;
		}
		if(flag==3)
		{
		LEDdisplay((AD2/1000)+20,(AD2%1000)/100,(AD2%100)/10,19,
        AD1/1000,(AD1%1000)/100,(AD1%100)/10,AD1%10);
		}
		if(flag==2)
		{
		LEDdisplay((AD2/1000)+20,(AD2%1000)/100,(AD2%100)/10,19,
        AD1/1000,(AD1%1000)/100,(AD1%100)/10,AD1%10);
        }		
		if(flag==1)
		{
		debout=LED4*1000+LED3*100+LED2*10+LED1;
		LEDdisplay(15,11,17,19,LED4,LED3,LED2,LED1); //显示FB-灭****
		/*EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;*/ 
		}
		if(flag==0)
		{
		deaout=LED4*1000+LED3*100+LED2*10+LED1;   //频率设定值
		LEDdisplay(15,10,17,19,LED4,LED3,LED2,LED1); //显示FA-灭****
 	     }
 	     } 
	/*EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
	return;	*/	
	}
}

interrupt  void  T1pint_isr(void)	//中断T1PINT更新PWMl～6的占空比(60k)
{
	static  long  Ia=0;
	static  long  resa=0;
//####  更新pwm1～6占空比EVA
	x1=Ia*dea;
	x1=resa+x1;//A相角度
	Ia++;	//时间
	if(x1>(sinlen-1))	//如果超出正弦表长，则减去超出正弦表长度
	{
	resa=x1-sinlen;	//A相角度范围限制在0～360
	x1=resa;
	Ia=1;	//重新开始计数，t=0
	}
	x2=x1-2731;	//x2=x1-sinlen／3；B相
	x3=x1-5461;	//x3=xl-sinlen*2／3；C相
	if(x2<0)
	x2=x2+sinlen;	//B相角度范围限制在0～360°
	if(x3<0)	
	x3=x3+sinlen;	//C相角度范围限制在0～360°
	EvaRegs.CMPR1=sindata[x1];	//刷新占空比
	EvaRegs.CMPR2=sindata[x2];
	EvaRegs.CMPR3=sindata[x3];
	//####  复位
	EvaRegs.EVAIFRA.bit.T1PINT=1;	//中断标志清
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP2; //允许下次中断
	return;
}

interrupt void  T3pint_isr (void)
	{
	static  long  Ib=0;
	static  long  resb=0;

//####  更新pwm7～11占空比EVB
	x4=Ib*deb;
	x4=resb+x4;//A相角度
	Ib++;	//时间
	if(x4>(sinlen-1))	//如果超出正弦表长度，则减去超出正弦表长度
	{
	resb=x4-sinlen;	//A相角度范围限制在0～360
	x4=resb;
	Ib=1;	//重新开始计数，t=0
	}
	x5=x4-2731;	//x5=x4-sinlen／3；B相
	x6=x4-5461;	//x6=x4-sinlen*2／3；C相
	if(x5<0)
	x5=x5+sinlen;	//B相角度范围限制在0～360°
	if(x6<0)	
	x6=x6+sinlen;		//C相角度范围限制在0～360°
	EvbRegs.CMPR4=sindata[x4];	//刷新占空比
	EvbRegs.CMPR5=sindata[x5];
	EvbRegs.CMPR6=sindata[x6];
	//####  复位
	EvbRegs.EVBIFRA.bit.T3PINT=1;	//中断标志清
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP4; //允许下次中断
	return;
	}

interrupt void   T4pint_isr(void)
	{
		DAC();  //四路DA转换
 	    /*if (KeyIn1() == 1)     // 调用查键K1~K8子程序
 	    {
 	    keyNum=keyNum+1; 	 
	    KeyFunction1(KeyReg1);
		SciaRegs.SCITXBUF=LEDReg+48;  //发送键值,转换成ASCII码0～9
		if(keyNum>4)
		{
		keyNum=keyNum-4;
		LED4=LEDReg;
		}
 	    if(keyNum==1)
		{
		LED4=LEDReg;
		}
		 if(keyNum==2)
		{
		LED3=LEDReg;
		}
		 if(keyNum==3)
		{
		LED2=LEDReg;
		}
 		if(keyNum==4)
		{
		LED1=LEDReg;
		}

 		if(flag==1)
		{
		debout=LED4*1000+LED3*100+LED2*10+LED1; 
 		LEDdisplay(15,11,17,19,LED4,LED3,LED2,LED1); //显示FB-灭****

		EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;
		}
		deaout=LED4*1000+LED3*100+LED2*10+LED1;
 		LEDdisplay(15,10,17,19,LED4,LED3,LED2,LED1); //显示FA-灭****
	
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;
 	    }

//功能E键:用键盘设定deaout频率;功能F键:用ADCINB7通道设定deaout频率;
//功能C键:用键盘设定debout频率;功能D键:用ADCINB7通道设定debout频率;

		if (KeyIn2() == 1)     // 调用查键K8~K16子程序
 	    {
 	    KeyFunction2(KeyReg2);
		SciaRegs.SCITXBUF=LEDReg+48;  //发送键值，转换成ASCII码0～9
		if(LEDReg==0x0E) 		//判是否为功能E键
		{
		flag=0;
		LEDdisplay(15,10,17,19,LED4,LED3,LED2,LED1); //显示FA-灭****
		deaout=LED4*1000+LED3*100+LED2*10+LED1;   //频率设定值
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;
		}

		if(LEDReg==0x0F)		////判是否为功能F键
		{
	 	flag=0;
		//功能F键:用ADCINB7通道设定deaout频率				
		deaout=AD1;
		LEDdisplay((AD2/1000)+20,(AD2%1000)/100,(AD2%100)/10,19,
        AD1/1000,(AD1%1000)/100,(AD1%100)/10,AD1%10);
			
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;	
		}	
	
	if(LEDReg==0x0C) 	//判是否为功能C键
		{
		flag=1;          //设置功能C键标志位
		LEDdisplay(15,11,17,19,LED4,LED3,LED2,LED1); //显示FB-灭****
		debout=LED4*1000+LED3*100+LED2*10+LED1;   //频率设定值

		EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;
		}

		if(LEDReg==0x0D)		//判是否为功能D键
		{
		//功能D键:用ADCINB7通道设定debout频率;			
		debout=AD1;
		LEDdisplay((AD2/1000)+20,(AD2%1000)/100,(AD2%100)/10,19,
        AD1/1000,(AD1%1000)/100,(AD1%100)/10,AD1%10);
			
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;	
		}

	if(LEDReg==0x0B)		////判是否为功能B键 CAN发送
		{	 
	 	if	(flag==1)
		{
		ECanaMboxes.MBOX0.MDL.all = 0x55555502;//发送标志
		ECanaMboxes.MBOX0.MDH.all = debout;
		ECanaRegs.CANTRS.all=0x0001; //发送0邮箱 
		while(ECanaRegs.CANTA.all!=0x0001) {};
		ECanaRegs.CANTA.all=0xFFFF;		//写1清.	
		}
		if	(flag==0)
		{
		ECanaMboxes.MBOX0.MDL.all = 0x55555501;//发送标志
		ECanaMboxes.MBOX0.MDH.all = deaout;
		ECanaRegs.CANTRS.all=0x0001;//发送0邮箱 
		while(ECanaRegs.CANTA.all!=0x0001) {};
		ECanaRegs.CANTA.all=0xFFFF;		//写1清
		}	
	 
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;
		}
 	   	keyNum=keyNum+1;		
		if(keyNum>4)
		{
		keyNum=keyNum-4;
		LED4=LEDReg;
		}
 	    if(keyNum==1)
		{
		LED4=LEDReg;
		}
		 if(keyNum==2)
		{
		LED3=LEDReg;
		}
		 if(keyNum==3)
		{
		LED2=LEDReg;
		}
 		if(keyNum==4)
		{
		LED1=LEDReg;
		}
				
		if(flag==1)
		{
		debout=LED4*1000+LED3*100+LED2*10+LED1;
		LEDdisplay(15,11,17,19,LED4,LED3,LED2,LED1); //显示FB-灭****
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return; 
		}
		deaout=LED4*1000+LED3*100+LED2*10+LED1;   //频率设定值
		LEDdisplay(15,10,17,19,LED4,LED3,LED2,LED1); //显示FA-灭****
 	     } */
	EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
	return;
	}

interrupt void   adc_isr(void)
	{	
		q++;
   	   AD1=AdcRegs.ADCRESULT0 >> 4;
	   AD2=(AD1*3*1000)/4095;   //实际AD值*1000		

	   AdcRegs.ADCST.bit.INT_SEQ1_CLR=1;//清中断标志
       PieCtrlRegs.PIEACK.all=PIEACK_GROUP1; //允许下次中断       		                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
	return;
	}

interrupt void  Scirxinta_isr(void)
	{
	  EINT;  //允许中断嵌套
	 RecieveChar=SciaRegs.SCIRXBUF.bit.RXDT;
	 SciaRegs.SCITXBUF = RecieveChar;
 	 while(SciaRegs.SCICTL2.bit.TXRDY ==  0){}		
   	 while(SciaRegs.SCICTL2.bit.TXEMPTY ==  0){}
      PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //允许下次中断	
	return;
	}

interrupt void Ecan0inta_isr(void)	//接收信息悬挂中断
	{
	if(ECanaRegs.CANRMP.all==0x00010000)
	{
	ECanaRegs.CANRMP.all=0xFFFF0000;
 	mailbox_read(16);
	if(	TestMbox1==0x55555501)
		{
		deaout=TestMbox2;
		LEDdisplay(15,10,17,19, deaout/1000,(deaout%1000)/100,(deaout%100)/10,deaout%10);	
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //允许下次中断	
		return;
		}
	if(	TestMbox1==0x55555502)
		{
		debout=TestMbox2;
		LEDdisplay(15,11,17,19, debout/1000,(debout%1000)/100,(debout%100)/10,debout%10);
	
		 PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //允许下次中断	
		return;
		}
	}
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //允许下次中断
	return;
	}

	interrupt void  pdpaint_isr(void)
	{
	return;
	}

int KeyIn1(void)
{
 	 EALLOW;  
    //将GPIOB8~GPIOB15配置为输入,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all&0x00ff;
     EDIS;    
    GpioDataRegs.GPEDAT.all = 0xfff8;     //选通KEY低8位
    for (i=0; i<100; i++){}               //延时
    //判K1~K8是否按下
    if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
    {
 //   	for (i=0; i<40000; i++){}        //延时消抖
        if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
        {
	   KeyReg1=GpioDataRegs.GPBDAT.all|0x00ff ;
    		while ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff) //判是否松开
    		{
    			GpioDataRegs.GPDDAT.bit.GPIOD1 = !GpioDataRegs.GPDDAT.bit.GPIOD1;
    			for (i=0; i<1000; i++){}     
    		}
    		return (1);			      
    	}
    }
    return (0);
}

int KeyIn2(void)
{
 	 EALLOW;  
    //将GPIOB8~GPIOB15配置为输入,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all&0x00ff;
     EDIS;    
    GpioDataRegs.GPEDAT.all = 0xfff9;     //选通KEY高8位
    for (i=0; i<100; i++){}               //延时
    //判K8~K16是否按下
    if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
    {
 //   	for (i=0; i<40000; i++){}        //延时消抖
        if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
        {
	   KeyReg2=GpioDataRegs.GPBDAT.all|0x00ff ;
    		while ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff) //判是否松开
    		{
    			GpioDataRegs.GPDDAT.bit.GPIOD1 = !GpioDataRegs.GPDDAT.bit.GPIOD1;
    			for (i=0; i<1000; i++){}     
    		}
    		return (1);			      
    	}
    }
    return (0);
}

void	KeyFunction1(unsigned int KeyReg1)
		{
			switch(KeyReg1)
				{
					case  K1: {
								LEDReg= 0x00; 							
							  }
							  break;
					case  K2: {
								LEDReg= 0x01;							
							  }
							  break;
					case  K3: {
								LEDReg= 0x02;
							  }
							  break;
					case  K4: {
								LEDReg= 0x03;
							  }
							  break;
					case  K5: {
								LEDReg= 0x04;
							  }
							  break;
					case  K6: {
								LEDReg= 0x05;	
							  }
							  break;
					case  K7: {
								LEDReg= 0x06;
							  }
							  break;
					case  K8: {
								LEDReg= 0x07;
							  }	 		 
							  break; 			  
					default:  
							  break;
			}				  				  			  				   			  			  			   		
		}

 void	KeyFunction2(unsigned int KeyReg2)
		{
			switch(KeyReg2)
				{
					case  K9: {
								LEDReg= 0x08; 							
							  }
							  break;
					case  K10: {
								LEDReg= 0x09;							
							  }
							  break;
					case  K11: {
								LEDReg= 0x0A;
							  }
							  break;
					case  K12: {
								LEDReg= 0x0B;
							  }
							  break;
					case  K13: {
								LEDReg= 0x0C;
							  }
							  break;
					case  K14: {
								LEDReg= 0x0D;	
							  }
							  break;
					case  K15: {
								LEDReg= 0x0E;
							  }
							  break;
					case  K16: {
								LEDReg= 0x0F;
							  }	 		 
							  break; 			  
					default:  
							  break;
			}				  				  			  				   			  			  			   		
		}

//MAX5742  四通道DAC转换,SPICCR需切换数据位数，分别指示deaout,debout,AD1,其值〈4095
	
void  DAC(void)
{  
 	SpiaRegs.SPICCR.all =0x00cf;   // 使SPI发送16位数据
 	 asm (" NOP ");
   	asm (" NOP ");
 	GpioDataRegs.GPFDAT.bit.GPIOF14 = 0;   //片选DAC  
	 SpiaRegs.SPITXBUF = 0xf010;    // 初始化指令：DAC_A-D通道使能
   	while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){} 	//发送是否完成
   SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;		//读出清标志
    GpioDataRegs.GPFDAT.bit.GPIOF14 = 1;	//片选高电平延时
    	for(i = 0; i<10; i++){}
   	GpioDataRegs.GPFDAT.bit.GPIOF14 = 0;
	SpiaRegs.SPITXBUF =deaout;          //  DAC_A通道置数
    	while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){}
   	 	SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;
    		GpioDataRegs.GPFDAT.bit.GPIOF14 = 1; 
    		for(i=0; i<10; i++){}

	GpioDataRegs.GPFDAT.bit.GPIOF14 = 0;
	SpiaRegs.SPITXBUF =debout|0x1000;     //  DAC_B通道置数
    	while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){}
  	 	SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;
   		GpioDataRegs.GPFDAT.bit.GPIOF14 = 1; 
    		for(i=0; i<10; i++){}

		GpioDataRegs.GPFDAT.bit.GPIOF14 = 0;
	SpiaRegs.SPITXBUF =AD1|0x2000;     //  DAC_C通道置数
    	while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){}
   	 	SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;
    		GpioDataRegs.GPFDAT.bit.GPIOF14 = 1; 
    		for(i=0; i<10; i++){}		

		GpioDataRegs.GPFDAT.bit.GPIOF14 = 0;
	SpiaRegs.SPITXBUF =AD1|0x3000;          //  DAC_D通道置数
    	while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){}
   	 	SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;
    		GpioDataRegs.GPFDAT.bit.GPIOF14 = 1; 
    		for(i=0; i<10; i++){}

	SpiaRegs.SPICCR.all =0x00c7; // 使SPI处于复位方式,下降沿,8位数据,切换为显示模式   
}

//通过SPI向LED发送显示数据
void Write_LED (int LEDReg)
{
Uint16 LEDcode[30]={0xc000,0xf900,0xA400,0xB000,0x9900,0x9200,0x8200,0xF800,
                    0x8000,0x9000,0x8800,0x8300,0xc600,0xa100,0x8600,0x8e00,
                    0x8c00,0xbf00,0xa700,0xff00,0x4000,0x7900,0x2400,0x3000,
                    0x1900,0x1200,0x0200,0x7800,0x0000,0x1000};//共阳字形码0~f, P，－，L，"灭",0.~9.	
	 		SpiaRegs.SPITXBUF =LEDcode[LEDReg]; //给数码管送数
    	 	 while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){} 		
    		SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF; //读出清标志
}
//=========================================================================================
// No more.
//=========================================================================================

