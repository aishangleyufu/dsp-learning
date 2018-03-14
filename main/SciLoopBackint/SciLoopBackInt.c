//******************************************************************************************
// �ļ�: Sci_LoopBack_Int.c
// ����: ͨ���жϽ���DSP281x����SCI FIFO���ֻ��Ͳ���,��SCIͨ�Ŵ����Զ�������ʽ������ʾ
// ����:����Ҫ������Ӳ�����á�//
//******************************************************************************************
#include "DSP281x_Device.h"     		// DSP281x ͷ�ļ������ļ���
#include "DSP281x_Examples.h"   		// DSP281x ʾ�������ļ���

	//���ļ������ĺ���ԭ��������
interrupt void sciaTxFifoIsr(void);		// SCIA�����Ƚ��ȳ��жϺ�����
interrupt void sciaRxFifoIsr(void);		// SCIA�����Ƚ��ȳ��жϺ�����
interrupt void scibTxFifoIsr(void);		// SCIB�����Ƚ��ȳ��жϺ�����
interrupt void scibRxFifoIsr(void);		// SCIB�����Ƚ��ȳ��жϺ�����
void scia_fifo_init(void);				// SCIA�Ƚ��ȳ���ʼ��������
void scib_fifo_init(void);				// SCIB�Ƚ��ȳ���ʼ��������
void error(void);						// ͨ�ų���������
void delay(Uint16 dly);
void LedOut(Uint16 led);	// ��SCIͨ�Ŵ����Զ�������ʽ������ʾ
void gpio_init();	       // ��ʼ��GPIO

void WriteXram(Uint16 *StartAddr, Uint16 *EndAddr);

	// ȫ�ֱ���
Uint16 count=0;							// ����������¼ѭ����ѭ���Ĵ�����

Uint16 sdataA[8];    					// SCI-A��������
Uint16 sdataB[8];    					// SCI-B��������
Uint16 rdataA[8];    					// SCI-A��������
Uint16 rdataB[8];    					// SCI-B��������
Uint16 rdata_pointA; 					// ���ڼ����յ�����
Uint16 rdata_pointB;

Uint16 *Xaddr=(Uint16 *)0x13f000; 	// ָ��������塣����ַ0x13f000Ϊ�������ݴ�����׵�ַ
Uint16 Xaddr_track=0;  				// ����ַ����ָ��

