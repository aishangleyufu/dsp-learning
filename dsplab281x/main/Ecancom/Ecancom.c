/******************************************************************************************
**����:Ecan���жϷ�ʽʵ��˫��ͨѶ,����B��������������,����Ӧ���趨ֵ���ͶԷ�
*;����λ:LED8��LED7��LED6������λ��LED3��LED2��LED1,ʮ�����Ʒ�ʽ
*�ж�T4PINTɨ�谴��(10.24 ms)
******************************************************************************************/

#include"DSP281x_Device.h"

unsigned int LEDReg;
unsigned int LED8=0; //��ʾ�ķ���λ
unsigned int LED7=0;
unsigned int LED6=0;   
unsigned int LED3=0; //��ʾ�Ľ���λ
unsigned int LED2=0;
unsigned int LED1=0;
unsigned int KeyReg1;
unsigned int KeyReg2;
unsigned long int i = 0;
void  IOinit(void);
int   KeyIn1(void);
void  KeyFunction1(unsigned int KeyReg1);
void  KeyFunction2(unsigned int KeyReg2);
void Write_LED (int LEDReg);//ͨ��SPI��LED������ʾ����

Uint16 keyNum = 0x0000;  //��������
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

interrupt void  T4pint_isr(void);//�ж��ӳ��������
interrupt void  Ecan0inta_isr(void);

void IOinit()
{
 	EALLOW;  
 	//��GPIOA����Ϊ�����
 	GpioMuxRegs.GPAMUX.all = 0xffff;

    //��GPIOE0~GPIOE2����Ϊһ��I/O�����,��138����  
 	GpioMuxRegs.GPEMUX.all = GpioMuxRegs.GPEMUX.all&0xfff8; 
	GpioMuxRegs.GPEDIR.all = GpioMuxRegs.GPEDIR.all|0x0007;
    //��GPIOB8~GPIOB15����Ϊһ��I/O��,D0~D7
	GpioMuxRegs.GPBMUX.all = GpioMuxRegs.GPBMUX.all&0x00ff;
	GpioMuxRegs.GPAMUX.bit.TDIRA_GPIOA11=0; //GPIOA11����Ϊһ��I/O��
	GpioMuxRegs.GPADIR.bit.GPIOA11=1;		//��GPIOA11����Ϊ��� 
	EDIS;
	GpioDataRegs.GPADAT.bit.GPIOA11=0; 	//GPIOA11=0;�ö˿�Ϊ74HC595�����ź�				
}

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

