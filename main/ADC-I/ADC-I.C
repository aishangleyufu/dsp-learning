//打开ADCIprj，同时把CPUtimer0里面的那些小C文件直接拖动到ADCIprj中

/**************************************************************
** 功能描述: ADC程序，对ADCINB7通道采样，中断方式**
**************************************************************/

#include "DSP281x_Device.h"
#include "DSP281x_Examples.h"   // DSP281x Examples Include File
unsigned long int  AD1=2000;  
unsigned long int  AD2; //实际AD值*1000
unsigned long int  sum=0;

//时钟相关定义
Uint16 Timer_10ms=0;//计时器10ms计数
Uint16 Clock_10ms=0;//时钟10ms计数
Uint16 Clock_ss=45;//时钟秒
Uint16 Clock_mm=59;//时钟分
Uint16 Clock_hh=23;//时钟时
Uint16 BDAT;
Uint16 BDIR;

interrupt void cpu_timer0_isr(void);//中断声明
interrupt void adc_isr(void);
void Write_LED (int LEDReg);//通过SPI向LED发送显示数据




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
    EDIS;
    GpioDataRegs.GPADAT.bit.GPIOA11=0; 	//GPIOA11=0;该端口为74HC595锁存信号
}

/*LED显示子程序*/
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
/*子程序结束*/
unsigned long int i = 0;

//ADC相关寄存器初始化
void Adc_Init()
{
	// Configure ADC @SYSCLKOUT = 150Mhz
    AdcRegs.ADCTRL1.bit.SEQ_CASC = 0;      //双序列/级连选择:双序列工作模式
    AdcRegs.ADCTRL3.bit.SMODE_SEL = 0;     //连续/并发选择:连续采样方式
    AdcRegs.ADCTRL1.bit.CONT_RUN = 0;      //启动-停止/连续转换选择:启动-停止方式
    AdcRegs.ADCTRL1.bit.CPS = 1;           //核时钟预定标器:ADC_CLK=ADCLKPS/2=3.125M
  	AdcRegs.ADCTRL1.bit.ACQ_PS = 0xf;      //采集窗口大小:SH pulse/clock=16
   	AdcRegs.ADCTRL3.bit.ADCCLKPS = 0x2;    //核时钟分频:ADCLKPS=HSPCLK/4=6.25M
	AdcRegs.ADCMAXCONV.all = 0x0000;       //转换通道数:SEQ1序列的通道数为1
  	AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0xf; //转换通道选择:ADCINB7 
}

//ADC模块上电延时
void Adc_PowerUP() 
{ 	
  	AdcRegs.ADCTRL3.bit.ADCBGRFDN = 0x3;     //ADC带隙和参考电路加电
  	for (i=0; i<1000000; i++){}      //至少5ms延时
  	AdcRegs.ADCTRL3.bit.ADCPWDN = 1;		 //ADC核模拟电路加电
  	for (i=0; i<10000; i++){}        //至少20us延时
}


//定义全局变量:
Uint16 LoopCount;//无效循环计数
Uint16 ConversionCount;//当前结果数0-20
Uint16 Voltage[20];//最后20个ADCRESULT0值

