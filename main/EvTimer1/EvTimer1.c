#include "DSP281x_Device.h"
#include "DSP281x_Examples.h"   // DSP281x Examples ͷ�ļ�
#include "math.h"


interrupt void cpu_timer0_isr(void);
//--------------------------------------------------------


unsigned int LEDReg;   //��ֵת����ʵ�ʵļ�ֵ
unsigned int KeyReg1;  //���̵�8λ��δת��ֵ��
unsigned int KeyReg2;  //���̸�8λ��δת��ֵ�� 


//--------------------------------------------------------

Uint16 LedDirection = 0;
Uint16 RunToMid = 0;
Uint16 LedRun = 0;
Uint16 LedAB = 0x0001;
Uint16 LedC =0x8001;

//������Ӧֵ
#define	  K1		0xfeff   //��keyreg1��2���Ƚϣ��߰�λ��ֵ��ӦGPIOB8~15
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

//����������0~f, P������L��"��",0.~9.
unsigned  int LEDCode[30]={0xc000,0xf900,0xA400,0xB000,0x9900,0x9200,0x8200,0xF800,
                    0x8000,0x9000,0x8800,0x8300,0xc600,0xa100,0x8600,0x8e00,
                    0x8c00,0xbf00,0xa700,0xff00,0x4000,0x7900,0x2400,0x3000,
                    0x1900,0x1200,0x0200,0x7800,0x0000,0x1000}; 

//��ɨ���ӳ���K1~K8
int Keyscan1(void)
{
	Uint16 i;
 	 EALLOW;  
    //��GPIOB8~GPIOB15����Ϊ����,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all&0x00ff;
     EDIS;    
    GpioDataRegs.GPEDAT.all = 0xfff8;    //ѡͨKEY��8λ
    for (i=0; i<100; i++){}             //��ʱ
    //��K1~K8�Ƿ���
    if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)  //B8~15��ĳһλ��0��˵���а�������
    {
    	for (i=0; i<30000; i++){}        //��ʱ����
        if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
        {
	   KeyReg1=GpioDataRegs.GPBDAT.all ;//����ֵ
    		while ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff) //��K1~K8�Ƿ��ɿ�
    		{
    			GpioDataRegs.GPDDAT.bit.GPIOD1 = !GpioDataRegs.GPDDAT.bit.GPIOD1; 
    			for (i=0; i<1000; i++){}     
    		}
    		return (1);			      
    	}
    }
    return (0);
}

//��ɨ���ӳ���K9~K16
int Keyscan2(void)
{
	Uint16 i;
 	 EALLOW;  
    //��GPIOB8~GPIOB15����Ϊ����,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all&0x00ff;
     EDIS;    
    GpioDataRegs.GPEDAT.all = 0xfff9;     //ѡͨKEY��8λ
    for (i=0; i<100; i++){}               //��ʱ
    //��K8~K16�Ƿ���
    if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
    {
    	for (i=0; i<30000; i++){}        //��ʱ����
        if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
        {
	   		KeyReg2=GpioDataRegs.GPBDAT.all ;//����ֵ
    		while ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff) //��K8~K16�Ƿ��ɿ�
    		{
    			GpioDataRegs.GPDDAT.bit.GPIOD1 = !GpioDataRegs.GPDDAT.bit.GPIOD1;
    			for (i=0; i<1000; i++){}     
    		}
    		return (1);			      
    	}
    }
    return (0);
}


//��ɢת�ӳ���K1~K8
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

//��ɢת�ӳ���K9~K16
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
    //��GPIOB8~GPIOB15����Ϊ���,D0~D7     
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all|0xff00;
	EDIS;	
	GpioDataRegs.GPEDAT.all = 0xfffb;    //LEDBѡͨ
	GpioDataRegs.GPBDAT.all = ~led;		//��ʾ��8λ
	for (i=0; i<100; i++){}              //��ʱ
    GpioDataRegs.GPEDAT.all = 0xffff;    //�����8λ  	
	GpioDataRegs.GPEDAT.all = 0xfffa;    //LEDAѡͨ
	GpioDataRegs.GPBDAT.all = ~(led<<8);//��ʾ��8λ
	for (i=0; i<100; i++){}
    GpioDataRegs.GPEDAT.all = 0xffff;    //�����8λ			
}

void spi_intial()
 {
 	SpiaRegs.SPICCR.all =0x0047;   // ʹSPI���ڸ�λ��ʽ, �½���, ��λ����  
	SpiaRegs.SPICTL.all =0x0006;   // ����ģʽ, һ��ʱ��ģʽ,ʹ��talk, �ر�SPI�ж�.
	SpiaRegs.SPIBRR =0x007F;       //���ò�����
	SpiaRegs.SPICCR.all =SpiaRegs.SPICCR.all|0x0080;  // �˳���λ״̬	 
	EALLOW;	
    GpioMuxRegs.GPFMUX.all|=0x000F;	// ����ͨ������ΪSPI����	 	
    EDIS;
  }

//IO��ʼ���ӳ���
void gpio_init()
{
	EALLOW;
	GpioMuxRegs.GPAMUX.bit.TDIRA_GPIOA11=0; //GPIOA11����Ϊһ��I/O��
	GpioMuxRegs.GPADIR.bit.GPIOA11=1;	//��GPIOA11����Ϊ���
    //��GPIOE0~GPIOE2����Ϊһ��I/O�����,��138����
 	GpioMuxRegs.GPEMUX.all = GpioMuxRegs.GPEMUX.all&0xfff8;
	GpioMuxRegs.GPEDIR.all = GpioMuxRegs.GPEDIR.all|0x0007;
    //��GPIOB8~GPIOB15����Ϊһ��I/O��,D0~D7
	GpioMuxRegs.GPBMUX.all = GpioMuxRegs.GPBMUX.all&0x00ff;
    EDIS;
    GpioDataRegs.GPADAT.bit.GPIOA11=0; 	//GPIOA11=0;�ö˿�Ϊ74HC595�����ź�
}