void  CAN_INIT()
{
	struct ECAN_REGS ECanaShadow;  
	EALLOW;
  	GpioMuxRegs.GPFMUX.bit.CANTXA_GPIOF6 = 1; //  ����GPIOF6ΪCANTX 
  	GpioMuxRegs.GPFMUX.bit.CANRXA_GPIOF7 = 1; //  ����GPIOF7ΪCANRX    	
	EDIS;
/*eCAN ���ƼĴ�����Ҫ32λ���ʡ��������һ������λ����д���������������ܻ�ʹ�����16λ���ʡ����������һ�ֽ��������������Ӱ�ӼĴ�����ʹ����32λ���ʡ� �������Ĵ�������һ��Ӱ�ӼĴ����� ������ʽ���32λ�ġ���32λд�����ı���Ҫ�ĵ�λ��Ȼ��Ѹ�ֵ������eCAN�Ĵ���*/
    EALLOW; 
   	ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all; //  ��CANTIOC����Ӱ�ӼĴ���
   	ECanaShadow.CANTIOC.bit.TXFUNC = 1;              //  �ⲿ����I/Oʹ�ܱ�־λ��
//  TXFUNC��1  CANTX���ű�����CAN���͹��ܡ�
//  TXFUNC��0  CANTX���ű���Ϊͨ��I/O���ű�ʹ�� 
	ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all; //  �����úõļĴ���ֵ��д
    ECanaShadow.CANRIOC.all = ECanaRegs.CANRIOC.all;  //  ��CANRIOC��Ӱ�ӼĴ���
	ECanaShadow.CANRIOC.bit.RXFUNC = 1;              //  �ⲿ����I/Oʹ�ܱ�־λ��
//  RXFUNC��1  CANRX���ű�����CAN���չ��ܡ�
//  RXFUNC��0  CANRX���ű���Ϊͨ��I/O���ű�ʹ�á�
    ECanaRegs.CANRIOC.all = ECanaShadow.CANRIOC.all;  //  �����úõļĴ���ֵ��д
    EDIS;
//  ����������IDֵ֮ǰ��CANME��Ӧ��λ���븴λ��
//  ���CANME�Ĵ����ж�Ӧ��λ����λ����IDд�������Ч��
	ECanaRegs.CANME.all = 0;                 //  ��λ���е�����
   	ECanaMboxes.MBOX0.MSGID.all = 0x15100000;  //  ���÷�������0��ID����չ��ʶ��29λ  
    ECanaMboxes.MBOX16.MSGID.all = 0x15200000; //  ȷ����������16ID 
    //  ����0��15 ����Ϊ�������� �� ������16��31 ��Ϊ��������
    ECanaRegs.CANMD.all = 0xFFFF0000; 
    ECanaRegs.CANME.all = 0xFFFFFFFF;         //  CANģ��ʹ�ܶ�Ӧ�����䣬
        
    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;       //  �ѷ��ͣ��������ݵĳ��ȶ���Ϊ8λ
    ECanaMboxes.MBOX0.MSGCTRL.bit.RTR = 0;       //  ��Զ��֡���  
//  ��ΪRTRλ�ڸ�λ��״̬����������ڳ�����г�ʼ����ʱ�����Ը�λ��ֵ��
    ECanaMboxes.MBOX1.MSGCTRL.bit.RTR = 0;
//  �Ѵ����͵�����д�뷢������ 
 	ECanaMboxes.MBOX0.MDL.all = 0x55555555;
	ECanaMboxes.MBOX0.MDH.all = 0x55555501;	 
	EALLOW;
//  �����ж����μĴ������ϵ�����е��ж�����λ��������ֹͣ�ж�ʹ�ܡ�
//  ��Щλ������������κ������жϡ�
	ECanaRegs.CANGIM.all = 0x0001;     //�ж�0ʹ��
	ECanaRegs.CANMIM.all = 0xFFFFFFFF;  
//  CANMIM .BIT.X��1  �����жϱ�ʹ�ܣ�X��1��31��
//  CANMIM .BIT.X��0  �����жϱ���ֹ��X��1��31��
	ECanaShadow.CANMC.all = ECanaRegs.CANMC.all; //  ��CANMC����Ӱ�ӼĴ���
	ECanaShadow.CANMC.bit.CCR = 1;               //  �ı���������λ
	ECanaShadow.CANMC.bit.SCB = 1;  			//eCANģʽ,��������ʹ��
	ECanaRegs.CANMC.all = ECanaShadow.CANMC.all; //  �����úõļĴ���ֵ��д
  	EDIS;
/*CPUҪ������üĴ���CANBTC��SCC�Ľ������μĴ���(CANGAM��LAM[0]��LAM[3])����д�������Ը�λ��λ��CPU����ȴ���ֱ��CANES�Ĵ�����CCE��־λ������CANBTC�Ĵ���֮ǰΪ1 */
do 
    {
      ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    } while(ECanaShadow.CANES.bit.CCE != 1 );  //  ��CCE��1ʱ���Զ�CANBTC���в�����
    //  ���ò�����
    EALLOW;
    ECanaShadow.CANBTC.all = ECanaRegs.CANBTC.all; //  ��CANBTC����Ӱ�ӼĴ���
    ECanaShadow.CANBTC.bit.BRPREG = 149;   //  (BRP+1)��150�� ��Сʱ�䵥λTQ��1us
    ECanaShadow.CANBTC.bit.TSEG2REG = 2;   //  λ��ʱbit-time��(TSEG1+1)+(TSEG1+1)+1
    ECanaShadow.CANBTC.bit.TSEG1REG = 3;   //  bit-time��8us�� ���Բ�����Ϊ125Kpbs       
    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;  //  �����úõļĴ���ֵ��д
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;   //  ��CANMC����Ӱ�ӼĴ���
    ECanaShadow.CANMC.bit.CCR = 0 ;       //  ����CCR��0�� CPU��������ģʽ
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;   //  �����úõļĴ���ֵ��д
    EDIS;
    do
    {
      ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    } while(ECanaShadow.CANES.bit.CCE != 0 );  //  �ȴ� CCE λ������
//  ����eCANΪ�Բ���ģʽ��ʹ��eCAN����ǿ����
    EALLOW;
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.STM = 0;        //  ����CAN Ϊ����ģʽ
  	// CANMC.bit.STM��0������ģʽ��CANMC.bit.STM��1���Բ���ģʽ
    ECanaShadow.CANMC.bit.SCB = 1;        // ѡ��HECC����ģʽ
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
    EDIS;
}

void mailbox_read(int16 MBXnbr)
{
  	volatile struct MBOX *Mailbox;
	Mailbox = &ECanaMboxes.MBOX0 + MBXnbr;
	TestMbox1 = Mailbox->MDL.all;    	// ������ǰ�������ݵ�4�ֽ�
	TestMbox2 = Mailbox->MDH.all;       //  ������ǰ�������ݸ�4�ֽ�
	TestMbox3 = Mailbox->MSGID.all;     //  ������ǰ����ID
} 

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
    asm (" NOP ");
   	InitSysCtrl();      //��ʼ��ϵͳ���ƼĴ���, ʱ��Ƶ��150M
	DINT;	        //�ر����жϣ�����жϱ�־
	IER = 0x0000;   //�ر���Χ�ж�
	IFR = 0x0000;	//���жϱ�־
	spi_intial();   //SPI��ʼ���ӳ���   
	IOinit();	    // I/O��ʼ���ӳ�
   	CAN_INIT();	//��ʼ��SCl
	InitPieCtrl(); //��ʼ��PIE���ƼĴ��� 
	InitPieVectTable(); //��ʼ��PIE�ж�������
	LEDdisplay(0,0,0,19,19,0,0,0);

