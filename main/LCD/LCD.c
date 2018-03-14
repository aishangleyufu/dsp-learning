#include "DSP281x_Device.h"
#include "DSP281x_Examples.h"   // DSP281x Examples 头文件
#include "math.h"


interrupt void cpu_timer0_isr(void);
//--------------------------------------------------------


unsigned int LEDReg;   //键值转换后实际的键值
unsigned int KeyReg1;  //键盘低8位（未转换值）
unsigned int KeyReg2;  //键盘高8位（未转换值） 


//--------------------------------------------------------

Uint16 LedDirection = 0;
Uint16 RunToMid = 0;
Uint16 LedRun = 0;
Uint16 LedAB = 0x0001;
Uint16 LedC =0x8001;

//按键对应值
#define	  K1		0xfeff   //与keyreg1、2做比较，高八位的值对应GPIOB8~15
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

//共阳字形码0~f, P，－，L，"灭",0.~9.
unsigned  int LEDCode[30]={0xc000,0xf900,0xA400,0xB000,0x9900,0x9200,0x8200,0xF800,
                    0x8000,0x9000,0x8800,0x8300,0xc600,0xa100,0x8600,0x8e00,
                    0x8c00,0xbf00,0xa700,0xff00,0x4000,0x7900,0x2400,0x3000,
                    0x1900,0x1200,0x0200,0x7800,0x0000,0x1000}; 

//键扫描子程序K1~K8
int Keyscan1(void)
{
	Uint16 i;
 	 EALLOW;  
    //将GPIOB8~GPIOB15配置为输入,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all&0x00ff;
     EDIS;    
    GpioDataRegs.GPEDAT.all = 0xfff8;    //选通KEY低8位
    for (i=0; i<100; i++){}             //延时
    //判K1~K8是否按下
    if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)  //B8~15口某一位是0，说明有按键按下
    {
    	for (i=0; i<30000; i++){}        //延时消抖
        if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
        {
	   KeyReg1=GpioDataRegs.GPBDAT.all ;//读键值
    		while ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff) //判K1~K8是否松开
    		{
    			GpioDataRegs.GPDDAT.bit.GPIOD1 = !GpioDataRegs.GPDDAT.bit.GPIOD1; 
    			for (i=0; i<1000; i++){}     
    		}
    		return (1);			      
    	}
    }
    return (0);
}

//键扫描子程序K9~K16
int Keyscan2(void)
{
	Uint16 i;
 	 EALLOW;  
    //将GPIOB8~GPIOB15配置为输入,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all&0x00ff;
     EDIS;    
    GpioDataRegs.GPEDAT.all = 0xfff9;     //选通KEY高8位
    for (i=0; i<100; i++){}               //延时
    //判K8~K16是否按下
    if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
    {
    	for (i=0; i<30000; i++){}        //延时消抖
        if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
        {
	   		KeyReg2=GpioDataRegs.GPBDAT.all ;//读键值
    		while ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff) //判K8~K16是否松开
    		{
    			GpioDataRegs.GPDDAT.bit.GPIOD1 = !GpioDataRegs.GPDDAT.bit.GPIOD1;
    			for (i=0; i<1000; i++){}     
    		}
    		return (1);			      
    	}
    }
    return (0);
}


