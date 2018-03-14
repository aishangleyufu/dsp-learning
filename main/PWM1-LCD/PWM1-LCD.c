/*****************************************************************
**����:����GP��ʱ��4�ıȽ����ڲ���PWM��������LCD��������**
**ϵͳʱ��150M����������ʱ��25M��128��Ƶ��ʱ��Ϊ5.12us******
****************************************************************/
#include "DSP281x_Device.h"

void 	EVB_Timer4()
{
    EvbRegs.EXTCONB.bit.INDCOE = 1; //����ʹ�ܱȽ����ģʽ
    EvbRegs.GPTCONB.all = 0x0024;    //GP��ʱ��4�Ƚ��������Ч
    EvbRegs.T4PR = 0x0016;           //��ʱ����Ϊ5.12us*(T1PR+1)
    EvbRegs.T4CMPR = 0x0008;  //GP��ʱ���ıȽϼĴ�����������ֵ���ɵ�����������
    EvbRegs.T4CNT = 0x0000;   //��ʱ����ֵ
    EvbRegs.T4CON.all = 0x1742;      //������������128��Ƶ��ʹ�ܱȽϣ��򿪶�ʱ��
}

void IOinit()
{
 	EALLOW;  
 	//��GPIOB7����Ϊ�����
 	GpioMuxRegs.GPBMUX.bit.T4PWM_GPIOB7 = 1;
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
	EVB_Timer4();
	for(;;){;}
} 
