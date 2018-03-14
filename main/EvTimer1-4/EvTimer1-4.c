
/*事件管理器定时器程序，在中断服务子程序中分别点亮指示灯DS4-DS7*/
#include "DSP281x_Device.h"
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

