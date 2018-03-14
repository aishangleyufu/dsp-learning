/******************************************************************************************
**����:PWM1-6���������6�����عܣ�����Ƶ��60KHz
***ADC�ж���T2����������Ƶ��10K(T2)
**�ж�T1PINT����PWM1��6��ռ�ձ�(60k)����ȫ�Ƚ�������3��PWM����GP��ʱ��1��ȫ�Ƚϵ�Ԫʱ��
**�ж�T3PINT����PWM7��12��ռ�ձ�(60k)����ȫ�Ƚ�������3��PWM����GP��ʱ��3��ȫ�Ƚϵ�Ԫʱ��
**�ж�T4PINTɨ�谴��(10.24 ms)��ʵ����·DAת��,������ֵ(0-9)��SCI��������*
**PDPINTA�жϹ��ϱ�������
**SCI���жϷ�ʽ����,����λ�����ڵ��Գ���ʵ����λ��ͨѶ
**Ecan���жϷ�ʽʵ��˫��ͨѶ,����B��������������,����Ӧ��Ƶ��ֵ���ͶԷ�
**���ж�Ƕ��
**����C��:�ü����趨deaoutƵ��;����D��:��ADCINB7ͨ���趨deaoutƵ��;
**���ܼ�E/F,��E�����Ƶ��debout�ɼ���0-D�趨,��F�����Ƶ��debout��ADCINB7�趨;
// sin��8192(8K)��ֵ0��1250, Ƶ�ʵ���4Hz��2KHz
******************************************************************************************/

#include"DSP281x_Device.h"
#include   "math.h"           //�õ�����ֵ�ļ���

static  long  x1=0;
static  long  x2=0;
static  long  x3=0;	
static  long  x4=0;
static  long  x5=0;
static  long  x6=0;	
unsigned int LEDReg;
unsigned int LED4=0;    //Ҫ��ʾ����λƵ��ֵ
unsigned int LED3=0;
unsigned int LED2=0;
unsigned int LED1=0;
unsigned int KeyReg1;
unsigned int KeyReg2;
unsigned int flag=0;
unsigned long int i = 0;
void  IOinit(void);
void  LedOut(Uint16 led);
int   KeyIn1(void);
void  KeyFunction1(unsigned int KeyReg1);
void  KeyFunction2(unsigned int KeyReg2);
void   DAC(void);
void Write_LED (int LEDReg);//ͨ��SPI��LED������ʾ����
Uint16 keyNum = 0x0000;  //��������
Uint16 RecieveChar; 
unsigned long int  AD1;  
unsigned long int AD2;  ////ʵ��ADֵ*1000

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

Uint16  sindata[8192];     //�����ұ�
interrupt void  T1pint_isr(void);//�ж��ӳ��������
interrupt void  T3pint_isr (void);
interrupt void  T4pint_isr(void);
interrupt void  adc_isr(void);
interrupt void  Scirxinta_isr(void);
interrupt void  Ecan0inta_isr(void);
interrupt void  pdpaint_isr(void);

void  stop_pwm(void);       //ֹͣPWM�ӳ���
void  starlpwm(void);       //����PWM�ӳ���

#define   PI2   3.1416* 2  //2��
#define   FC    60000    //�����ز�Ƶ��60KHz(PWM1��6)
#define   FC2   10000     //A��D����Ƶ��10KHz
#define   sinlen 8192     //8K�����ұ���

long     Tlpr;            //����Tl��ʱ�������ڼĴ�����ֵ
long     T2pr;
long     T3pr;           //����T2��T3��ʱ�������ڼĴ�����ֵ

unsigned long int    deaout=0;   //EVA���Ƶ�ʣ���T4�жϳ������ɰ����ı�
unsigned long int    dea=0;   //EVA���Ƶ�ʶ�Ӧ���ȷ�ֵ
unsigned long int    dea1=0;   //��Ӧ���ȷ�ֵ�м����
unsigned long int    debout=0;   //EVB���Ƶ�ʣ���T4�жϳ������ɰ����ı�
unsigned long int    deb=0;   //EVB���Ƶ�ʶ�Ӧ���ȷ�ֵ
unsigned long int    deb1=0;   //��Ӧ���ȷ�ֵ�м����
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
    EvaRegs.T1CON.all = 0x0846;//.������������,1��Ƶ,ʹ�ܱȽ�,�ڲ�ʱ��,�򿪶�ʱ��
}