//键散转子程序K1~K8
void  KeyFunction1(unsigned int KeyReg1)
		{
			switch(KeyReg1|0x00FF)
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

//键散转子程序K9~K16
void	KeyFunction2(unsigned int KeyReg2)
		{
			switch(KeyReg2|0x00FF)
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

void LedOut(Uint16 led)
{
 	Uint16	i;
 	EALLOW;  
    //将GPIOB8~GPIOB15配置为输出,D0~D7     
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all|0xff00;
	EDIS;	
	GpioDataRegs.GPEDAT.all = 0xfffb;    //LEDB选通
	GpioDataRegs.GPBDAT.all = ~led;		//显示高8位
	for (i=0; i<100; i++){}              //延时
    GpioDataRegs.GPEDAT.all = 0xffff;    //锁存高8位  	
	GpioDataRegs.GPEDAT.all = 0xfffa;    //LEDA选通
	GpioDataRegs.GPBDAT.all = ~(led<<8);//显示低8位
	for (i=0; i<100; i++){}
    GpioDataRegs.GPEDAT.all = 0xffff;    //锁存低8位			
}

void spi_intial()
 {
 	SpiaRegs.SPICCR.all =0x0047;   // 使SPI处于复位方式, 下降沿, 八位数据  
	SpiaRegs.SPICTL.all =0x0006;   // 主控模式, 一般时钟模式,使能talk, 关闭SPI中断.
	SpiaRegs.SPIBRR =0x007F;       //配置波特率
	SpiaRegs.SPICCR.all =SpiaRegs.SPICCR.all|0x0080;  // 退出复位状态	 
	EALLOW;	
    GpioMuxRegs.GPFMUX.all|=0x000F;	// 设置通用引脚为SPI引脚	 	
    EDIS;
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
    GpioDataRegs.GPADAT.bit.GPIOA11=0; 	//GPIOA11=0;该端口为74HC595锁存信号
}


void IOinit() //I/O口初始化
{
 	EALLOW;  
 	 //将GPIOB8~GPIOB15配置为一般I/O口,D0~D7
	GpioMuxRegs.GPBMUX.all = GpioMuxRegs.GPBMUX.all&0x00ff;
	 //将GPIOB8~GPIOB15配置为输出,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all|0xff00;
    //将GPIOE0~GPIOE2配置为一般I/O口输出,作138译码 	  
 	GpioMuxRegs.GPEMUX.all = GpioMuxRegs.GPEMUX.all&0xfff8; 
	GpioMuxRegs.GPEDIR.all = GpioMuxRegs.GPEDIR.all|0x0007;
  	EDIS;			
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


void main(void)
{

// Step 1. //系统初始化(在DSP281x_SysCtrl.c中)
// PLL, WatchDog, enable Peripheral Clocks 
   InitSysCtrl();//系统初始化

// Step 2. 初始化 GPIO:
   IOinit();
   
// Step 3. 清中断及初始化PIE矢量表:
   DINT;	/*关CPU中断*/

   InitPieCtrl();//初始化PIE控制寄存器 DSP281x_PieCtrl.c   
   IER = 0x0000;//关闭外围中断
   IFR = 0x0000;//清中断标志
 
   InitPieVectTable();//初始化PIE矢量表DSP281x_PieVect.c
  
   EALLOW;  //写EALLOW保护寄存器
   PieVectTable.TINT0 = &cpu_timer0_isr;//PIE中断矢量表
   EDIS;    

   InitCpuTimers();   // For this example, only initialize the Cpu Timers
//定时器的一般操作如下：将周期寄存器的PRDH:PRD中的值装入32位计数寄存器TIMH:TIM。
//然后计数寄存器以C28X 的SYSCLKOUT速率递减。当计数器减到0时，就会产生一个定时器
//中断输出信号（一个中断脉冲）。
// 150MHz CPU Freq, 0.5 second Period (in uSeconds)
//产生中断的间隔=（周期寄存器中的值）/SYSCLKOUT/分频系数
//周期寄存器中的值=(Freq*Period)
   ConfigCpuTimer(&CpuTimer0, 150, 500000);
   StartCpuTimer0();

   IER |= M_INT1;	//使能CPU INT1 

   PieCtrlRegs.PIEIER1.bit.INTx7 = 1;//CPU-Timer0使能位于PIE第1组第7个，将其使能

   EINT;   //开放全局中断 

	for(;;)
	{
		if (Keyscan2() == 1)     // 调用键扫描K8~K16子程序
 	    {	  
	    	KeyFunction2(KeyReg2);
			switch(LEDReg)
			{
				case 0x0A:
					LedDirection = 1;  //A键，跑马灯从右到左
					RunToMid = 0;
					break;

				case 0x0B:
					LedDirection = 0;  //B键，跑马灯从左到右
					RunToMid = 0;
					break;

				case 0x0C:
					RunToMid = 1;  //C键，跑马灯从两边向中间跑
					break;
			}
 	    }	
		
		if (LedRun == 1)    //跑马灯程序
		{
			LedRun = 0;

			if (RunToMid == 1)  //从两边向中间
			{
				LedOut(LedC);
				if(LedC == 0x0180 )
				{
					LedC = 0x8001;	
				}
				else
				{
				 	LedC = ((LedC & 0xFF00)>>1) + ((LedC & 0x00FF)<<1);	
				}						
			}
			else if(LedDirection == 1)  //从右向左
			{
				LedOut(LedAB);
				if(LedAB == 0x0001)
				{
					LedAB = 0x8000;
				}
				else
				{
					LedAB = LedAB >>1;	
				}
			}
			else if(LedDirection == 0)  //从左向右
			{
				LedOut(LedAB);
				if(LedAB == 0x8000)
				{
					LedAB = 0x0001;
				}
				else
				{
					LedAB = LedAB <<1;	
				}
			}
		}
	
	
	}

}

interrupt void cpu_timer0_isr(void)
{
	 LedRun=1;  
     PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;// 清中断应答信号，准备接收下一次中断
}

void Delay(Uint16  data)
{
	Uint16	i;
	for (i=0;i<data;i++) { ; }	
}


//===========================================================================
// No more.
//===========================================================================




   