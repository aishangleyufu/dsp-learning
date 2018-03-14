//文件:EvTimer1-4 X_Flash.c
//功能:这个程序设置EVA定时器1，EVA定时器2，EVB定时器3及EVB定时器4触发一个定时器
//		周期溢出中断。中断服务子程序中分别点亮指示灯DS4-DS7*/

// 条  	件:	这个程序需将其配制成"引导到Flash"("boot to Flash")的操作方式
// 将EvTimer1-4在RAM中运行改变为在Flash中运行的步骤。
//
//   (1) 改变连接器cmd 文件以便反映flash 存储器映射。本例用使用DSP281x F2812.cmd文件
//		 替代通常使用的F2812_EzDSP_RAM_lnk.cmd文件。	 
//   (2) 确认在Flash地址0x3F7FF6入口处有一个开始运行代码的分支指令。本例采用在项目栏
//		 的initialization目录加入DSP281x_CodeStartBranch.asm 文件实现这一点。
//	 (3) 在项目栏的initialization目录加入DSP281x_CSMPasswords.asm文件，必要时可对加载到Flash
//		 的代码进行加密处理。
//	 (4) 在项目栏的initialization目录加入DSP281x_MemCopy.c文件，以便对存储器进行拷贝。
//	 (5) 在F2812.cmd文件中定义的用于存储器拷贝的3个外部变量RamfuncsLoadStart，
//		 RamfuncsLoadEnd，RamfuncsRunStart，必须main函数头部进行外部变量声明，见标注。
//	 (6) 在主函数中嵌入MemCopy()及InitFlash()函数，见标注。
//	 (7) 设置引导方式跳线为引导到Flash(boot to Flash)。	
//	 (8) 为了使程序从Flash更好地运行，修改等待状态并且使能如本例指出的flash流水线。
//        
// 观察变量:      EvaTimer1InterruptCount;
//                EvaTimer2InterruptCount;
//                EvbTimer3InterruptCount;
//                EvbTimer4InterruptCount;

#include "DSP281x_Device.h"
// 这些变量由命令文件定义(见F2812.cmd)。下面3条指令为Flash加载专用
//############################################################################################
extern Uint16 RamfuncsLoadStart;
extern Uint16 RamfuncsLoadEnd;
extern Uint16 RamfuncsRunStart;
//###########################################################################################
unsigned long int i = 0;
void IOinit(void);
void LedOut(Uint16 led);
void Delay(Uint16  data);
void InitEv(void);

void InitEv(void)
{
    //初始化EVA寄存器 Timer1     
    EvaRegs.GPTCONA.all = 0;      
    EvaRegs.T1PR = 0x8000;      // 设置定时周期
    EvaRegs.T1CMPR = 0x0000;    // 比较寄存器    
    EvaRegs.EVAIMRA.bit.T1PINT = 1;//使能T1PINT中断
    EvaRegs.EVAIFRA.bit.T1PINT = 1;//先清标志位  
    EvaRegs.T1CNT = 0x0000; //计数器清零
    EvaRegs.T1CON.all = 0x1742;//连续增计数，128分频，使能比较，打开定时器
    // Start EVA ADC Conversion on timer 1 Period interrupt
    EvaRegs.GPTCONA.bit.T1TOADC = 2;

    //初始化EVA寄存器 Timer 2:   
    EvaRegs.GPTCONA.all = 0;    
    EvaRegs.T2PR = 0xFF00;     // 设置定时周期
    EvaRegs.T2CMPR = 0x0000;    // 比较寄存器
    EvaRegs.EVAIMRB.bit.T2PINT = 1;	//使能T2PINT中断
    EvaRegs.EVAIFRB.bit.T2PINT = 1;	//先清标志位    
    EvaRegs.T2CNT = 0x0000; 	// 计数器清零
    EvaRegs.T2CON.all = 0x1742;	//连续增计数，128分频，使能比较，打开定时器
    // Start EVA ADC Conversion on timer 2 Period interrupt
    EvaRegs.GPTCONA.bit.T2TOADC = 2;

 //初始化EVA寄存器 Timer 3:
    EvbRegs.GPTCONB.all = 0; 
    EvbRegs.T3PR = 0x7000;       // 设置定时周期
    EvbRegs.T3CMPR = 0x0000;     // 比较寄存器   
    EvbRegs.EVBIMRA.bit.T3PINT = 1;	//使能T3PINT中断
    EvbRegs.EVBIFRA.bit.T3PINT = 1;	//先清标志位   
    EvbRegs.T3CNT = 0x0000;	// 计数器清零
    EvbRegs.T3CON.all = 0x1742;	//连续增计数，128分频，使能比较，打开定时器
    // Start EVA ADC Conversion on timer 3 Period interrupt
    EvbRegs.GPTCONB.bit.T3TOADC = 2;

 //初始化EVA寄存器 Timer4:
    EvbRegs.GPTCONB.all = 0; 
    EvbRegs.T4PR = 0xF000;    // 设置定时周期
    EvbRegs.T4CMPR = 0x0000;   //比较寄存器
    EvbRegs.EVBIMRB.bit.T4PINT = 1;	//使能T4PINT中断
    EvbRegs.EVBIFRB.bit.T4PINT = 1;	//先清标志位   
    EvbRegs.T4CNT = 0x0000;	//计数器清零
    EvbRegs.T4CON.all = 0x1742;	//连续增计数，128分频，使能比较，打开定时器
    // Start EVA ADC Conversion on timer 4 Period interrupt
    EvbRegs.GPTCONB.bit.T4TOADC = 2;
}
	