void EVB_PWM()
{
   EvbRegs.EXTCONB.bit.INDCOE = 1;  //����ʹ�ܱȽ����ģʽ
    EvbRegs.ACTRB.all = 0x0aaa;     //�ռ�ʸ��������
    EvbRegs.DBTCONB.all = 0x08ec;   //������ʱ������
    EvbRegs.CMPR4 = 0x0010;
    EvbRegs.CMPR5 = 0x0005;
    EvbRegs.CMPR6 = 0x0004;
    EvbRegs.COMCONB.all = 0xa6e0;   //�ռ�������ֹ��ȫ�Ƚ�ʹ�ܣ������ֹ
}

void EVB_Timer3()
{
   EvbRegs.EXTCONB.bit.INDCOE = 1;  //����ʹ�ܱȽ����ģʽ
    EvbRegs.GPTCONB.all = 0x0000;   //GP��ʱ��1�Ƚ��������Ч
 	T3pr=75000000/FC;
	EvbRegs.T3PR=T3pr;   
    EvbRegs.T3CNT = 0x0000;     // ��ʱ����ֵ
    EvbRegs.T3CON.all = 0x0846;////������������,1��Ƶ,ʹ�ܱȽ�,�ڲ�ʱ��,�򿪶�ʱ��
}

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
	EDIS;				
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

void gpio_init()
{ 
	EALLOW;
	GpioMuxRegs.GPAMUX.bit.TDIRA_GPIOA11=0; //GPIOA11����Ϊһ��I/O��
	GpioMuxRegs.GPADIR.bit.GPIOA11=1;		//��GPIOA11����Ϊ���						    
   
	GpioMuxRegs.GPFMUX.bit.XF_GPIOF14 = 0;  //  GPIOF14����Ϊһ��I/O�˿�
	GpioMuxRegs.GPFDIR.bit.GPIOF14 = 1;	   //  ��GPIOF14����Ϊ���			
    EDIS;
    GpioDataRegs.GPADAT.bit.GPIOA11=0; 	//GPIOA11=0;�ö˿�Ϊ74HC595�����ź�
    GpioDataRegs.GPFDAT.bit.GPIOF14 = 1; //  GPIOF14��1;�ö˿�Ϊʹ���ź�
}