//T4������ ����ʼ����������ж�
	EvbRegs.T4PR=12000;	//T4��ʱ������
	EvbRegs.T4CON.all=0x1744;	//x��128��������10��24 ms
	EvbRegs.T4CNT=0x0000;//T4��������

	EvbRegs.EVBIMRB.bit.T4PINT=1;	//ʹ��T4PINT�ж�
	EvbRegs.EVBIFRB.bit.T4PINT=1;	//�����־
	InitXintf();	//��ʼ��XINTF
//####�ж�ʹ��
	EALLOW;
	PieVectTable.T4PINT=&T4pint_isr;//PIE�ж�ʸ����	
	PieVectTable.ECAN0INTA=&Ecan0inta_isr;
	EDIS;
	PieCtrlRegs.PIEIER5.bit.INTx1=1;	//ʹ��T4PINT	
	PieCtrlRegs.PIEIER9.bit.INTx5=1;	//ʹ��ECAN0lNT

	IER |=( M_INT5 |M_INT9 );//ʹ��INT5��INT9

	EINT;	//����ȫ���ж�
	ERTM;	//ʹ��ȫ��ʵʱ�ж�

//��ѭ��
   for(;;){;}
}

interrupt void   T4pint_isr(void)
	{	
 	    if (KeyIn1() == 1)     // ���ò��K1~K8�ӳ���
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
		transmitChar=LED8*256+LED7*16+LED6;//ת����ʮ���Ƶ�ֵ
 		LEDdisplay(LED8,LED7,LED6,19,19,LED3,LED2,LED1); //��ʾ����λ-����-����λ****

		EvbRegs.EVBIFRB.bit.T4PINT=1;	//�жϱ�־��
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //�����´��ж�
		return;
		}

		if (KeyIn2() == 1)     // ���ò��K8~K16�ӳ���
 	    {
 	    KeyFunction2(KeyReg2);	

	if(LEDReg==0x0B)		////���Ƿ�Ϊ����B�� CAN����
		{	 	
		ECanaMboxes.MBOX0.MDL.all = 0x55555502;//���ͱ�־
		ECanaMboxes.MBOX0.MDH.all = transmitChar;
		ECanaRegs.CANTRS.all=0x0001; //����0���� 
		while(ECanaRegs.CANTA.all!=0x0001) {};
		ECanaRegs.CANTA.all=0xFFFF;		//д1��.	
	 
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//�жϱ�־��
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //�����´��ж�
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
 		
		transmitChar=LED8*256+LED7*16+LED6;//ת����ʮ���Ƶ�ֵ
 		LEDdisplay(LED8,LED7,LED6,19,19,LED3,LED2,LED1); //��ʾ����λ-����-����λ
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//�жϱ�־��
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //�����´��ж�
		return; 		
	}
	EvbRegs.EVBIFRB.bit.T4PINT=1;	//�жϱ�־��
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //�����´��ж�
	return;
}
interrupt void Ecan0inta_isr(void)	//������Ϣ�����ж�
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
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //�����´��ж�	
	return;
	}
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //�����´��ж�
	return;
}	

int KeyIn1(void)
{
 	 EALLOW;  
    //��GPIOB8~GPIOB15����Ϊ����,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all&0x00ff;
     EDIS;    
    GpioDataRegs.GPEDAT.all = 0xfff8;     //ѡͨKEY��8λ
    for (i=0; i<100; i++){}               //��ʱ
    //��K1~K8�Ƿ���
    if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
    {
    	for (i=0; i<40000; i++){}        //��ʱ����
        if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
        {
	   KeyReg1=GpioDataRegs.GPBDAT.all|0x00ff ;
    		while ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff) //���Ƿ��ɿ�
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
    //��GPIOB8~GPIOB15����Ϊ����,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all&0x00ff;
     EDIS;    
    GpioDataRegs.GPEDAT.all = 0xfff9;     //ѡͨKEY��8λ
    for (i=0; i<100; i++){}               //��ʱ
    //��s8~s16�Ƿ���
    if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
    {
    	for (i=0; i<40000; i++){}        //��ʱ����
        if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
        {
	   KeyReg2=GpioDataRegs.GPBDAT.all|0x00ff ;
    		while ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff) //���Ƿ��Ϳ�
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
//=========================================================================================
// No more.
//=========================================================================================
