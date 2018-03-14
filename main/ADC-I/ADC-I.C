//��ADCIprj��ͬʱ��CPUtimer0�������ЩСC�ļ�ֱ���϶���ADCIprj��

/**************************************************************
** ��������: ADC���򣬶�ADCINB7ͨ���������жϷ�ʽ**
**************************************************************/

#include "DSP281x_Device.h"
#include "DSP281x_Examples.h"   // DSP281x Examples Include File
unsigned long int  AD1=2000;  
unsigned long int  AD2; //ʵ��ADֵ*1000
unsigned long int  sum=0;

//ʱ����ض���
Uint16 Timer_10ms=0;//��ʱ��10ms����
Uint16 Clock_10ms=0;//ʱ��10ms����
Uint16 Clock_ss=45;//ʱ����
Uint16 Clock_mm=59;//ʱ�ӷ�
Uint16 Clock_hh=23;//ʱ��ʱ
Uint16 BDAT;
Uint16 BDIR;

interrupt void cpu_timer0_isr(void);//�ж�����
interrupt void adc_isr(void);
void Write_LED (int LEDReg);//ͨ��SPI��LED������ʾ����




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
    EDIS;
    GpioDataRegs.GPADAT.bit.GPIOA11=0; 	//GPIOA11=0;�ö˿�Ϊ74HC595�����ź�
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
/*�ӳ������*/
unsigned long int i = 0;

//ADC��ؼĴ�����ʼ��
void Adc_Init()
{
	// Configure ADC @SYSCLKOUT = 150Mhz
    AdcRegs.ADCTRL1.bit.SEQ_CASC = 0;      //˫����/����ѡ��:˫���й���ģʽ
    AdcRegs.ADCTRL3.bit.SMODE_SEL = 0;     //����/����ѡ��:����������ʽ
    AdcRegs.ADCTRL1.bit.CONT_RUN = 0;      //����-ֹͣ/����ת��ѡ��:����-ֹͣ��ʽ
    AdcRegs.ADCTRL1.bit.CPS = 1;           //��ʱ��Ԥ������:ADC_CLK=ADCLKPS/2=3.125M
  	AdcRegs.ADCTRL1.bit.ACQ_PS = 0xf;      //�ɼ����ڴ�С:SH pulse/clock=16
   	AdcRegs.ADCTRL3.bit.ADCCLKPS = 0x2;    //��ʱ�ӷ�Ƶ:ADCLKPS=HSPCLK/4=6.25M
	AdcRegs.ADCMAXCONV.all = 0x0000;       //ת��ͨ����:SEQ1���е�ͨ����Ϊ1
  	AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0xf; //ת��ͨ��ѡ��:ADCINB7 
}

//ADCģ���ϵ���ʱ
void Adc_PowerUP() 
{ 	
  	AdcRegs.ADCTRL3.bit.ADCBGRFDN = 0x3;     //ADC��϶�Ͳο���·�ӵ�
  	for (i=0; i<1000000; i++){}      //����5ms��ʱ
  	AdcRegs.ADCTRL3.bit.ADCPWDN = 1;		 //ADC��ģ���·�ӵ�
  	for (i=0; i<10000; i++){}        //����20us��ʱ
}


//����ȫ�ֱ���:
Uint16 LoopCount;//��Чѭ������
Uint16 ConversionCount;//��ǰ�����0-20
Uint16 Voltage[20];//���20��ADCRESULT0ֵ