void Scia_init()
{    
	EALLOW;
 	GpioMuxRegs.GPFMUX.bit.SCITXDA_GPIOF4 = 1;	 //  ����F4��F5Ϊͨ�Ŷ˿�
 	GpioMuxRegs.GPFMUX.bit.SCIRXDA_GPIOF5 = 1;	 
    EDIS;
	SciaRegs.SCICTL2.all = 0x0002;  //  ����RX�ж�
 	SciaRegs.SCILBAUD = 0x00E7;     //  �����ʣ�9600
	SciaRegs.SCIHBAUD = 0x0001;  	     
  SciaRegs.SCICCR.all = 0x0007;  	//  1��ֹͣλ����ֹУ�飬8λ�ַ� 
                             		//  ��ֹ�Բ��ԣ��첽������Э��
	SciaRegs.SCICTL1.all = 0x0023;  //  ���븴λ״̬��ʹ�ܽ��շ���
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

//ADCģ���ϵ���ʱ
void Adc_PowerUP() 
{ 	
  	AdcRegs.ADCTRL3.bit.ADCBGRFDN = 0x3;  //ADC��϶�Ͳο���·�ӵ�
  	for (i=0; i<1000000; i++){}      //����5ms��ʱ
  	AdcRegs.ADCTRL3.bit.ADCPWDN = 1; //ADC��ģ���·�ӵ�
  	for (i=0; i<10000; i++){}        //����20us��ʱ
}

void Adc_Init()
{
    AdcRegs.ADCTRL1.bit.CONT_RUN = 0;      //����-ֹͣ/����ת��ѡ��:����-ֹͣ��ʽ
    AdcRegs.ADCTRL1.bit.CPS = 1;           //��ʱ��Ԥ������:ADC_CLK=ADCLKPS/2=3.125M
  	AdcRegs.ADCTRL1.bit.ACQ_PS = 0xf;      //�ɼ����ڴ�С:SH pulse/clock=16
   	AdcRegs.ADCTRL3.bit.ADCCLKPS = 0x5;    //��ʱ�ӷ�Ƶ:ADCLKPS=HSPCLK/4=6.25M
	AdcRegs.ADCMAXCONV.all = 0x0000;       //ת��ͨ����:SEQ1���е�ͨ����Ϊ1
  	AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0xf; //ת��ͨ��ѡ��:ADCINB7
  	 
	AdcRegs.ADCTRL2.bit.EVA_SOC_SEQ1=1;  //SEQl��EVA�Ĵ���Դ����
  	AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1=1;  //ʹ���ж�		
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
   	asm (" NOP ");
   	InitSysCtrl();      //��ʼ��ϵͳ���ƼĴ���, ʱ��Ƶ��150M
	EALLOW;				
	SysCtrlRegs.HISPCP.all = 0x0001;//����ʱ�ӵĹ���Ƶ�ʣ�SYSCLKOUT/2=75M
	EDIS;
	DINT;	        //�ر����жϣ�����жϱ�־
	IER = 0x0000;   //�ر���Χ�ж�
	IFR = 0x0000;	//���жϱ�־
	spi_intial();   //SPI��ʼ���ӳ���
	gpio_init();   
	IOinit();	    // I/O��ʼ���ӳ���	
	EVA_PWM();	
	EVA_Timer1();
	EVB_PWM();	
	EVB_Timer3 ();
  	Scia_init();//��ʼ��SCl
   	CAN_INIT();

	EALLOW;
	GpioMuxRegs.GPAMUX.all=0x003F;    //EVAPWMl-6
	GpioMuxRegs.GPBMUX.all=0x003F;    //EVBPWM7-12
	EDIS;

//��ʼ��PIE���ƼĴ���
	InitPieCtrl();       //DSP281x_PieCtrl��C
//��ʼ��PIE�ж�������
	InitPieVectTable();  //DSP281x_PieVect��C
	LEDdisplay(0,0,0,0,0,0,0,0);

//����ʼ��ADC�Ĵ���
 	Adc_PowerUP();
 	Adc_Init();

//ʹ��Eva��Evb��Ӧλ
	EvaRegs.COMCONA.bit.FCOMPOE=1;	//�Ƚϵ�Ԫʹ��
	EvaRegs.EVAIMRA.bit.T1PINT=1;	//ʹ��T1PINT�ж�
	EvaRegs.EVAIFRA.bit.T1PINT=1;	//����

	EvbRegs.COMCONB.bit.FCOMPOE=1;	//�Ƚϵ�Ԫʹ��
	EvbRegs.EVBIMRA.bit.T3PINT=1;	//ʹ��T3PINT�ж�
	EvbRegs.EVBIFRA.bit.T3PINT=1;	//����

//T2������ADת��
	T2pr=75000000/FC2;
	EvaRegs.T2PR=T2pr;	//T2��ʱ������
	EvaRegs.GPTCONA.bit.T2TOADC=2;	//�ɶ�ʱ��2�����ж�����A��Dת��
	EvaRegs.T2CON.all=0x084C;
	EvaRegs.T2CNT=0x0000;	//T2��������

//T4������ ����ʼ����������ж�
	EvbRegs.T4PR=12000;	//T4��ʱ������
	EvbRegs.T4CON.all=0x1744;	//x��128��������10��24 ms
	EvbRegs.T4CNT=0x0000;//T4��������

	EvbRegs.EVBIMRB.bit.T4PINT=1;	//ʹ��T4PINT�ж�
	EvbRegs.EVBIFRB.bit.T4PINT=1;	//����

//����ʼ��ռ�ձ��б�
	for(j=0;j<8192;j++)	//���߼������ұ�
	{
	sindata[j]=625-sin(j*PI2/8192)*625*0.85;
	}

//####����ʼ��Xintf
	InitXintf();	//��ʼ��XINTF

//####�ж�ʹ��
	EALLOW;
	PieVectTable.T1PINT=&T1pint_isr;//PIE�ж�ʸ����
	PieVectTable.T3PINT=&T3pint_isr;
	PieVectTable.T4PINT=&T4pint_isr;
	PieVectTable.ADCINT=&adc_isr;
	PieVectTable.RXAINT=&Scirxinta_isr;
	PieVectTable.PDPINTA=&pdpaint_isr;	
	PieVectTable.ECAN0INTA=&Ecan0inta_isr;
	EDIS;

	PieCtrlRegs.PIEIER2.bit.INTx4=1;	//ʹ��T1PINT
	PieCtrlRegs.PIEIER4.bit.INTx4=1;	//ʹ��T3PINT
	PieCtrlRegs.PIEIER5.bit.INTx1=1;	//ʹ��T4PINT
	PieCtrlRegs.PIEIER1.bit.INTx6=1;	//ʹ��ADCINT
	PieCtrlRegs.PIEIER9.bit.INTx1=1;	//ʹ��SCIRXlNTA
	PieCtrlRegs.PIEIER9.bit.INTx5=1;	//ʹ��ECAN0lNT
	PieCtrlRegs.PIEIER1.bit.INTx1=1;	//ʹ��PDPAINT

	IER |=(M_INT1 | M_INT2 | M_INT5 |M_INT9 |M_INT4);//ʹ��INTl��INT2��INT5��INT9

	EINT;	//����ȫ���ж�
	ERTM;	//EnableGlobal realtime interrupt DBGM

//��ѭ��
	while (1)
 	{
	dea1=8192*deaout*2/1000;//�м���������������
	dea=dea1*Tlpr/75000;//������ȷ�ֵ
	deb1=8192*debout*2/1000;
	deb=deb1*T3pr/75000;		
	}
}

interrupt  void  T1pint_isr(void)	//�ж�T1PINT����PWMl��6��ռ�ձ�(60k)
{
	static  long  Ia=0;
	static  long  resa=0;
//####  ����pwm1��6ռ�ձ�EVA
	x1=Ia*dea;
	x1=resa+x1;//A��Ƕ�
	Ia++;	//ʱ��
	if(x1>(sinlen-1))	//����������ұ������ȥ�������ұ���
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
	x3=x3+sinlen;	//C��Ƕȷ�Χ������0��360��
	EvaRegs.CMPR1=sindata[x1];	//ˢ��ռ�ձ�
	EvaRegs.CMPR2=sindata[x2];
	EvaRegs.CMPR3=sindata[x3];
	//####  ��λ
	EvaRegs.EVAIFRA.bit.T1PINT=1;	//�жϱ�־��
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP2; //�����´��ж�
	return;
}

interrupt void  T3pint_isr (void)
	{
	static  long  Ib=0;
	static  long  resb=0;

//####  ����pwm7��11ռ�ձ�EVB
	x4=Ib*deb;
	x4=resb+x4;//A��Ƕ�
	Ib++;	//ʱ��
	if(x4>(sinlen-1))	//����������ұ��ȣ����ȥ�������ұ���
	{
	resb=x4-sinlen;	//A��Ƕȷ�Χ������0��360
	x4=resb;
	Ib=1;	//���¿�ʼ������t=0
	}
	x5=x4-2731;	//x5=x4-sinlen��3��B��
	x6=x4-5461;	//x6=x4-sinlen*2��3��C��
	if(x5<0)
	x5=x5+sinlen;	//B��Ƕȷ�Χ������0��360��
	if(x6<0)	
	x6=x6+sinlen;		//C��Ƕȷ�Χ������0��360��
	EvbRegs.CMPR4=sindata[x4];	//ˢ��ռ�ձ�
	EvbRegs.CMPR5=sindata[x5];
	EvbRegs.CMPR6=sindata[x6];
	//####  ��λ
	EvbRegs.EVBIFRA.bit.T3PINT=1;	//�жϱ�־��
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP4; //�����´��ж�
	return;
	}

interrupt void   T4pint_isr(void)
	{
		DAC();  //��·DAת��
 	    if (KeyIn1() == 1)     // ���ò��K1~K8�ӳ���
 	    {
 	    keyNum=keyNum+1; 	 
	    KeyFunction1(KeyReg1);
		SciaRegs.SCITXBUF=LEDReg+48;  //���ͼ�ֵ,ת����ASCII��0��9
		if(keyNum>4)
		{
		keyNum=keyNum-4;
		LED4=LEDReg;
		}
 	    if(keyNum==1)
		{
		LED4=LEDReg;
		}
		 if(keyNum==2)
		{
		LED3=LEDReg;
		}
		 if(keyNum==3)
		{
		LED2=LEDReg;
		}
 		if(keyNum==4)
		{
		LED1=LEDReg;
		}

 		if(flag==1)
		{
		debout=LED4*1000+LED3*100+LED2*10+LED1; 
 		LEDdisplay(15,11,17,19,LED4,LED3,LED2,LED1); //��ʾFB-��****

		EvbRegs.EVBIFRB.bit.T4PINT=1;	//�жϱ�־��
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //�����´��ж�
		return;
		}
		deaout=LED4*1000+LED3*100+LED2*10+LED1;
 		LEDdisplay(15,10,17,19,LED4,LED3,LED2,LED1); //��ʾFA-��****
	
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//�жϱ�־��
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //�����´��ж�
		return;
 	    }

//����E��:�ü����趨deaoutƵ��;����F��:��ADCINB7ͨ���趨deaoutƵ��;
//����C��:�ü����趨deboutƵ��;����D��:��ADCINB7ͨ���趨deboutƵ��;

		if (KeyIn2() == 1)     // ���ò��K8~K16�ӳ���
 	    {
 	    KeyFunction2(KeyReg2);
		SciaRegs.SCITXBUF=LEDReg+48;  //���ͼ�ֵ��ת����ASCII��0��9
		if(LEDReg==0x0E) 		//���Ƿ�Ϊ����E��
		{
		flag=0;
		LEDdisplay(15,10,17,19,LED4,LED3,LED2,LED1); //��ʾFA-��****
		deaout=LED4*1000+LED3*100+LED2*10+LED1;   //Ƶ���趨ֵ
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//�жϱ�־��
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //�����´��ж�
		return;
		}

		if(LEDReg==0x0F)		////���Ƿ�Ϊ����F��
		{
	 	flag=0;
		//����F��:��ADCINB7ͨ���趨deaoutƵ��				
		deaout=AD1;
		LEDdisplay((AD2/1000)+20,(AD2%1000)/100,(AD2%100)/10,19,
        AD1/1000,(AD1%1000)/100,(AD1%100)/10,AD1%10);
			
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//�жϱ�־��
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //�����´��ж�
		return;	
		}	
	
	if(LEDReg==0x0C) 	//���Ƿ�Ϊ����C��
		{
		flag=1;          //���ù���C����־λ
		LEDdisplay(15,11,17,19,LED4,LED3,LED2,LED1); //��ʾFB-��****
		debout=LED4*1000+LED3*100+LED2*10+LED1;   //Ƶ���趨ֵ

		EvbRegs.EVBIFRB.bit.T4PINT=1;	//�жϱ�־��
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //�����´��ж�
		return;
		}

		if(LEDReg==0x0D)		//���Ƿ�Ϊ����D��
		{
		//����D��:��ADCINB7ͨ���趨deboutƵ��;			
		debout=AD1;
		LEDdisplay((AD2/1000)+20,(AD2%1000)/100,(AD2%100)/10,19,
        AD1/1000,(AD1%1000)/100,(AD1%100)/10,AD1%10);
			
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//�жϱ�־��
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //�����´��ж�
		return;	
		}

	if(LEDReg==0x0B)		////���Ƿ�Ϊ����B�� CAN����
		{	 
	 	if	(flag==1)
		{
		ECanaMboxes.MBOX0.MDL.all = 0x55555502;//���ͱ�־
		ECanaMboxes.MBOX0.MDH.all = debout;
		ECanaRegs.CANTRS.all=0x0001; //����0���� 
		while(ECanaRegs.CANTA.all!=0x0001) {};
		ECanaRegs.CANTA.all=0xFFFF;		//д1��.	
		}
		if	(flag==0)
		{
		ECanaMboxes.MBOX0.MDL.all = 0x55555501;//���ͱ�־
		ECanaMboxes.MBOX0.MDH.all = deaout;
		ECanaRegs.CANTRS.all=0x0001;//����0���� 
		while(ECanaRegs.CANTA.all!=0x0001) {};
		ECanaRegs.CANTA.all=0xFFFF;		//д1��
		}	
	 
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//�жϱ�־��
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //�����´��ж�
		return;
		}
 	   	keyNum=keyNum+1;		
		if(keyNum>4)
		{
		keyNum=keyNum-4;
		LED4=LEDReg;
		}
 	    if(keyNum==1)
		{
		LED4=LEDReg;
		}
		 if(keyNum==2)
		{
		LED3=LEDReg;
		}
		 if(keyNum==3)
		{
		LED2=LEDReg;
		}
 		if(keyNum==4)
		{
		LED1=LEDReg;
		}
				
		if(flag==1)
		{
		debout=LED4*1000+LED3*100+LED2*10+LED1;
		LEDdisplay(15,11,17,19,LED4,LED3,LED2,LED1); //��ʾFB-��****
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//�жϱ�־��
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //�����´��ж�
		return; 
		}
		deaout=LED4*1000+LED3*100+LED2*10+LED1;   //Ƶ���趨ֵ
		LEDdisplay(15,10,17,19,LED4,LED3,LED2,LED1); //��ʾFA-��****
 	     } 
	EvbRegs.EVBIFRB.bit.T4PINT=1;	//�жϱ�־��
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //�����´��ж�
	return; 
	}

interrupt void   adc_isr(void)
	{	
   	   AD1=AdcRegs.ADCRESULT0 >> 4;
	   AD2=(AD1*3*1000)/4095;   //ʵ��ADֵ*1000		

	   AdcRegs.ADCST.bit.INT_SEQ1_CLR=1;//���жϱ�־
       PieCtrlRegs.PIEACK.all=PIEACK_GROUP1; //�����´��ж�       		                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
	return;
	}

interrupt void  Scirxinta_isr(void)
	{
	  EINT;  //�����ж�Ƕ��
	 RecieveChar=SciaRegs.SCIRXBUF.bit.RXDT;
	 SciaRegs.SCITXBUF = RecieveChar;
 	 while(SciaRegs.SCICTL2.bit.TXRDY ==  0){}		
   	 while(SciaRegs.SCICTL2.bit.TXEMPTY ==  0){}
      PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //�����´��ж�	
	return;
	}

interrupt void Ecan0inta_isr(void)	//������Ϣ�����ж�
	{
	if(ECanaRegs.CANRMP.all==0x00010000)
	{
	ECanaRegs.CANRMP.all=0xFFFF0000;
 	mailbox_read(16);
	if(	TestMbox1==0x55555501)
		{
		deaout=TestMbox2;
		LEDdisplay(15,10,17,19, deaout/1000,(deaout%1000)/100,(deaout%100)/10,deaout%10);	
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //�����´��ж�	
		return;
		}
	if(	TestMbox1==0x55555502)
		{
		debout=TestMbox2;
		LEDdisplay(15,11,17,19, debout/1000,(debout%1000)/100,(debout%100)/10,debout%10);
	
		 PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //�����´��ж�	
		return;
		}
	}
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //�����´��ж�
	return;
	}

	interrupt void  pdpaint_isr(void)
	{
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
    //��K8~K16�Ƿ���
    if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
    {
    	for (i=0; i<40000; i++){}        //��ʱ����
        if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
        {
	   KeyReg2=GpioDataRegs.GPBDAT.all|0x00ff ;
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

//MAX5742  ��ͨ��DACת��,SPICCR���л�����λ�����ֱ�ָʾdeaout,debout,AD1,��ֵ��4095
	
void  DAC(void)
{  
 	SpiaRegs.SPICCR.all =0x00cf;   // ʹSPI����16λ����
 	 asm (" NOP ");
   	asm (" NOP ");
 	GpioDataRegs.GPFDAT.bit.GPIOF14 = 0;   //ƬѡDAC  
	 SpiaRegs.SPITXBUF = 0xf010;    // ��ʼ��ָ�DAC_A-Dͨ��ʹ��
   	while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){} 	//�����Ƿ����
   SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;		//�������־
    GpioDataRegs.GPFDAT.bit.GPIOF14 = 1;	//Ƭѡ�ߵ�ƽ��ʱ
    	for(i = 0; i<10; i++){}
   	GpioDataRegs.GPFDAT.bit.GPIOF14 = 0;
	SpiaRegs.SPITXBUF =deaout;          //  DAC_Aͨ������
    	while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){}
   	 	SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;
    		GpioDataRegs.GPFDAT.bit.GPIOF14 = 1; 
    		for(i=0; i<10; i++){}

	GpioDataRegs.GPFDAT.bit.GPIOF14 = 0;
	SpiaRegs.SPITXBUF =debout|0x1000;     //  DAC_Bͨ������
    	while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){}
  	 	SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;
   		GpioDataRegs.GPFDAT.bit.GPIOF14 = 1; 
    		for(i=0; i<10; i++){}

		GpioDataRegs.GPFDAT.bit.GPIOF14 = 0;
	SpiaRegs.SPITXBUF =AD1|0x2000;     //  DAC_Cͨ������
    	while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){}
   	 	SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;
    		GpioDataRegs.GPFDAT.bit.GPIOF14 = 1; 
    		for(i=0; i<10; i++){}		

		GpioDataRegs.GPFDAT.bit.GPIOF14 = 0;
	SpiaRegs.SPITXBUF =AD1|0x3000;          //  DAC_Dͨ������
    	while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){}
   	 	SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;
    		GpioDataRegs.GPFDAT.bit.GPIOF14 = 1; 
    		for(i=0; i<10; i++){}

	SpiaRegs.SPICCR.all =0x00c7; // ʹSPI���ڸ�λ��ʽ,�½���,8λ����,�л�Ϊ��ʾģʽ   
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