void main(void)
{ 
   	Uint16 i;
	// ϵͳ���Ƴ�ʼ��: PLL, WatchDog, ʹ������ʱ��	
   	InitSysCtrl();					
	// ��ʼ��GPIO,����led��ʾ
 	gpio_init();
	// ��GP I/O ���ó�SCI-A �� SCI-B ����									
   	EALLOW;	  		// ��������ܱ����ļĴ���
	GpioMuxRegs.GPFMUX.all = 0x0030;
	GpioMuxRegs.GPGMUX.all = 0x0030;			 
	EDIS;			// ��ֹ�����ܱ����ļĴ�����
		//��������жϺͳ�ʼ��PIE������			
	DINT;			// ��CPU�ж�

   	InitPieCtrl();		// ��PIE���ƼĴ�����ʼ��Ϊ��Ĭ��״̬
   	IER = 0x0000;		// ��CPU�жϡ�	
   	IFR = 0x0000;		// ���CPU�жϱ�־λ��	
   	InitPieVectTable();	// ��ʼ��PIE������  				

   	EALLOW;		// ����д���ܱ����ļĴ���
   	PieVectTable.RXAINT = &sciaRxFifoIsr;  //��ʼ�������ж���չ(PIE)������
   				// RXAINTλ�ڵ�9���еĵ�1����TXAINTλ�ڵ�9���еĵ�2����
   				// RXBINTλ�ڵ�9���еĵ�3����TXBINTλ�ڵ�9���еĵ�4����
   	PieVectTable.TXAINT = &sciaTxFifoIsr;
   	PieVectTable.RXBINT = &scibRxFifoIsr;
   	PieVectTable.TXBINT = &scibTxFifoIsr;
   	EDIS;   	// ��ֹд���ܱ����ļĴ���
		
   	scia_fifo_init();  			// Init SCI-A ��ʼ��SCI-A
   	scib_fifo_init();  			// Init SCI-B ��ʼ��SCI-B
	
		//  �û����Դ��룬ʹ���ж�
		// �������ݳ�ʼ������ÿ�δ�������֮�󣬸����ݽ�����Ϊ��һ��Ҫ���͵����ݡ�
   	for(i = 0; i<8; i++)	{ sdataA[i] = i;	}  
   								// sdataA[0]=0��sdataA[1]=1��... sdataA[7]=7��
   	for(i = 0; i<8; i++)	{ sdataB[i] = 0xFF - i;	 }
								// sdataB[0]=0xff��sdataB[1]=0xfe��... sdataB[7]=0xf8��
   	rdata_pointA = sdataA[0];	// rdata_pointA��һ����̬������ָ��������A 
//  sdataA[]�еĵ�һ��Ԫ�ء�	
   	rdata_pointB = sdataB[0]; 	// rdata_pointB��һ����̬������ָ��������B 
   								// sdataB[]�еĵ�һ��Ԫ�ء�	���������������ڲ����Ͳ��ԡ�

		/********* ʹ�ܱ�ʾ��������ж� **********/
   	PieCtrlRegs.PIECRTL.bit.ENPIE = 1;  	// ʹ��PIE������   		
   	PieCtrlRegs.PIEIER9.bit.INTx1=1;    	// ʹ��PIE ��9�� INT1(RXAINT)�ж� 
   	PieCtrlRegs.PIEIER9.bit.INTx2=1;    	// ʹ��PIE ��9�� INT2(TXAINT)�ж�
   	PieCtrlRegs.PIEIER9.bit.INTx3=1;    	// ʹ��PIE ��9�� INT3(RXBINT)�ж�
   	PieCtrlRegs.PIEIER9.bit.INTx4=1;    	// ʹ��PIE ��9�� INT4(TXBINT)�ж�
   	IER |= M_INT9;	//IER = 0x100;			// ʹ�ܵ�9���ж�
   	EINT; 									// ���ſ������ж�  
      
		//  ���ж�,����ѭ��	
	for(;;){;}	
} 	
//*******************************************************************************************
// ������ sciaTxFifoIsr
// ���ܣ� SCI_A �����Ƚ��ȳ�(FIFO)�жϷ����ӳ���(ISR)
//*******************************************************************************************
void error(void)
{
	asm("     ESTOP0"); 	// ���Գ�����ֹ�� C�����в�����ָ��ĸ�ʽ: asm("  ESTOP0");
							// ��Ϊ ����ֹͣ0(ESTOP0)����һ��˫������ָ��֮��Ҫ�ո�
    for (;;);				// ���޿�ѭ����
}

//******************************************************************************************
// ������ sciaTxFifoIsr
// ���ܣ� SCI_A �����Ƚ��ȳ�(FIFO)�жϷ����ӳ���(ISR)
//******************************************************************************************
interrupt void sciaTxFifoIsr(void)
{   
    Uint16 i;
    for(i=0; i< 8; i++)	{ SciaRegs.SCITXBUF=sdataA[i]; }
 	      // ����������A��i������sdataA[i]���뷢�ͻ�����SCITXBUF������8������
    for(i=0; i< 8; i++)                 	// ��һ�����ڣ���������ȡ������
    {
 	   sdataA[i] = (sdataA[i]+1) & 0x00FF; 
 	   		// ������2��sdataA[i]����һ��sdataA[i]�Ǵ��ȰsdataA[]������ȡ
 	   		// ��i��Ԫ�أ��ڼ�1�����θ�8λ�����¸�ֵ����i��Ԫ�ء�
 	   		// ������ǰһ�������8��Ԫ��Ϊ:		2,3,4,5,6,7,8,9
 	   		// ���һ�������8��Ԫ��Ϊ:		3,4,5,6,7,8,9,10...
	}	
	SciaRegs.SCIFFTX.bit.TXINTCLR=1;		// SCIFFTX[6]ΪTXINTCLRλ����λ��1
											// ���SCI�жϱ�־λTXFFIN
	PieCtrlRegs.PIEACK.all |= 0x100;    	// ����PIEӦ����ʾ��ӦINT9�ж�
}