main() 
{
    InitSysCtrl();   

    EALLOW;
    SysCtrlRegs.HISPCP.all = 0x3;	// HSPCLK = SYSCLKOUT/6，得25HZ
    EDIS;

	DINT;                           /*关CPU中断*/
	IER = 0x0000;                   //关闭外围中断
	IFR = 0x0000;                    //清中断标志
	spi_intial();                    //SPI初始化子程序
	gpio_init();	                 //GPIO初始化子程序
	Adc_PowerUP();
	Adc_Init();
	
	InitPieCtrl();       //初始化PIE控制寄存器 DSP281x_PieCtrl.c

	InitPieVectTable();	//初始化PIE矢量表DSP281x_PieVect.c       
        InitAdc();    //初始化ADC模块，该函数在DSP28_Adc.c文件中
	
        EALLOW;	//使能写保护寄存器的写操作
	PieVectTable.ADCINT = &adc_isr;        //把用户中断服务的入口地址
                                               //赋给中断向量表头文件中的对应向量
        PieVectTable.TINT0 = &cpu_timer0_isr;//PIE中断矢量表
	EDIS;       // 禁止写保护寄存器的写操作

	   InitCpuTimers();   // For this example, only initialize the Cpu Timers
	//定时器的一般操作如下：将周期寄存器的PRDH:PRD中的值装入32位计数寄存器TIMH:TIM。
	//然后计数寄存器以C28X 的SYSCLKOUT速率递减。当计数器减到0时，就会产生一个定时器
	//中断输出信号（一个中断脉冲）。
	// 150MHz CPU Freq, 1 second Period (in uSeconds)
	//产生中断的间隔=（周期寄存器中的值）/SYSCLKOUT/分频系数
	//周期寄存器中的值=(Freq*Period)
	   ConfigCpuTimer(&CpuTimer0, 150, AD1*5);//因为精确到小数点后两位，所以周期定位10ms
	   StartCpuTimer0();


        PieCtrlRegs.PIEIER1.bit.INTx6 = 1; //使能PIE中的ADCINT中断
        PieCtrlRegs.PIEIER1.bit.INTx7 = 1;//CPU-Timer0使能位于PIE第1组第7个，将其使能
/*调试时刻注意此处的PIEIER1的位置是否冲突。*/

	IER |= M_INT1;					// // 使能 CPU 中断 1，使能全部 INT1

	EINT;   								// 使能全局中断 INTM
	ERTM;	  							// 使能全局实时中断 DBGM
    LoopCount = 0;           				//循环计数器清零
    ConversionCount = 0;      				//当前转化结果数清零
// 配置 ADC
    AdcRegs.ADCMAXCONV.all = 0x0000;       // 设置SEQ1的1个转化通道
  	AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0xf; //转换通道选择:ADCINB7 

    AdcRegs.ADCTRL2.bit.EVA_SOC_SEQ1 = 1;  // 使能 EVASOC 去启动 SEQ1
    AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1 = 1;  // 使能 SEQ1 中断 (每次 EOS)
// 配置 EVA
// 假设EVA已经在 InitSysCtrl()中使能;
    EvaRegs.T1CMPR =0x0080;               // 设置 T1 比较值
    EvaRegs.T1PR = 0xFFFF;                 // 设置周期寄存器
    EvaRegs.GPTCONA.bit.T1TOADC = 1; // 使能EVA中的 EVASOC(下溢中断启动ADC) 
    EvaRegs.T1CON.all = 0x1042;        // 使能定时器1比较操作 (增计数模式 )
//等ADC转换
    while (1)
    {
   				
     LEDdisplay(
					Clock_hh/10,
					Clock_hh%10,
					17,         //斜杠
					Clock_mm/10,
					Clock_mm%10,
					17,
					Clock_ss/10,
					Clock_ss%10
					);
    }
}
interrupt void  adc_isr(void)
{
  Voltage[ConversionCount] =( AdcRegs.ADCRESULT0>>4);
 
// 如果已记录了20结果则重新开始转换
  if(ConversionCount == 19) 
  {
     ConversionCount = 0;
   		AD1=sum/20;
		AD2=(AD1*3*1000)/4095;   //实际AD值*1000
   		sum=0;
  }
  else 
  {
     sum=sum+Voltage[ConversionCount];
     ConversionCount++;
  } 	
// 重新初始化下一次ADC转换
  AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1;         // 复位 SEQ1
  AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;		// 清 INT SEQ1位
  PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;   // 清中断应答信号，准备接收下一次中断
  return;                      
}

//中断服务程序
interrupt void cpu_timer0_isr(void)
{

	//BDAT=GpioDataRegs.GPBDAT.all;
	//BDIR=GpioMuxRegs.GPEDIR.all;

	Clock_10ms=Clock_10ms+AD1/100+1;

	if(Clock_10ms>100)
	{
		Clock_10ms=0;
		if(++Clock_ss>=60)
		{
			Clock_ss=0;
			if(++Clock_mm>=60)
			{
				Clock_mm=0;
				if(++Clock_hh>=24)
				{
					Clock_hh=0;
				}
			}
		}
	}
	//GpioDataRegs.GPBDAT.all=BDAT;
	//GpioMuxRegs.GPEDIR.all=BDIR;

	PieCtrlRegs.PIEACK.all=0x0001;//清除中断应答信号，准备接收下一次中断??????????????????
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