void InitGpio(void)  //IO初始化
{
    EALLOW;
    //将GPIOE0~GPIOE2配置为一般I/O口输出,作138译码  
 	GpioMuxRegs.GPEMUX.all = GpioMuxRegs.GPEMUX.all&0xfff8; 
	GpioMuxRegs.GPEDIR.all = GpioMuxRegs.GPEDIR.all|0x0007;
    //将GPIOB8~GPIOB15配置为一般I/O口,D0~D7
	GpioMuxRegs.GPBMUX.all = GpioMuxRegs.GPBMUX.all&0x00ff;
	 //将GPIOB8~GPIOB15配置为输出,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all|0xff00; 
	EDIS;
}

interrupt void eva_timer1_isr(void);	//中断子程序的声明
interrupt void eva_timer2_isr(void);
interrupt void evb_timer3_isr(void);
interrupt void evb_timer4_isr(void);

// Global counts used in this example
Uint32	EvaTimer1InterruptCount;
Uint32  EvaTimer2InterruptCount;
Uint32	EvbTimer3InterruptCount;
Uint32  EvbTimer4InterruptCount;

void main(void)
{
	InitSysCtrl();	/*初始化系统*/	
	DINT;	/*关中断*/
	IER = 0x0000;
	IFR = 0x0000;	
	InitPieCtrl();	/*初始化PIE*/
	InitPieVectTable();	/*初始化PIE中断矢量表*/	
    InitEv();	/*初始化事件管理器*/  
	InitGpio(); /*初始化IO端口*/ 
	InitXIntrupt();
	 // RamfuncsLoadStart, RamfuncsLoadEnd,及RamfuncsRunStart符号由命令文件生成。
		// 参考F2812.cmd文件,下面2条指令为Flash加载专用
//############################################################################################
   MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);		
   InitFlash();		// 调用Flash初始化函数以便设置Flash等待状态。这个函数必须驻留在RAM中。
//############################################################################################
	LedOut(0);	 // 16个LED都熄灭	
	
	EALLOW;	
	PieVectTable.T1PINT = &eva_timer1_isr;	//PIE中断矢量表
	PieVectTable.T2PINT = &eva_timer2_isr;
	PieVectTable.T3PINT = &evb_timer3_isr;
	PieVectTable.T4PINT = &evb_timer4_isr;
	EDIS; 
    
    PieCtrlRegs.PIEIER2.all = M_INT4;	//使能T1PINT    
    PieCtrlRegs.PIEIER3.all = M_INT1;   //使能T2PINT    
    PieCtrlRegs.PIEIER4.all = M_INT4;	//使能T3PINT    
    PieCtrlRegs.PIEIER5.all = M_INT1;	//使能T4PINT	
    
	IER |= (M_INT2 | M_INT3 | M_INT4 | M_INT5);	//使能INT2，INT3，INT4，INT5   
	
	EINT;   //开放全局中断
	ERTM;	// Enable Global realtime interrupt DBGM		
	for(;;);

} 	

/*中断服务子程序*/

interrupt void eva_timer1_isr(void)
{
	EvaTimer1InterruptCount++;	
 	LedOut(1);	//指示灯DS4（IOOUT0）亮
    // Enable more interrupts from this timer
	EvaRegs.EVAIMRA.bit.T1PINT = 1;
    EvaRegs.EVAIFRA.all = BIT7;	//中断标志清
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;	//允许下次中断
}


interrupt void eva_timer2_isr(void)
{
	EvaTimer2InterruptCount++;
	 LedOut(2);	//指示灯DS5（IOOUT1）亮	
    // Enable more interrupts from this timer
	EvaRegs.EVAIMRB.bit.T2PINT = 1;
    EvaRegs.EVAIFRB.all = BIT0;	//中断标志清
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3	;//允许下次中断
}

interrupt void evb_timer3_isr(void)
{
	EvbTimer3InterruptCount++;
	LedOut(4);	//指示灯DS6（IOOUT2）亮		
    EvbRegs.EVBIFRA.all = BIT7;	//中断标志清
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;//允许下次中断
}

interrupt void evb_timer4_isr(void)
{
	EvbTimer4InterruptCount++;
	LedOut(8);	//指示灯DS7（IOOUT3）亮
    EvbRegs.EVBIFRB.all = BIT0;	//中断标志清
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP5;//允许下次中断
}


void	Delay(Uint16  data)
{
	Uint16	i;
	for (i=0;i<data;i++) { ; }	
}	

void LedOut(Uint16 led)
{
 
	GpioDataRegs.GPEDAT.all = 0xfffb;   //LEDB置0
	GpioDataRegs.GPBDAT.all = ~led;
	for (i=0; i<100; i++){}             //延时
    GpioDataRegs.GPEDAT.all = 0xffff;   //锁存高8位	
	GpioDataRegs.GPEDAT.all = 0xfffa;   //LEDA置0
	GpioDataRegs.GPBDAT.all = ~(led<<8);
	for (i=0; i<100; i++){}
    GpioDataRegs.GPEDAT.all = 0xffff;   //锁存低8位			
}
//===========================================================================
// No more.
//===========================================================================