//******************************************************************************************
// ������ sciaRxFifoIsr()
// ���ܣ� SCI_A �����Ƚ��ȳ�(FIFO)�жϷ����ӳ���(ISR)
//******************************************************************************************
interrupt void sciaRxFifoIsr(void)
{   
    Uint16 i;
	for(i=0;i<8;i++)	{ rdataA[i]=SciaRegs.SCIRXBUF.all; }
	   	 	// �������ջ�����SCIRXBUF�е����ݣ�������������iλ������8������
	for(i=0;i<8;i++)                     // �����յ�����
	{
	   if(rdataA[i] != ( (rdata_pointA+i) & 0x00FF) ) error();
// ���rdataA[i]��rdata_pointA+i��Ӧ8��������ȷ񣬲���ת�����������ֹ�������; ���˳ִ��
	   		// ����rdata_pointA ���ٷ�������A sdataA[]�еĵ�һ��Ԫ�أ�
// ���(rdata_pointA+i)��Ϊ��������Ķ�ӦԪ�ء�
// ���������i��Ԫ��rdataA[i]Ϊ��������A sdataA[]�еĵ�i��Ԫ�أ�
	   		// rdataA[i]��(rdata_pointA+i)������ȣ��������󣬷������
	   		// �����õ�DSP281x���д��͵��ڲ����͹��ܡ�
	}
	rdata_pointA = (rdata_pointA+1) & 0x00FF;  
												// ָ����һ�η��������һ��Ԫ��
	if(count>0xFFFF) count=0;
		else
		count++;//��¼����ͨ�ŵĴ���
	LedOut(count);//��ʾ����ͨ��SCI-A�Ĵ���,���ڴ˴���̽�������Ӵ����й۲�
	SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;  		// ��������־λ
	SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;  		// ����жϱ�־λ
	PieCtrlRegs.PIEACK.all|=0x100;      		// ����PIEӦ����ʾ��ӦINT9�ж�		
												// PIEACK��0x100��λ����ٶ�PIEACK��ֵ
}

//******************************************************************************************
// ������ scia_fifo_init():
// ���ܣ� SCI_A �Ƚ��ȳ�(FIFO)��ʼ���ӳ���
// ע��:  DSP281x_SysCtrl.c�ļ��е����໷����InitPll(0x5)��ʵ��ԭֵΪ0xA,�ָ�Ϊ0x5��
// ����Ӿ���30MHz��������,�����໷����ʵ��Ϊ0xAʱ��ϵͳʱ��  SYSCLKOUT=(OSCCLK*DIV)/2=(30*10)/2=150MHz
// (LOSPCP)�Ĵ�����Ĭ��ֵ(Ĭ��ֵΪ2)���ʵ�������ʱ�� LSPCLK=150/4=37.5 MHz
//******************************************************************************************
void scia_fifo_init()										
{
   	SciaRegs.SCICCR.all =0x0007;   
		// ��SCIͨ�ſ��ƼĴ���(SCICCR)�������á�һ������λ����ֹ���ͣ�
		// ��ֹ���Թ��ܣ�8λ�ַ����첽��ʽ��ѡ�������Э�顣
   	SciaRegs.SCICTL1.all =0x0003;  
         // ʹ��TX, RX, �ڲ�SCICLK,��ֹ���մ����жϣ���ֹ˯�ߣ���ֹTXWAKE
    	SciaRegs.SCICTL2.bit.TXINTENA =1;
   		// SCITXBUF�ж�ʹ�ܣ���ʾSCITXBUF�Ĵ���׼��������һ���ַ���
   	SciaRegs.SCICTL2.bit.RXBKINTENA =1;
   		// �������ջ�����/�жϡ���λ��SCI����״̬�Ĵ���(SCIRXST)��RXRDY��
   		// BRKDT��־(SCIRXST.6��.5λ)λ���п���
	ScibRegs.SCIHBAUD   =0x0001; 
   		// SCIHBAUD: ������ѡ�������Ч�ֽڼĴ���
    ScibRegs.SCILBAUD   =0x00E7; 
   		// SCILBAUD: ������ѡ�������Ч�ֽڼĴ���
	
   	SciaRegs.SCICCR.bit.LOOPBKENA =1;		// �����Բ���ģʽ,Tx�������ڲ����ӵ�Rx����
   	SciaRegs.SCIFFTX.all=0xC028;			// SCI FIFO ���ͼĴ���(SCIFFTX)
   											// SCI FIFO���¿�ʼ�ͽ��գ�ʹ��
   											// SCI FIFO����ǿ�͹���
   											// ʹ��ƥ���жϣ�FIFO��λΪ8		
   	SciaRegs.SCIFFRX.all=0x0028;			// ʹ��ƥ���жϣ�FIFO��λΪ8
//   SciaRegs.SCIFFCT.all=0x00;				// SCI FIFO ���ƼĴ���(SCIFFCT)
   	SciaRegs.SCICTL1.all =0x0023;     		// ����ʹ��SCI
   	SciaRegs.SCIFFTX.bit.TXFIFOXRESET=1;	//����ʹ��FIFO����
   	SciaRegs.SCIFFRX.bit.RXFIFORESET=1;	 	//����ʹ��FIFO����
}

