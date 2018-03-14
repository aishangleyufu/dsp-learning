/*****************************************************************
** ��������: ������ʾ���򣬶԰�����¼�������Զ�������ʾ��16��Led��
*****************************************************************/
#include "DSP281x_Device.h"
unsigned int LEDReg;
unsigned int KeyReg1;
unsigned int KeyReg2;
unsigned long int i = 0;

//������Ӧֵ
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

//����������0~f, P������L��"��",0.~9.
unsigned  int LEDCode[30]={0xc000,0xf900,0xA400,0xB000,0x9900,0x9200,0x8200,0xF800,
                    0x8000,0x9000,0x8800,0x8300,0xc600,0xa100,0x8600,0x8e00,
                    0x8c00,0xbf00,0xa700,0xff00,0x4000,0x7900,0x2400,0x3000,
                    0x1900,0x1200,0x0200,0x7800,0x0000,0x1000};

 
//SPI��ʼ���ӳ���
void spi_intial()
 {
 	SpiaRegs.SPICCR.all =0x0047;   // ʹSPI���ڸ�λ��ʽ, �½���, ��λ����  
	SpiaRegs.SPICTL.all =0x0006;   // ����ģʽ, һ��ʱ��ģʽ,ʹ��talk, �ر�SPI�ж�.
	SpiaRegs.SPIBRR =0x007F;       //���ò�����
	SpiaRegs.SPICCR.all =SpiaRegs.SPICCR.all|0x0080;  // �˳���λ״̬	 
	EALLOW;	
    GpioMuxRegs.GPFMUX.all=0x000F;	// ����ͨ������ΪSPI����	 	
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

//��ɨ���ӳ���K1~K8
int Keyscan1(void)
{
 	 EALLOW;  
    //��GPIOB8~GPIOB15����Ϊ����,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all&0x00ff;
     EDIS;    
    GpioDataRegs.GPEDAT.all = 0xfff8;    //ѡͨKEY��8λ
    for (i=0; i<100; i++){}             //��ʱ
    //��K1~K8�Ƿ���
    if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
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
//������¼��������ʾ��16��LED��
void LedOut(Uint16 led)
{
 	EALLOW;  
    //��GPIOB8~GPIOB15����Ϊ���,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all|0xff00;
	EDIS;	
	GpioDataRegs.GPEDAT.all = 0xfffb;    //LEDB��0
	GpioDataRegs.GPBDAT.all = ~led;
	for (i=0; i<100; i++){}              //��ʱ
    GpioDataRegs.GPEDAT.all = 0xffff;    //�����8λ	
	GpioDataRegs.GPEDAT.all = 0xfffa;    //LEDA��0
	GpioDataRegs.GPBDAT.all = ~(led<<8);
	for (i=0; i<100; i++){}
    GpioDataRegs.GPEDAT.all = 0xffff;    //�����8λ			
}

//��ɢת�ӳ���K1~K8
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

//��ɢת�ӳ���K9~K16
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
//��ʾ�ӳ���
void display (LEDReg)
{
	GpioDataRegs.GPADAT.bit.GPIOA11=0; //��LACK�ź�һ���͵�ƽ	
	 		SpiaRegs.SPITXBUF =LEDCode[LEDReg]; //�����������
    	 	 while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){} 		
    		SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;	
     
     		GpioDataRegs.GPADAT.bit.GPIOA11=1; //��LACK�ź�һ���ߵ�ƽΪ����74HC595
     	  	for(i=0;i<10;i++){}	//��ʱ
}


void main(void)
{
    Uint16 keyNum = 0x0000;  //����������ʼ��
    InitSysCtrl();  // ϵͳ��ʼ������
 	DINT;           //�ر����ж�
	spi_intial();   //SPI��ʼ���ӳ���
	gpio_init();    // I/O��ʼ���ӳ���
	IER = 0x0000;   // �ر���Χ�ж�
	IFR = 0x0000;   // ���жϱ�־		
	LedOut(keyNum); // LEDָʾ�Ƴ�ʼ����
	for(i=0;i<8;i++)// 8���������0	
		{
			SpiaRegs.SPITXBUF =LEDCode[0]; //�����������
    		while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){} 		
    		SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;	
     	}
     		GpioDataRegs.GPADAT.bit.GPIOA11=1; //��LACK�ź�һ���ߵ�ƽΪ����74HC595
     		for(i=0;i<10;i++){}	
 	
	while (1)
	{
 	    if (Keyscan1() == 1)     // ���ü�ɨ��K1~K8�ӳ���
 	    {
 	    	keyNum=keyNum+1;// ��������	 
	    	KeyFunction1(KeyReg1);
	        display (LEDReg);//��ʾ��ֵ			
 	     	 LedOut(keyNum);//��ʾ��������
 	    }
		if (Keyscan2() == 1)     // ���ü�ɨ��K8~K16�ӳ���
 	    {
 	   
 	   		keyNum=keyNum+1; 	  
	    	KeyFunction2(KeyReg2);
			display (LEDReg);//��ʾ��ֵ		
 	     	LedOut(keyNum);//��ʾ��������
 	     }
    }
}
