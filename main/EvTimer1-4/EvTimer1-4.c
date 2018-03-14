
/*�¼���������ʱ���������жϷ����ӳ����зֱ����ָʾ��DS4-DS7*/
#include "DSP281x_Device.h"
unsigned long int i = 0;
void IOinit(void);
void LedOut(Uint16 led);
void Delay(Uint16  data);
void InitEv(void);

void InitEv(void)
{
    //��ʼ��EVA�Ĵ��� Timer1     
    EvaRegs.GPTCONA.all = 0;      
    EvaRegs.T1PR = 0x8000;      // ���ö�ʱ����
    EvaRegs.T1CMPR = 0x0000;    // �ȽϼĴ���    
    EvaRegs.EVAIMRA.bit.T1PINT = 1;//ʹ��T1PINT�ж�
    EvaRegs.EVAIFRA.bit.T1PINT = 1;//�����־λ  
    EvaRegs.T1CNT = 0x0000; //����������
    EvaRegs.T1CON.all = 0x1742;//������������128��Ƶ��ʹ�ܱȽϣ��򿪶�ʱ��
    // Start EVA ADC Conversion on timer 1 Period interrupt
    EvaRegs.GPTCONA.bit.T1TOADC = 2;

    //��ʼ��EVA�Ĵ��� Timer 2:   
    EvaRegs.GPTCONA.all = 0;    
    EvaRegs.T2PR = 0xFF00;     // ���ö�ʱ����
    EvaRegs.T2CMPR = 0x0000;    // �ȽϼĴ���
    EvaRegs.EVAIMRB.bit.T2PINT = 1;	//ʹ��T2PINT�ж�
    EvaRegs.EVAIFRB.bit.T2PINT = 1;	//�����־λ    
    EvaRegs.T2CNT = 0x0000; 	// ����������
    EvaRegs.T2CON.all = 0x1742;	//������������128��Ƶ��ʹ�ܱȽϣ��򿪶�ʱ��
    // Start EVA ADC Conversion on timer 2 Period interrupt
    EvaRegs.GPTCONA.bit.T2TOADC = 2;

 //��ʼ��EVA�Ĵ��� Timer 3:
    EvbRegs.GPTCONB.all = 0; 
    EvbRegs.T3PR = 0x7000;       // ���ö�ʱ����
    EvbRegs.T3CMPR = 0x0000;     // �ȽϼĴ���   
    EvbRegs.EVBIMRA.bit.T3PINT = 1;	//ʹ��T3PINT�ж�
    EvbRegs.EVBIFRA.bit.T3PINT = 1;	//�����־λ   
    EvbRegs.T3CNT = 0x0000;	// ����������
    EvbRegs.T3CON.all = 0x1742;	//������������128��Ƶ��ʹ�ܱȽϣ��򿪶�ʱ��
    // Start EVA ADC Conversion on timer 3 Period interrupt
    EvbRegs.GPTCONB.bit.T3TOADC = 2;

 //��ʼ��EVA�Ĵ��� Timer4:
    EvbRegs.GPTCONB.all = 0; 
    EvbRegs.T4PR = 0xF000;    // ���ö�ʱ����
    EvbRegs.T4CMPR = 0x0000;   //�ȽϼĴ���
    EvbRegs.EVBIMRB.bit.T4PINT = 1;	//ʹ��T4PINT�ж�
    EvbRegs.EVBIFRB.bit.T4PINT = 1;	//�����־λ   
    EvbRegs.T4CNT = 0x0000;	//����������
    EvbRegs.T4CON.all = 0x1742;	//������������128��Ƶ��ʹ�ܱȽϣ��򿪶�ʱ��
    // Start EVA ADC Conversion on timer 4 Period interrupt
    EvbRegs.GPTCONB.bit.T4TOADC = 2;
}
	
void InitGpio(void)  //IO��ʼ��
{
    EALLOW;
    //��GPIOE0~GPIOE2����Ϊһ��I/O�����,��138����  
 	GpioMuxRegs.GPEMUX.all = GpioMuxRegs.GPEMUX.all&0xfff8; 
	GpioMuxRegs.GPEDIR.all = GpioMuxRegs.GPEDIR.all|0x0007;
    //��GPIOB8~GPIOB15����Ϊһ��I/O��,D0~D7
	GpioMuxRegs.GPBMUX.all = GpioMuxRegs.GPBMUX.all&0x00ff;
	 //��GPIOB8~GPIOB15����Ϊ���,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all|0xff00; 
	EDIS;
}