//******************************************************************************************
// ������ scibTxFifoIsr
// ���ܣ� SCI_B �����Ƚ��ȳ�(FIFO)�жϷ����ӳ���(ISR)
//******************************************************************************************
interrupt void scibTxFifoIsr(void)
{       
    Uint16 i;
    for(i=0; i< 8; i++)		{ ScibRegs.SCITXBUF=sdataB[i]; }
 	   		// ����������B��i������sdataB[i]���뷢�ͻ�����SCITXBUF������8������
    for(i=0; i< 8; i++)                 		// ��һ�����ڣ���������ȡ������
    {
 	   sdataB[i] = (sdataB[i]-1) & 0x00FF; 
 	   		// ������2��sdataB[i]����һ��sdataB[i]�Ǵ���ǰsdataB[]������ȡ
 	   		// ��i��Ԫ�أ��ڼ�1�����θ�8λ�����¸�ֵ����i��Ԫ�ء�
 	   		// ������ǰһ�������8��Ԫ��Ϊ:		DC,DD,DE,DF,E0,E1,E2,E3
 	   		// ���һ�������8��Ԫ��Ϊ:			DB,DC,DD,DE,DF,E0,E1,E2
	}	
	ScibRegs.SCIFFTX.bit.TXINTCLR=1;    		// SCIFFTX[6]ΪTXINTCLRλ����λ��1
												// ���SCI�жϱ�־λTXFFIN
	PieCtrlRegs.PIEACK.all|=0x100;      		// ����PIEӦ����ʾ��ӦINT9�ж�
}

