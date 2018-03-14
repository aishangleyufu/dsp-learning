/****************************************************************
**����:�����¼�������GP��ʱ��1��GPIOF8~GPIOF13�����ϲ�������ƣ�**
**ϵͳʱ��150M����������ʱ��25M��128��Ƶ��ʱ������Ϊ5.12us****
****************************************************************/
#include "DSP281x_Device.h"
interrupt void eva_timer1_isr(void);
unsigned int LedCount;
const	Uint16	LedCode[]={0xFE00,0xFD00,0xFB00,0xF700,0xEF00,0xDF00,0xFF00};
void IOinit()
{
 	EALLOW;  
 	//��GPIOF8~GPIOF13����Ϊһ��I/O�ڣ����
 	GpioMuxRegs.GPFMUX.all = 0xc0ff;
    GpioMuxRegs.GPFDIR.all = 0x3f00;   
  	EDIS;			
}

void 	EVA_Timer1()
{
    EvaRegs.GPTCONA.all = 0;         // ��ʼ�� EVA Timer 1
    EvaRegs.T1PR = 0x9895;           // ��ʱ����Ϊ5.12us*(T1PR+1)=0.2s
    EvaRegs.EVAIMRA.bit.T1PINT = 1;  //ʹ�ܶ�ʱ��1�������ж�
    EvaRegs.EVAIFRA.bit.T1PINT = 1;   //д1�����ʱ��1�������жϱ�־
    EvaRegs.T1CNT = 0x0000;
    EvaRegs.T1CON.all = 0x1740;       //������������128��Ƶ���򿪶�ʱ��
}
void main(void)
{
	LedCount=0;
	InitSysCtrl();      //��ʼ��ϵͳ���ƼĴ���, ʱ��Ƶ��150M
	EALLOW;				
	SysCtrlRegs.HISPCP.all = 0x0003;//����ʱ�ӵĹ���Ƶ�ʣ�25M
	EDIS;
	DINT;	        //�ر����жϣ�����жϱ�־
	IER = 0x0000;    //�ر���Χ�ж�
	IFR = 0x0000;    	//���жϱ�־
	InitPieCtrl();		//��ʼ��PIE���ƼĴ���
	InitPieVectTable();
	IOinit();		
	EVA_Timer1();
	EALLOW;				
	
	PieVectTable.T1PINT = &eva_timer1_isr;     //�жϷ��������ڵ�ַ�����ж�������
	EDIS;              
	//����ʹ�ܸ����жϣ���������Ӧ�ж�λ->PIE������->CPU
    PieCtrlRegs.PIEIER2.all = M_INT4;   //GP��ʱ��1ʹ��λ��PIE��2���4��������ʹ��
	IER |= M_INT2;            //PIE��2���Ӧ��CPU�Ŀ������ж�2��INT2��������ʹ��
	EINT;   //�����ж�
	for(;;){;}
} 	

interrupt void eva_timer1_isr(void)
{ 	
	GpioDataRegs.GPFDAT.all= LedCode[LedCount];
	LedCount++;
	if (LedCount>=7)	LedCount=0;

	EvaRegs.EVAIMRA.bit.T1PINT = 1;	         //ʹ�ܶ�ʱ��1�������ж�
    EvaRegs.EVAIFRA.bit.T1PINT = 1;		     //д1�����ʱ��1�������жϱ�־
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;  //���� PIEACK�еĵ�2���ж϶�Ӧλ                                       
}