main() 
{
    InitSysCtrl();   

    EALLOW;
    SysCtrlRegs.HISPCP.all = 0x3;	// HSPCLK = SYSCLKOUT/6����25HZ
    EDIS;

	DINT;                           /*��CPU�ж�*/
	IER = 0x0000;                   //�ر���Χ�ж�
	IFR = 0x0000;                    //���жϱ�־
	spi_intial();                    //SPI��ʼ���ӳ���
	gpio_init();	                 //GPIO��ʼ���ӳ���
	Adc_PowerUP();
	Adc_Init();
	
	InitPieCtrl();       //��ʼ��PIE���ƼĴ��� DSP281x_PieCtrl.c

	InitPieVectTable();	//��ʼ��PIEʸ����DSP281x_PieVect.c       
        InitAdc();    //��ʼ��ADCģ�飬�ú�����DSP28_Adc.c�ļ���
	
        EALLOW;	//ʹ��д�����Ĵ�����д����
	PieVectTable.ADCINT = &adc_isr;        //���û��жϷ������ڵ�ַ
                                               //�����ж�������ͷ�ļ��еĶ�Ӧ����
        PieVectTable.TINT0 = &cpu_timer0_isr;//PIE�ж�ʸ����
	EDIS;       // ��ֹд�����Ĵ�����д����

	   InitCpuTimers();   // For this example, only initialize the Cpu Timers
	//��ʱ����һ��������£������ڼĴ�����PRDH:PRD�е�ֵװ��32λ�����Ĵ���TIMH:TIM��
	//Ȼ������Ĵ�����C28X ��SYSCLKOUT���ʵݼ���������������0ʱ���ͻ����һ����ʱ��
	//�ж�����źţ�һ���ж����壩��
	// 150MHz CPU Freq, 1 second Period (in uSeconds)
	//�����жϵļ��=�����ڼĴ����е�ֵ��/SYSCLKOUT/��Ƶϵ��
	//���ڼĴ����е�ֵ=(Freq*Period)
	   ConfigCpuTimer(&CpuTimer0, 150, AD1*5);//��Ϊ��ȷ��С�������λ���������ڶ�λ10ms
	   StartCpuTimer0();


        PieCtrlRegs.PIEIER1.bit.INTx6 = 1; //ʹ��PIE�е�ADCINT�ж�
        PieCtrlRegs.PIEIER1.bit.INTx7 = 1;//CPU-Timer0ʹ��λ��PIE��1���7��������ʹ��
/*����ʱ��ע��˴���PIEIER1��λ���Ƿ��ͻ��*/

	IER |= M_INT1;					// // ʹ�� CPU �ж� 1��ʹ��ȫ�� INT1

	EINT;   								// ʹ��ȫ���ж� INTM
	ERTM;	  							// ʹ��ȫ��ʵʱ�ж� DBGM
    LoopCount = 0;           				//ѭ������������
    ConversionCount = 0;      				//��ǰת�����������
// ���� ADC
    AdcRegs.ADCMAXCONV.all = 0x0000;       // ����SEQ1��1��ת��ͨ��
  	AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0xf; //ת��ͨ��ѡ��:ADCINB7 

    AdcRegs.ADCTRL2.bit.EVA_SOC_SEQ1 = 1;  // ʹ�� EVASOC ȥ���� SEQ1
    AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1 = 1;  // ʹ�� SEQ1 �ж� (ÿ�� EOS)
// ���� EVA
// ����EVA�Ѿ��� InitSysCtrl()��ʹ��;
    EvaRegs.T1CMPR =0x0080;               // ���� T1 �Ƚ�ֵ
    EvaRegs.T1PR = 0xFFFF;                 // �������ڼĴ���
    EvaRegs.GPTCONA.bit.T1TOADC = 1; // ʹ��EVA�е� EVASOC(�����ж�����ADC) 
    EvaRegs.T1CON.all = 0x1042;        // ʹ�ܶ�ʱ��1�Ƚϲ��� (������ģʽ )
//��ADCת��
    while (1)
    {
   				
     LEDdisplay(
					Clock_hh/10,
					Clock_hh%10,
					17,         //б��
					Clock_mm/10,
					Clock_mm%10,
					17,
					Clock_ss/10,
					Clock_ss%10
					);
    }
}
interrupt void  adc_isr(void)
{
  Voltage[ConversionCount] =( AdcRegs.ADCRESULT0>>4);
 
// ����Ѽ�¼��20��������¿�ʼת��
  if(ConversionCount == 19) 
  {
     ConversionCount = 0;
   		AD1=sum/20;
		AD2=(AD1*3*1000)/4095;   //ʵ��ADֵ*1000
   		sum=0;
  }
  else 
  {
     sum=sum+Voltage[ConversionCount];
     ConversionCount++;
  } 	
// ���³�ʼ����һ��ADCת��
  AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1;         // ��λ SEQ1
  AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;		// �� INT SEQ1λ
  PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;   // ���ж�Ӧ���źţ�׼��������һ���ж�
  return;                      
}

//�жϷ������
interrupt void cpu_timer0_isr(void)
{

	//BDAT=GpioDataRegs.GPBDAT.all;
	//BDIR=GpioMuxRegs.GPEDIR.all;

	Clock_10ms=Clock_10ms+AD1/100+1;

	if(Clock_10ms>100)
	{
		Clock_10ms=0;
		if(++Clock_ss>=60)
		{
			Clock_ss=0;
			if(++Clock_mm>=60)
			{
				Clock_mm=0;
				if(++Clock_hh>=24)
				{
					Clock_hh=0;
				}
			}
		}
	}
	//GpioDataRegs.GPBDAT.all=BDAT;
	//GpioMuxRegs.GPEDIR.all=BDIR;

	PieCtrlRegs.PIEACK.all=0x0001;//����ж�Ӧ���źţ�׼��������һ���ж�??????????????????
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