//*******************************************************************************************
// ������ scibRxFifoIsr()
// ���ܣ� SCI_B �����Ƚ��ȳ�(FIFO)�жϷ����ӳ���(ISR)
//*******************************************************************************************
interrupt void scibRxFifoIsr(void)
{
    Uint16 i;
	for(i=0;i<8;i++) {rdataB[i]=ScibRegs.SCIRXBUF.all; }
	   	 	// �������ջ�����SCIRXBUF�е����ݣ�������������iλ������8������
	for(i=0;i<8;i++)                     		// �����յ�����
	{
	   if(rdataB[i] != ( (rdata_pointB-i) & 0x00FF) ) error();
	   		// ���rdataB[i]��(rdata_pointB-i)��Ӧ8��������ȷ񣬲���ת��������ֹ�������;
			// ���˳ִ�� ����rdata_pointB ���ٷ�������B sdataB[]�еĵ�һ��Ԫ�أ����
			// (rdata_pointB-i) ��Ϊ��������Ķ�ӦԪ�ء����������i��Ԫ��rdataB[i]Ϊ��������B 
			// sdataB[]�еĵ�i��Ԫ�أ�rdataB[i]��(rdata_pointB-i)������ȣ��������󣬷������
	   		// �����õ�DSP281x���д��͵��ڲ����͹��ܡ�
	}

//******************************************************************************************
//  ����3��ָ������Ϊ:
//		(1) ������ֵ��˳���������ַ0x13f000��ʼ�Ķ�Ӧ��44����Ԫ�� 
//		(2) Xaddr_trackΪ��ַ����ָ�룬��Xaddr_trackͬ��������44ʱ��λ��ַָ��
//			����ַ����ָ�롣
//******************************************************************************************
   	for(i=0; i<8; i++)
   	{
		if(Xaddr_track>43) {Xaddr=(Uint16 *)0x13f000;Xaddr_track=0;}
		*Xaddr++ = rdataB[i];		//���ڴ˴���̽����������ַ0x13f000�Ӵ����й۲�
		Xaddr_track++;
	}

	rdata_pointB = (rdata_pointB-1) & 0x00FF; 	// ָ����һ�η��������һ��Ԫ��  
	ScibRegs.SCIFFRX.bit.RXFFOVRCLR=1;  		// ��������־λ
	ScibRegs.SCIFFRX.bit.RXFFINTCLR=1; 			// ����жϱ�־λ
	PieCtrlRegs.PIEACK.all|=0x100;  			// ����PIEӦ����ʾ��ӦINT9�ж�	
												// PIEACK��0x100��λ����ٶ�PIEACK��ֵ
}

//******************************************************************************************
// ������ scib_fifo_init()	
// ���ܣ� SCI_B �Ƚ��ȳ�(FIFO)��ʼ���ӳ���
//******************************************************************************************
void scib_fifo_init()										
{
   ScibRegs.SCICCR.all =0x0007;
			// ��SCIͨ�ſ��ƼĴ���(SCICCR)�������á�һ������λ����ֹ���ͣ�
			// ��ֹ���Թ��ܣ�8λ�ַ����첽��ʽ��ѡ�������Э�顣
   ScibRegs.SCICTL1.all =0x0003;   
         	// ʹ��TX, RX, �ڲ�SCICLK,��ֹ���մ����жϣ���ֹ˯�ߣ���ֹTXWAKE
   ScibRegs.SCICTL2.bit.TXINTENA =1;
			// SCITXBUF�ж�ʹ�ܣ���ʾSCITXBUF�Ĵ���׼��������һ���ַ���
   ScibRegs.SCICTL2.bit.RXBKINTENA =1;
   			// �������ջ�����/�жϡ���λ��SCI����״̬�Ĵ���(SCIRXST)��RXRDY��
   			// BRKDT��־(SCIRXST.6��.5λ)λ���п���

   ScibRegs.SCIHBAUD    =0x0001;		// ������=9600���봮�е�������9600 
   ScibRegs.SCILBAUD    =0xE7;			// ������ƥ�䣬ͨ��
   ScibRegs.SCICCR.bit.LOOPBKENA =1; 	// �����Բ���ģʽ,Tx�������ڲ����ӵ�Rx����
   ScibRegs.SCIFFTX.all=0xC028;			// SCI FIFO ���ͼĴ���(SCIFFTX)	
   										// SCI FIFO���¿�ʼ�ͽ��գ�ʹ��
   										// SCI FIFO����ǿ�͹���
   										// ʹ��ƥ���жϣ�FIFO��λΪ8		  
   ScibRegs.SCIFFRX.all=0x0028;			// ʹ��ƥ���жϣ�FIFO��λΪ8

   ScibRegs.SCICTL1.all =0x0023;     	// ����ʹ��SCI
   ScibRegs.SCIFFTX.bit.TXFIFOXRESET=1;	//����ʹ��FIFO����
   ScibRegs.SCIFFRX.bit.RXFIFORESET=1;	 //����ʹ��FIFO����
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
}

//��¼SCI-Aͨ�Ŵ�������ʾ��16��LED��
void LedOut(Uint16 led)
{
 	Uint16 i;
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
//==========================================================================================
// No more.
//==========================================================================================

