#include "DSP281x_Device.h"
#include "DSP281x_Examples.h"

void Timer(void);		 			//ʱ�Ӽ�ʱ
interrupt void INT_1_7(void);		//CPU_timer0��ʱ���ж�
void InitCputimer(void);			//CPU_timer0��ʼ��
void LEDdisplay(Uint16* led);
void Write_LED (Uint16 LEDReg);
void spi_intial();

//*��ʱ������
Uint16 cnt_2ms=0;				//��ʱ��2ms����
Uint16 cnt_500ms=0;				//��ʱ��500ms����
extern Uint16 cnt_second_l=0;   //��ʱ���������λ		/�ֳɸ�λ�͵�λ���ڰ����޸�
extern Uint16 cnt_min_h=0;      //��ʱ���ּ�����λ
extern Uint16 cnt_min_l=9;      //��ʱ���ּ�����λ
extern Uint16 cnt_hour_l=1;     //��ʱ��ʱ������λ

Uint16 ledred=0x01;				//led��ֵ 
Uint16 LED[8]={0};				//*��ʾbuf
extern Uint16 state=1;			//*��ǰģʽ

unsigned long int i = 0;		//*�������

int main(void) {
		InitSysCtrl(); 					//ϵͳ��ʼ��
		EALLOW;
		SysCtrlRegs.HISPCP.all = 0x3;	//��������ʱ��HSPCLK=SYSCLKOUT/6=25Mhz
		EDIS;
		DINT;
		spi_intial();                   //SPI��ʼ��
		InitCputimer();					//CPU_timer0��ʼ��
		EALLOW;
		PieVectTable.TINT0= & INT_1_7;	//�ж�������ֵ
		asm("  and IFR,#00H");
		asm("  or  IER,#01H");
		EINT;							//�����ж�
		EDIS;
		while (1);
	}

void InitCputimer(void)
{	EALLOW;
 	CpuTimer0Regs.TPR.all = 149;
 	CpuTimer0Regs.TPRH.all= 0;
 	CpuTimer0Regs.PRD.all = 1999;
 	CpuTimer0Regs.TCR.all =0xf000;
 	PieCtrlRegs.PIEIFR1.all=0x0000;
 	PieCtrlRegs.PIEIER1.all=0x0040;
 	PieCtrlRegs.PIECRTL.bit.ENPIE=0x1;
 	PieCtrlRegs.PIEACK.all =0x1;
 	EDIS;
}


interrupt void INT_1_7(void)			//�жϷ������
{
		Timer();

        switch(state){	
		 case 1:{
			 LED[7]=2;
			 LED[6]=0;
			 LED[5]=17;
			 LED[4]=cnt_hour_l;
			 LED[3]=cnt_min_h;
			 LED[2]=cnt_min_l;
			 LED[1]=cnt_second_l;
			 LED[0]=17;
				}break;
		 }
		LEDdisplay(LED);
   
  PieCtrlRegs.PIEACK.all =0x1;
  CpuTimer0Regs.TCR.all  =0xf000;
}

void Timer(void)
{
	cnt_2ms++;
	if(cnt_2ms>=250){
	cnt_2ms=0;
	cnt_500ms++;
	if(cnt_500ms>=2){
		cnt_500ms=0;
		cnt_second_l++;
		if(cnt_second_l>=5){
			cnt_second_l=0;
			   cnt_min_l++;
			   if(cnt_min_l>=10){
				   cnt_min_l=0;
				   cnt_min_h++;
				   }else if(cnt_min_l == 1 && cnt_min_h == 1){
				   cnt_min_l=0;
				   cnt_min_h=0;
				   cnt_hour_l++;
				  if(cnt_hour_l>=2){
					cnt_hour_l=0;

						  }
					   }
				   }
			   }
			}
		
	}

void spi_intial()
 {
 	SpiaRegs.SPICCR.all =0x0047;   // ʹSPI���ڸ�λ��ʽ, �½���, ��λ����
	SpiaRegs.SPICTL.all =0x0006;   // ����ģʽ, һ��ʱ��ģʽ,ʹ��talk, �ر�SPI�ж�.
	SpiaRegs.SPIBRR =0x007F;       //���ò�����
	SpiaRegs.SPICCR.all =SpiaRegs.SPICCR.all|0x0080;  // �˳���λ״̬
	EALLOW;
    GpioMuxRegs.GPFMUX.all|=0x000F;	// ����ͨ������ΪSPI����

	GpioMuxRegs.GPAMUX.bit.TDIRA_GPIOA11=0; //GPIOA11����Ϊһ��I/O��
	GpioMuxRegs.GPADIR.bit.GPIOA11=1;		//��GPIOA11����Ϊ���
    EDIS;
    GpioDataRegs.GPADAT.bit.GPIOA11=0; 	//GPIOA11=0;�ö˿�Ϊ74HC595�����ź�
  }



//ͨ��SPI��LED������ʾ����
void Write_LED (Uint16 LEDReg)
{
Uint16 LEDcode[30]={0xc000,0xf900,0xA400,0xB000,0x9900,0x9200,0x8200,0xF800,
                    0x8000,0x9000,0x8800,0x8300,0xc600,0xa100,0x8600,0x8e00,
                    0x8c00,0xbf00,0xa700,0xff00,0x4000,0x7900,0x2400,0x3000,
                    0x1900,0x1200,0x0200,0x7800,0x0000,0xf700};//����������0~f, P������L��"��",0.~9.
	 		SpiaRegs.SPITXBUF =LEDcode[LEDReg]; //�����������
    	 	 while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){}
    		SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF; //�������־
}


/*LED��ʾ�ӳ���*/
void LEDdisplay(Uint16* led)
{
	int i;
    GpioDataRegs.GPADAT.bit.GPIOA11=0; //��LACK�ź�һ���͵�ƽ

	  for(i=7;i>=0;i--)
	Write_LED (led[i]);//��LED���λд����,��ʾLED8

     GpioDataRegs.GPADAT.bit.GPIOA11=1; //��LACK�ź�һ���ߵ�ƽΪ����74HC595

}
/*�ӳ������*/


