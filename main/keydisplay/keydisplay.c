#include "DSP281x_Device.h"
#include "DSP281x_Examples.h"

void Timer(void);		 			//时钟计时
interrupt void INT_1_7(void);		//CPU_timer0定时器中断
void InitCputimer(void);			//CPU_timer0初始化
void LEDdisplay(Uint16* led);
void Write_LED (Uint16 LEDReg);
void spi_intial();

//*定时器变量
Uint16 cnt_2ms=0;				//定时器2ms计数
Uint16 cnt_500ms=0;				//定时器500ms计数
extern Uint16 cnt_second_l=0;   //定时器秒计数低位		/分成高位和低位便于按键修改
extern Uint16 cnt_min_h=0;      //定时器分计数高位
extern Uint16 cnt_min_l=9;      //定时器分计数低位
extern Uint16 cnt_hour_l=1;     //定时器时计数低位

Uint16 ledred=0x01;				//led初值 
Uint16 LED[8]={0};				//*显示buf
extern Uint16 state=1;			//*当前模式

unsigned long int i = 0;		//*随机变量

int main(void) {
		InitSysCtrl(); 					//系统初始化
		EALLOW;
		SysCtrlRegs.HISPCP.all = 0x3;	//高速外设时钟HSPCLK=SYSCLKOUT/6=25Mhz
		EDIS;
		DINT;
		spi_intial();                   //SPI初始化
		InitCputimer();					//CPU_timer0初始化
		EALLOW;
		PieVectTable.TINT0= & INT_1_7;	//中断向量赋值
		asm("  and IFR,#00H");
		asm("  or  IER,#01H");
		EINT;							//开总中断
		EDIS;
		while (1);
	}

void InitCputimer(void)
{	EALLOW;
 	CpuTimer0Regs.TPR.all = 149;
 	CpuTimer0Regs.TPRH.all= 0;
 	CpuTimer0Regs.PRD.all = 1999;
 	CpuTimer0Regs.TCR.all =0xf000;
 	PieCtrlRegs.PIEIFR1.all=0x0000;
 	PieCtrlRegs.PIEIER1.all=0x0040;
 	PieCtrlRegs.PIECRTL.bit.ENPIE=0x1;
 	PieCtrlRegs.PIEACK.all =0x1;
 	EDIS;
}


interrupt void INT_1_7(void)			//中断服务程序
{
		Timer();

        switch(state){	
		 case 1:{
			 LED[7]=2;
			 LED[6]=0;
			 LED[5]=17;
			 LED[4]=cnt_hour_l;
			 LED[3]=cnt_min_h;
			 LED[2]=cnt_min_l;
			 LED[1]=cnt_second_l;
			 LED[0]=17;
				}break;
		 }
		LEDdisplay(LED);
   
  PieCtrlRegs.PIEACK.all =0x1;
  CpuTimer0Regs.TCR.all  =0xf000;
}

void Timer(void)
{
	cnt_2ms++;
	if(cnt_2ms>=250){
	cnt_2ms=0;
	cnt_500ms++;
	if(cnt_500ms>=2){
		cnt_500ms=0;
		cnt_second_l++;
		if(cnt_second_l>=5){
			cnt_second_l=0;
			   cnt_min_l++;
			   if(cnt_min_l>=10){
				   cnt_min_l=0;
				   cnt_min_h++;
				   }else if(cnt_min_l == 1 && cnt_min_h == 1){
				   cnt_min_l=0;
				   cnt_min_h=0;
				   cnt_hour_l++;
				  if(cnt_hour_l>=2){
					cnt_hour_l=0;

						  }
					   }
				   }
			   }
			}
		
	}

void spi_intial()
 {
 	SpiaRegs.SPICCR.all =0x0047;   // 使SPI处于复位方式, 下降沿, 八位数据
	SpiaRegs.SPICTL.all =0x0006;   // 主控模式, 一般时钟模式,使能talk, 关闭SPI中断.
	SpiaRegs.SPIBRR =0x007F;       //配置波特率
	SpiaRegs.SPICCR.all =SpiaRegs.SPICCR.all|0x0080;  // 退出复位状态
	EALLOW;
    GpioMuxRegs.GPFMUX.all|=0x000F;	// 设置通用引脚为SPI引脚

	GpioMuxRegs.GPAMUX.bit.TDIRA_GPIOA11=0; //GPIOA11设置为一般I/O口
	GpioMuxRegs.GPADIR.bit.GPIOA11=1;		//把GPIOA11设置为输出
    EDIS;
    GpioDataRegs.GPADAT.bit.GPIOA11=0; 	//GPIOA11=0;该端口为74HC595锁存信号
  }



//通过SPI向LED发送显示数据
void Write_LED (Uint16 LEDReg)
{
Uint16 LEDcode[30]={0xc000,0xf900,0xA400,0xB000,0x9900,0x9200,0x8200,0xF800,
                    0x8000,0x9000,0x8800,0x8300,0xc600,0xa100,0x8600,0x8e00,
                    0x8c00,0xbf00,0xa700,0xff00,0x4000,0x7900,0x2400,0x3000,
                    0x1900,0x1200,0x0200,0x7800,0x0000,0xf700};//共阳字形码0~f, P，－，L，"灭",0.~9.
	 		SpiaRegs.SPITXBUF =LEDcode[LEDReg]; //给数码管送数
    	 	 while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){}
    		SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF; //读出清标志
}


/*LED显示子程序*/
void LEDdisplay(Uint16* led)
{
	int i;
    GpioDataRegs.GPADAT.bit.GPIOA11=0; //给LACK信号一个低电平

	  for(i=7;i>=0;i--)
	Write_LED (led[i]);//向LED最高位写数据,显示LED8

     GpioDataRegs.GPADAT.bit.GPIOA11=1; //给LACK信号一个高电平为锁存74HC595

}
/*子程序结束*/