interrupt void eva_timer1_isr(void);	//�ж��ӳ��������
interrupt void eva_timer2_isr(void);
interrupt void evb_timer3_isr(void);
interrupt void evb_timer4_isr(void);

// Global counts used in this example
Uint32	EvaTimer1InterruptCount;
Uint32  EvaTimer2InterruptCount;
Uint32	EvbTimer3InterruptCount;
Uint32  EvbTimer4InterruptCount;

void main(void)
{
	InitSysCtrl();	/*��ʼ��ϵͳ*/	
	DINT;	/*���ж�*/
	IER = 0x0000;
	IFR = 0x0000;	
	InitPieCtrl();	/*��ʼ��PIE*/
	InitPieVectTable();	/*��ʼ��PIE�ж�ʸ����*/	
    InitEv();	/*��ʼ���¼�������*/  
	InitGpio(); /*��ʼ��IO�˿�*/ 
	InitXIntrupt();
	LedOut(0);	 // 16��LED��Ϩ��	
	
	EALLOW;	
	PieVectTable.T1PINT = &eva_timer1_isr;	//PIE�ж�ʸ����
	PieVectTable.T2PINT = &eva_timer2_isr;
	PieVectTable.T3PINT = &evb_timer3_isr;
	PieVectTable.T4PINT = &evb_timer4_isr;
	EDIS; 
    
    PieCtrlRegs.PIEIER2.all = M_INT4;	//ʹ��T1PINT    
    PieCtrlRegs.PIEIER3.all = M_INT1;   //ʹ��T2PINT    
    PieCtrlRegs.PIEIER4.all = M_INT4;	//ʹ��T3PINT    
    PieCtrlRegs.PIEIER5.all = M_INT1;	//ʹ��T4PINT	
    
	IER |= (M_INT2 | M_INT3 | M_INT4 | M_INT5);	//ʹ��INT2��INT3��INT4��INT5   
	
	EINT;   //����ȫ���ж�
	ERTM;	// Enable Global realtime interrupt DBGM		
	for(;;);

} 	

/*�жϷ����ӳ���*/

interrupt void eva_timer1_isr(void)
{
	EvaTimer1InterruptCount++;	
 	LedOut(1);	//ָʾ��DS4��IOOUT0����
    // Enable more interrupts from this timer
	EvaRegs.EVAIMRA.bit.T1PINT = 1;
    EvaRegs.EVAIFRA.all = BIT7;	//�жϱ�־��
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;	//�����´��ж�
}


interrupt void eva_timer2_isr(void)
{
	EvaTimer2InterruptCount++;
	 LedOut(2);	//ָʾ��DS5��IOOUT1����	
    // Enable more interrupts from this timer
	EvaRegs.EVAIMRB.bit.T2PINT = 1;
    EvaRegs.EVAIFRB.all = BIT0;	//�жϱ�־��
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3	;//�����´��ж�
}

interrupt void evb_timer3_isr(void)
{
	EvbTimer3InterruptCount++;
	LedOut(4);	//ָʾ��DS6��IOOUT2����		
    EvbRegs.EVBIFRA.all = BIT7;	//�жϱ�־��
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;//�����´��ж�
}

interrupt void evb_timer4_isr(void)
{
	EvbTimer4InterruptCount++;
	LedOut(8);	//ָʾ��DS7��IOOUT3����
    EvbRegs.EVBIFRB.all = BIT0;	//�жϱ�־��
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP5;//�����´��ж�
}


void	Delay(Uint16  data)
{
	Uint16	i;
	for (i=0;i<data;i++) { ; }	
}	

void LedOut(Uint16 led)
{
 
	GpioDataRegs.GPEDAT.all = 0xfffb;   //LEDB��0
	GpioDataRegs.GPBDAT.all = ~led;
	for (i=0; i<100; i++){}             //��ʱ
    GpioDataRegs.GPEDAT.all = 0xffff;   //�����8λ	
	GpioDataRegs.GPEDAT.all = 0xfffa;   //LEDA��0
	GpioDataRegs.GPBDAT.all = ~(led<<8);
	for (i=0; i<100; i++){}
    GpioDataRegs.GPEDAT.all = 0xffff;   //�����8λ			
}
//===========================================================================
// No more.
//===========================================================================

