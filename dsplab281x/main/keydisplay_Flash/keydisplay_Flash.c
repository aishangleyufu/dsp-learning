/*****************************************************************
** 功能描述: 按键显示程序，对按键记录次数，以二进制显示在16个Led上
*****************************************************************/
#include "DSP281x_Device.h"
unsigned int LEDReg;
unsigned int KeyReg1;
unsigned int KeyReg2;
unsigned long int i = 0;
// 这些变量由连接器定义(见F2812.cmd)。下面3条指令为Flash加载专用
//############################################################################################
extern Uint16 RamfuncsLoadStart;
extern Uint16 RamfuncsLoadEnd;
extern Uint16 RamfuncsRunStart;
//###############################################################################

//按键对应值
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

//共阳字形码0~f, P，－，L，"灭",0.~9.
unsigned  int LEDCode[30]={0xc000,0xf900,0xA400,0xB000,0x9900,0x9200,0x8200,0xF800,
                    0x8000,0x9000,0x8800,0x8300,0xc600,0xa100,0x8600,0x8e00,
                    0x8c00,0xbf00,0xa700,0xff00,0x4000,0x7900,0x2400,0x3000,
                    0x1900,0x1200,0x0200,0x7800,0x0000,0x1000};

 
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

//键扫描子程序K1~K8
int Keyscan1(void)
{
 	 EALLOW;  
    //将GPIOB8~GPIOB15配置为输入,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all&0x00ff;
     EDIS;    
    GpioDataRegs.GPEDAT.all = 0xfff8;    //选通KEY低8位
    for (i=0; i<100; i++){}             //延时
    //判K1~K8是否按下
    if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
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
//按键记录次数，显示在16个LED上
void LedOut(Uint16 led)
{
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

//键散转子程序K1~K8
void  KeyFunction1(unsigned int KeyReg1)
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

//键散转子程序K9~K16
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
//显示子程序
void display (LEDReg)
{
	GpioDataRegs.GPADAT.bit.GPIOA11=0; //给LACK信号一个低电平	
	 		SpiaRegs.SPITXBUF =LEDCode[LEDReg]; //给数码管送数
    	 	 while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){} 		
    		SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;	
     
     		GpioDataRegs.GPADAT.bit.GPIOA11=1; //给LACK信号一个高电平为锁存74HC595
     	  	for(i=0;i<10;i++){}	//延时
}


void main(void)
{
    Uint16 keyNum = 0x0000;  //按键次数初始化
    InitSysCtrl();  // 系统初始化程序	
 	DINT;           //关闭总中断
	spi_intial();   //SPI初始化子程序
	gpio_init();    // I/O初始化子程序
	IER = 0x0000;   // 关闭外围中断
	IFR = 0x0000;   // 清中断标志	
		 // RamfuncsLoadStart, RamfuncsLoadEnd,及RamfuncsRunStart符号由连接器生成。
		// 参考F2812.cmd文件,下面2条指令为Flash加载专用
//############################################################################################
   MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);		
   InitFlash();		// 调用Flash初始化函数以便设置Flash等待状态。这个函数必须驻留在RAM中。
//###########################################################################################	
	LedOut(keyNum); // LED指示灯初始化
	for(i=0;i<8;i++)// 8个数码管清0	
		{
			SpiaRegs.SPITXBUF =LEDCode[0]; //给数码管送数
    		while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){} 		
    		SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;	
     	}
     		GpioDataRegs.GPADAT.bit.GPIOA11=1; //给LACK信号一个高电平为锁存74HC595
     		for(i=0;i<10;i++){}	
		
 	
	while (1)
	{
 	    if (Keyscan1() == 1)     // 调用键扫描K1~K8子程序
 	    {
 	    	keyNum=keyNum+1;// 按键记数	 
	    	KeyFunction1(KeyReg1);
	        display (LEDReg);//显示键值			
 	     	 LedOut(keyNum);//显示按键次数
 	    }
		if (Keyscan2() == 1)     // 调用键扫描K8~K16子程序
 	    {
 	   
 	   		keyNum=keyNum+1; 	  
	    	KeyFunction2(KeyReg2);
			display (LEDReg);//显示键值		
 	     	LedOut(keyNum);//显示按键次数
 	     }
    }
}
