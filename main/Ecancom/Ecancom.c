/******************************************************************************************
**描述:Ecan以中断方式实现双机通讯,功能B键用于启动发送,将相应的设定值发送对方
*;发送位:LED8、LED7、LED6，接收位：LED3、LED2、LED1,十六进制方式
*中断T4PINT扫描按键(10.24 ms)
******************************************************************************************/

#include"DSP281x_Device.h"

unsigned int LEDReg;
unsigned int LED8=0; //显示的发送位
unsigned int LED7=0;
unsigned int LED6=0;   
unsigned int LED3=0; //显示的接收位
unsigned int LED2=0;
unsigned int LED1=0;
unsigned int KeyReg1;
unsigned int KeyReg2;
unsigned long int i = 0;
void  IOinit(void);
int   KeyIn1(void);
void  KeyFunction1(unsigned int KeyReg1);
void  KeyFunction2(unsigned int KeyReg2);
void Write_LED (int LEDReg);//通过SPI向LED发送显示数据

Uint16 keyNum = 0x0000;  //按键次数
Uint16 RecieveChar;
Uint16 transmitChar;

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

interrupt void  T4pint_isr(void);//中断子程序的声明
interrupt void  Ecan0inta_isr(void);

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
	GpioMuxRegs.GPAMUX.bit.TDIRA_GPIOA11=0; //GPIOA11设置为一般I/O口
	GpioMuxRegs.GPADIR.bit.GPIOA11=1;		//把GPIOA11设置为输出 
	EDIS;
	GpioDataRegs.GPADAT.bit.GPIOA11=0; 	//GPIOA11=0;该端口为74HC595锁存信号				
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
    ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;       //  把发送，接收数据的长度定义为8位
    ECanaMboxes.MBOX0.MSGCTRL.bit.RTR = 0;       //  无远程帧请�  
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
   	InitSysCtrl();      //初始化系统控制寄存器, 时钟频率150M
	DINT;	        //关闭总中断，清除中断标志
	IER = 0x0000;   //关闭外围中断
	IFR = 0x0000;	//清中断标志
	spi_intial();   //SPI初始化子程序   
	IOinit();	    // I/O初始化子程
   	CAN_INIT();	//初始化SCl
	InitPieCtrl(); //初始化PIE控制寄存器 
	InitPieVectTable(); //初始化PIE中断向量表
	LEDdisplay(0,0,0,19,19,0,0,0);

//T4：按键 ，初始化按键检测中断
	EvbRegs.T4PR=12000;	//T4定时器周期
	EvbRegs.T4CON.all=0x1744;	//x／128，增计数10．24 ms
	EvbRegs.T4CNT=0x0000;//T4计数器清

	EvbRegs.EVBIMRB.bit.T4PINT=1;	//使能T4PINT中断
	EvbRegs.EVBIFRB.bit.T4PINT=1;	//先清标志
	InitXintf();	//初始化XINTF
//####中断使能
	EALLOW;
	PieVectTable.T4PINT=&T4pint_isr;//PIE中断矢量表	
	PieVectTable.ECAN0INTA=&Ecan0inta_isr;
	EDIS;
	PieCtrlRegs.PIEIER5.bit.INTx1=1;	//使能T4PINT	
	PieCtrlRegs.PIEIER9.bit.INTx5=1;	//使能ECAN0lNT

	IER |=( M_INT5 |M_INT9 );//使能INT5，INT9

	EINT;	//开放全局中断
	ERTM;	//使能全局实时中断

//主循环
   for(;;){;}
}

interrupt void   T4pint_isr(void)
	{	
 	    if (KeyIn1() == 1)     // 调用查键K1~K8子程序
 	    {
 	    keyNum=keyNum+1; 	 
	    KeyFunction1(KeyReg1);
	
		if(keyNum>3)
		{
		keyNum=keyNum-3;
		LED8=LEDReg;
		}
 	    if(keyNum==1)
		{
		LED8=LEDReg;
		}
		 if(keyNum==2)
		{
		LED7=LEDReg;
		}
		 if(keyNum==3)
		{
		LED6=LEDReg;
		} 	
		transmitChar=LED8*256+LED7*16+LED6;//转换成十进制的值
 		LEDdisplay(LED8,LED7,LED6,19,19,LED3,LED2,LED1); //显示发送位-灭灭-接收位****

		EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;
		}

		if (KeyIn2() == 1)     // 调用查键K8~K16子程序
 	    {
 	    KeyFunction2(KeyReg2);	

	if(LEDReg==0x0B)		////判是否为功能B键 CAN发送
		{	 	
		ECanaMboxes.MBOX0.MDL.all = 0x55555502;//发送标志
		ECanaMboxes.MBOX0.MDH.all = transmitChar;
		ECanaRegs.CANTRS.all=0x0001; //发送0邮箱 
		while(ECanaRegs.CANTA.all!=0x0001) {};
		ECanaRegs.CANTA.all=0xFFFF;		//写1清.	
	 
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return;
		}
 	   	keyNum=keyNum+1;		
		if(keyNum>3)
		{
		keyNum=keyNum-3;
		LED8=LEDReg;
		}
 	    if(keyNum==1)
		{
		LED8=LEDReg;
		}
		 if(keyNum==2)
		{
		LED7=LEDReg;
		}
		 if(keyNum==3)
		{
		LED6=LEDReg;
		}
 		
		transmitChar=LED8*256+LED7*16+LED6;//转换成十进制的值
 		LEDdisplay(LED8,LED7,LED6,19,19,LED3,LED2,LED1); //显示发送位-灭灭-接收位
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
		return; 		
	}
	EvbRegs.EVBIFRB.bit.T4PINT=1;	//中断标志清
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //允许下次中断
	return;
}
interrupt void Ecan0inta_isr(void)	//接收信息悬挂中断
{
	if(ECanaRegs.CANRMP.all==0x00010000)
	 {
	ECanaRegs.CANRMP.all=0xFFFF0000;
 	mailbox_read(16);
	RecieveChar=TestMbox2;
	LED3=RecieveChar/256;
	LED2=(RecieveChar%256)/16;
	LED1=RecieveChar%16;
	LEDdisplay(LED8,LED7,LED6,19,19,LED3,LED2,LED1);	
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //允许下次中断	
	return;
	}
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //允许下次中断
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
    	for (i=0; i<40000; i++){}        //延时消抖
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
    //判s8~s16是否按下
    if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
    {
    	for (i=0; i<40000; i++){}        //延时消抖
        if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
        {
	   KeyReg2=GpioDataRegs.GPBDAT.all|0x00ff ;
    		while ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff) //判是否送开
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
