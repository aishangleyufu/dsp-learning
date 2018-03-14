/************************************************************
**����:����GP��ʱ��1�ıȽ����ڲ���һ·PWM��������ʱ��25M**
**ͬʱ��ȫ�Ƚ�������3��PWM����GP��ʱ��1��ȫ�Ƚϵ�Ԫʱ��***
************************************************************/
#include "DSP281x_Device.h"

void EVA_PWM()
{
    EvaRegs.EXTCONA.bit.INDCOE = 1;  //����ʹ�ܱȽ����ģʽ
    EvaRegs.ACTRA.all = 0x0aaa;     //�ռ�ʸ��������
    EvaRegs.DBTCONA.all = 0x08ec;   //������ʱ������
    EvaRegs.CMPR1 = 0x0006;
    EvaRegs.CMPR2 = 0x0005;
    EvaRegs.CMPR3 = 0x0004;
    EvaRegs.COMCONA.all = 0xa4e0;   //�ռ�������ֹ��ȫ�Ƚ�ʹ�ܣ������ֹ
}

void EVA_Timer1()
{
    EvaRegs.EXTCONA.bit.INDCOE = 1;  //����ʹ�ܱȽ����ģʽ
    EvaRegs.GPTCONA.all = 0x0012;   //GP��ʱ��1�Ƚ��������Ч
    EvaRegs.T1PR = 0x0013;      // ��ʱ����Ϊ5.12us*(T1PR+1)
    EvaRegs.T1CMPR = 0x0003;    // GP��ʱ���ıȽϼĴ���
    EvaRegs.T1CNT = 0x0000;     // ��ʱ����ֵ
    EvaRegs.T1CON.all = 0x1742;//������������128��Ƶ��ʹ�ܱȽϣ��򿪶�ʱ��
}

void IOinit()
{
 	EALLOW;  
 	//��GPIOA����Ϊ�����
 	GpioMuxRegs.GPAMUX.all = 0xffff;
	EDIS;			
}

void main(void)
{		
   	InitSysCtrl();      //��ʼ��ϵͳ���ƼĴ���, ʱ��Ƶ��150M
	EALLOW;				
	SysCtrlRegs.HISPCP.all = 0x0003;//����ʱ�ӵĹ���Ƶ�ʣ�25M
	EDIS;
	DINT;	        //�ر����жϣ�����жϱ�־
	IER = 0x0000;   //�ر���Χ�ж�
	IFR = 0x0000;	//���жϱ�־
	IOinit();	
	EVA_PWM();	
	EVA_Timer1();
	for(;;){;}
} 
