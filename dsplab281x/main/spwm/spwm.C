/******************************************************************************************
*����:spwm���η�����,PWM1-6���ƿ���Ƶ��60KHz, sin����8192(8K)��ֵ0��1250,Ƶ�ʿɵ���Χ4Hz��2KHz
*�ж�T1PINT����PWM1��6��ռ�ձȣ���ȫ�Ƚ�������3��PWM����GP��ʱ��1��ȫ�Ƚϵ�Ԫʱ��
******************************************************************************************/
#include"DSP281x_Device.h"
#include   "math.h"           //Ҫ�õ�����ֵ�ļ���

static  long  x1=0;
static  long  x2=0;
static  long  x3=0;	

unsigned long int i = 0;
void  IOinit(void);

Uint16  sindata[8192];     //�����ұ�
interrupt void   T1pint_isr(void);//�ж��ӳ��������

void  stop_pwm(void);        //ֹͣPWM�ӳ���
void  starlpwm(void);         //����PWM�ӳ���

#define   PI2   3.1416* 2      //2��
#define   FC    60000        //�����ز�Ƶ��60KHz(PWM1��6)
#define   sinlen 8192           //8K�����ұ�����
long     Tlpr;              //����n��ʱ�������ڼĴ�����ֵ

unsigned long int    deaout=0;   //EVA���Ƶ��
unsigned long int    dea=0;   //EVA���Ƶ�ʶ�Ӧ����ȷ�ֵ
unsigned long int    dea1=0;   //��Ӧ����ȷ�ֵ�м����
int  j;

void EVA_PWM()
{
   	EvaRegs.EXTCONA.bit.INDCOE = 1;  //����ʹ�ܱȽ����ģʽ
    EvaRegs.ACTRA.all = 0x0aaa;     //�ռ�ʸ��������
    EvaRegs.DBTCONA.all = 0x08ec;   //������ʱ������
    EvaRegs.CMPR1 = 0x0006;
    EvaRegs.CMPR2 = 0x0005;
    EvaRegs.CMPR3 = 0x0004;
    EvaRegs.COMCONA.all = 0xa6e0;   //�ռ�������ֹ��ȫ�Ƚ�ʹ�ܣ������ֹ
}

void EVA_Timer1()
{
   	EvaRegs.EXTCONA.bit.INDCOE = 1;  //����ʹ�ܱȽ����ģʽ
    EvaRegs.GPTCONA.all = 0x0000;   //GP��ʱ��1�Ƚ��������Ч
  	Tlpr=75000000/FC;
	EvaRegs.T1PR=Tlpr;   
    EvaRegs.T1CNT = 0x0000;     // ��ʱ����ֵ
    EvaRegs.T1CON.all = 0x0846;//������������,1��Ƶ,ʹ�ܱȽ�,�ڲ�ʱ��,�򿪶�ʱ��
}

void main(void)
{		
    InitSysCtrl();  //��ʼ��ϵͳ���ƼĴ���, ʱ��Ƶ��150M,HSPCLKΪ75M
	DINT;	        //�ر����жϣ�����жϱ�־
	IER = 0x0000;   //�ر���Χ�ж�
	IFR = 0x0000;	//���жϱ�־ 	
	EVA_PWM();	
	EVA_Timer1();

	EALLOW;
	GpioMuxRegs.GPAMUX.all=0x003F;    //EVAPWMl-6
	EDIS;

//��ʼ��PIE���ƼĴ���
InitPieCtrl();    
//��ʼ��PIE�ж�������
InitPieVectTable(); 

//ʹ��Eva��Ӧλ
	EvaRegs.COMCONA.bit.FCOMPOE=1;	//�Ƚϵ�Ԫʹ��
	EvaRegs.EVAIMRA.bit.T1PINT=1;	//ʹ��T1PINT�ж�
	EvaRegs.EVAIFRA.bit.T1PINT=1;	//����

//����ʼ��ռ�ձ��б�
	for(j=0;j<8192;j++)	//���߼������ұ�
	{
	sindata[j]=625+sin(j*PI2/8192)*625*0.85;
	}

//####�ж�ʹ��
	EALLOW;
	PieVectTable.T1PINT=&T1pint_isr;//PIE�ж�ʸ����	
	EDIS;

	PieCtrlRegs.PIEIER2.bit.INTx4=1;	//ʹ��T1PINT
	IER |= M_INT2;	//ʹ��INT2

	EINT;	//����ȫ���ж�
	ERTM;	//EnableGlobal realtime interrupt DBGM

//��ѭ��
// for(;;){;}
 	while (1)
 	{
	dea1=8192*deaout*2/1000;//�м���������������
	dea=dea1*Tlpr/75000;//�������ȷ�ֵ
	}
}

	interrupt  void  T1pint_isr(void)//�ж�T1PINT����PWMl��6��ռ�ձ�(60k)
{
	static  long  Ia=0;
	static  long  resa=0;

//####  ����pwm1��6ռ�ձ�
	x1=Ia*dea;
	x1=resa+x1;//A��Ƕ�
	Ia++;	//ʱ��ȷ�
	if(x1>(sinlen-1))	//����������ұ����ȣ����ȥ�������ұ�����
	{
	resa=x1-sinlen;	//A��Ƕȷ�Χ������0��360
	x1=resa;
	Ia=1;	//���¿�ʼ������t=0
	}
	x2=x1-2731;	//x2=x1-sinlen��3��B��
	x3=x1-5461;	//x3=xl-sinlen*2��3��C��
	if(x2<0)
	x2=x2+sinlen;	//B��Ƕȷ�Χ������0��360��
	if(x3<0)	
	x3=x3+sinlen;		//C��Ƕȷ�Χ������0��360��
	EvaRegs.CMPR1=sindata[x1];	//ˢ��ռ�ձ�
	EvaRegs.CMPR2=sindata[x2];
	EvaRegs.CMPR3=sindata[x3];

	//####  ��λ
	EvaRegs.EVAIFRA.bit.T1PINT=1;	//�жϱ�־��
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP2; //�����´��ж�
	return;
}