void IOinit() //I/O�ڳ�ʼ��
{
 	EALLOW;  
 	 //��GPIOB8~GPIOB15����Ϊһ��I/O��,D0~D7
	GpioMuxRegs.GPBMUX.all = GpioMuxRegs.GPBMUX.all&0x00ff;
	 //��GPIOB8~GPIOB15����Ϊ���,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all|0xff00;
    //��GPIOE0~GPIOE2����Ϊһ��I/O�����,��138���� 	  
 	GpioMuxRegs.GPEMUX.all = GpioMuxRegs.GPEMUX.all&0xfff8; 
	GpioMuxRegs.GPEDIR.all = GpioMuxRegs.GPEDIR.all|0x0007;
  	EDIS;			
}

//ͨ��SPI��LED������ʾ����
void Write_LED (int LEDReg)
{
Uint16 LEDcode[30]={0xc000,0xf900,0xA400,0xB000,0x9900,0x9200,0x8200,0xF800,
                    0x8000,0x9000,0x8800,0x8300,0xc600,0xa100,0x8600,0x8e00,
                    0x8c00,0xbf00,0xa700,0xff00,0x4000,0x7900,0x2400,0x3000,
                    0x1900,0x1200,0x0200,0x7800,0x0000,0x1000};//����������0~f, P������L��"��",0.~9.	
	 		SpiaRegs.SPITXBUF =LEDcode[LEDReg]; //�����������
    	 	 while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){} 		
    		SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF; //�������־
}

/*LED��ʾ�ӳ���*/
void LEDdisplay(int LED8,int LED7,int LED6,int LED5,int LED4,int LED3,int LED2,int LED1)
{
    GpioDataRegs.GPADAT.bit.GPIOA11=0; //��LACK�ź�һ���͵�ƽ

	Write_LED (LED8);//��LED���λд����,��ʾLED8
	Write_LED (LED7);
	Write_LED (LED6);
	Write_LED (LED5);
	Write_LED (LED4);
	Write_LED (LED3);
	Write_LED (LED2);
	Write_LED (LED1);//��LED���λд����,��ʾLED1

     GpioDataRegs.GPADAT.bit.GPIOA11=1; //��LACK�ź�һ���ߵ�ƽΪ����74HC595
}


void main(void)
{

// Step 1. //ϵͳ��ʼ��(��DSP281x_SysCtrl.c��)
// PLL, WatchDog, enable Peripheral Clocks 
   InitSysCtrl();//ϵͳ��ʼ��

// Step 2. ��ʼ�� GPIO:
   IOinit();
   
// Step 3. ���жϼ���ʼ��PIEʸ����:
   DINT;	/*��CPU�ж�*/

   InitPieCtrl();//��ʼ��PIE���ƼĴ��� DSP281x_PieCtrl.c   
   IER = 0x0000;//�ر���Χ�ж�
   IFR = 0x0000;//���жϱ�־
 
   InitPieVectTable();//��ʼ��PIEʸ����DSP281x_PieVect.c
  
   EALLOW;  //дEALLOW�����Ĵ���
   PieVectTable.TINT0 = &cpu_timer0_isr;//PIE�ж�ʸ����
   EDIS;    

   InitCpuTimers();   // For this example, only initialize the Cpu Timers
//��ʱ����һ��������£������ڼĴ�����PRDH:PRD�е�ֵװ��32λ�����Ĵ���TIMH:TIM��
//Ȼ������Ĵ�����C28X ��SYSCLKOUT���ʵݼ���������������0ʱ���ͻ����һ����ʱ��
//�ж�����źţ�һ���ж����壩��
// 150MHz CPU Freq, 0.5 second Period (in uSeconds)
//�����жϵļ��=�����ڼĴ����е�ֵ��/SYSCLKOUT/��Ƶϵ��
//���ڼĴ����е�ֵ=(Freq*Period)
   ConfigCpuTimer(&CpuTimer0, 150, 500000);
   StartCpuTimer0();

   IER |= M_INT1;	//ʹ��CPU INT1 

   PieCtrlRegs.PIEIER1.bit.INTx7 = 1;//CPU-Timer0ʹ��λ��PIE��1���7��������ʹ��

   EINT;   //����ȫ���ж� 

	for(;;)
	{
		if (Keyscan2() == 1)     // ���ü�ɨ��K8~K16�ӳ���
 	    {	  
	    	KeyFunction2(KeyReg2);
			switch(LEDReg)
			{
				case 0x0A:
					LedDirection = 1;  //A��������ƴ��ҵ���
					RunToMid = 0;
					break;

				case 0x0B:
					LedDirection = 0;  //B��������ƴ�����
					RunToMid = 0;
					break;

				case 0x0C:
					RunToMid = 1;  //C��������ƴ��������м���
					break;
			}
 	    }	
		
		if (LedRun == 1)    //����Ƴ���
		{
			LedRun = 0;

			if (RunToMid == 1)  //���������м�
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
			else if(LedDirection == 1)  //��������
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
			else if(LedDirection == 0)  //��������
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
     PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;// ���ж�Ӧ���źţ�׼��������һ���ж�
}

void Delay(Uint16  data)
{
	Uint16	i;
	for (i=0;i<data;i++) { ; }	
}


//===========================================================================
// No more.
//===========================================================================




   