/*******************************************************************************************
�ļ�:   ECanSelf.c
����:	�����Լ췽ʽ����DSP281x eCAN ���ݵĻ��ͺͽ��ա�		
		�����Գ���ͣ�ظ��ٻ������ݣ��Խ��յ������ݽ���У�飬�д����ʶ��
		MBX0���͵�MBX16, MBX1���͵�MBX17 ....  ����������Լ췽ʽ��Ӧ�á�
		����������led�����������ʾ(DS20��DS25)�Ľӿڡ��Ա㷴ӳeCANͨ�ŵĴ���
*******************************************************************************************/

#include "DSP281x_Device.h"     // DSP281x ͷ�ļ������ļ�
#include "DSP281x_Examples.h"   // DSP281x ʾ�������ļ�

	// ���ļ������ĺ���ԭ������
void mailbox_check(int32 T1, int32 T2, int32 T3);	
	// ���������յ�����
void mailbox_read(int16 i); 						
	// �����ɲ���MBXnbrָ������������ݡ�
void WriteXram(Uint16 *StartAddr, Uint16 *EndAddr, Uint16 number);
	// ��ָ��������ַStartAddr��ʼ��EndAddr������˳��д��number���ݡ�
void WriteMDLH(Uint16 *StartAddr, Uint16 *EndAddr);
	// �Ը��ټ�����TrackcountΪ�������ӵ�һ���������俪ʼ��Trackcount��1����������
	// ��������ĸ�����Ԫ(�ܹ�64���ֵ�Ԫ)��
void CompMDLH(Uint16 *Comp1, Uint16 *Comp2);
	// ��eCAN���͵������£���ǰ16������������������16���������������һһ�Ƚϣ�
	// �������ɴ��������ErrorCount�ۼƼ�����
void delay(Uint16 dly);

	// ������ȫ�ֱ���
Uint32  ErrorCount=0;			// ���������
Uint32  MessageReceivedCount;	// ���Ľ��ռ�����

Uint32  TestMbox1 = 0;
Uint32  TestMbox2 = 0;
Uint32  TestMbox3 = 0;

Uint32	Trackcount=0;			// �������䶯̬���ݸ��¸��ټ�����������Trackcount��1
								// ������ʽ�����δ���16���������乲64���֡���һ�η�
								// ���������ݸ��£���ǰһ����̬��TrackcountΪ������

void main(void)
{
		//	eCAN���ƼĴ�������32λ���ݽ��ж�д���ʣ�����ڱ����й�����һ��Ӱ�ӼĴ�����
		//	����Ӱ�ӼĴ�������32λ���ݷ��ʷ�ʽ����16λ��ʽ��
	struct ECAN_REGS ECanaShadow; 	
					// ����ECanaShadowΪ����ECAN_REGS��ͬ���͵Ľṹ�����.

		//  ϵͳ���Ƴ�ʼ��:
   	InitSysCtrl();	// ���໷(PLL),���Ź�(WatchDog)������ʱ��(PeripheralClocks)��ʼ����
					// InitSysCtrl()������DSP281x_SysCtrl.c�ļ�������
  	EALLOW;			// ��������ܱ����Ŀռ䡣

		//	���������GPIO��·����������CAN����
   	EALLOW;
   	GpioMuxRegs.GPFMUX.bit.CANTXA_GPIOF6 = 1;		
   					// ��CANTXA_GPIOF6=0����GPIOF6���ó���ͨI/O ��
					// ��CANTXA_GPIOF6=1����GPIOF6���ó�CANTXA����
   	GpioMuxRegs.GPFMUX.bit.CANRXA_GPIOF7 = 1;
					// ��GPIOF7���ó�CANRXA����
   	EDIS;
	//��ʼ��GPIO:
	EALLOW;	// ��������ܱ����ļĴ���
		//��GPIOF8~GPIOF13����Ϊһ��I/O�ڣ���������������������ʾ(DS20��DS25)
 	GpioMuxRegs.GPFMUX.all = 0xc0ff;
    GpioMuxRegs.GPFDIR.all = 0x3f00; 		 
	EDIS;								// ��ֹ�����ܱ����ļĴ���
    GpioDataRegs.GPFDAT.all=0x5555;		// ���һ������LED��������ܡ�
		
   	DINT;	//	��CPU�ж� 
 
	IER = 0x0000;
	IFR = 0x0000;	// ��CPU�жϲ�������CPU�жϱ�ʶ���� 
	
    MessageReceivedCount = 0;
    ErrorCount = 0;

/*******************************************************************************************
	�����eCAN����RAM����0x6100-0x61FF���ó�ֵ0��������Ϊ��Ͻ������丳ֵ���顣�μ�
����������丳ֵ�����˵����
	ʵ��"(Uint16 *)0x6100" ΪWriteXram�����β�"Uint16 *StartAddr" ȷ����ʼ��ַָ��
	ʵ��"(Uint16 *)0x6200" ΪWriteXram�����β�"Uint16 *EndAddr" ȷ��������ַָ��
*******************************************************************************************/
	WriteXram((Uint16 *)0x6100,(Uint16 *)0x6200,0x000000);

    
/*******************************************************************************************
32λ���ʷ�������:

		eCAN���ƺ�״̬�Ĵ�������32λ���ݽ��ж�д���ʣ������Ҫд������һλ��λ����ͨ
	����������������:

		(1) �Ƚ���һ������32λECAN_REGS�ṹ�����͵�Ӱ�ӼĴ�������ECanaShadow(�����ļ�
			�ĵ�һ��ָ�� "struct ECAN_REGS ECanaShadow;" )��������DSP281x_ECan.hͷ�ļ�
			���Ѿ���ָ�� "extern volatile struct ECAN_REGS ECanaRegs;",�ʳ�֮ΪӰ�Ӽ�
			������
			�����Ҫд������һλ�������CAN����IO���ƼĴ��� CANTIOC �� TXFUNC λ��1��
			��ͨ������3����ɡ�
		(2) ��32λ��ʽ�������Ĵ�������Ӱ�ӼĴ���:
				ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all;
		(3) �ı�Ӱ�ӼĴ�����Ҫ������λ:
    			ECanaShadow.CANTIOC.bit.TXFUNC = 1;
		(4) ��Ӱ�ӼĴ����ؿ�����32λд��eCAN�Ĵ�����
			    ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all;

		�����8��ָ������eCAN��RX��TX����ΪeCAN���ͷ�ʽ
*******************************************************************************************/ 
    EALLOW;
    ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all;	// CANTIOC: CAN����IO���ƼĴ���
    ECanaShadow.CANTIOC.bit.TXFUNC = 1;					// CANTX��������CAN���Ͳ���	 (P590)
    ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all;

    ECanaShadow.CANRIOC.all = ECanaRegs.CANRIOC.all;	// CANRIOC: CAN����IO���ƼĴ���
    ECanaShadow.CANRIOC.bit.RXFUNC = 1;					// CANRX��������CAN���ղ���
    ECanaRegs.CANRIOC.all = ECanaShadow.CANRIOC.all;
    EDIS;
     
    ECanaRegs.CANME.all = 0;							// CANME:����ʹ�ܼĴ�����	 (P571)
    		// ���������䡣������д�������Ĵ���(������λ��)���ʲ���ҪӰ�ӼĴ�����

/*******************************************************************************************	
����������Ϣ��ʶ���Ĵ���(MSGID)��ֵ˵����

	(1) ����ͨ�������ָ��"ECanaShadow.CANMC.bit.SCB = 1;"����ΪeCAN��ʽ��ʹ��32�����䡣
		��ͨ��ָ��"ECanaRegs.CANMD.all = 0xFFFF0000;"��ǰ16����������Ϊ���(���λ��Ӧ)��
		��16����������Ϊ����(���λ��Ӧ)��
	(2) һ�����õķ�������ı�ʶ���ĸ�ֵ������ĳһ����������ı�ʶ���ĸ�ֵ���(ƥ��)����
		��ͨ��ʱ�����������ܹ��������硣��˿ɰѱ�ʶ������ͨ�������롣
	(3) MSGID��һ��32λ�ļĴ����������λMSGID.31��ʼ��˳��3λ�ֱ��ǣ���ʶ����չλ IDE,
		��������ʹ��λ AME ���Զ�Ӧ��ģʽλAAM��MSGID.28-MSGID.0 Ϊ���ͨ�����������Ϣ
		��ʶ��λ��
	(4) ��ʶ���б�׼����չ����ģʽ����׼ռ��MSGID.28-18����չռ��MSGID.28-0����
		IDE=0ʱ��ѡ�ñ�׼ģʽ����IDE=1ʱ��ѡ����չģʽ������Ϊ��չģʽ��
	(5) ��������ʹ��λ AME ֻ���ڽ����䣬��AME=1ʱ��ȫ�ֽ������μĴ���(CANGAM)����ʹ�á�
		CANGAM.28-0�Ƕ�Ӧ��MSGID.28-0��ȫ�ֽ�������λ����ĳλ��1�����Ӧ����MSGID.28-0��
		ĳλ������ȡAME=0�������ý������Ρ�
	(6) �Զ�Ӧ��ģʽλ AAM ����������Ϊ���Ͳ�������Ϣ������Ч�����ڽ������䣬��λ��Ч��
		�� AAM=1ʱ��Ϊ�����Զ�Ӧ��ģʽ����ʱ�������Ͷ��յ�һ��ƥ���Զ������CANģ��ͨ
		�����͸������������Ӧ��Զ�����󡣵� AAM=0ʱ��Ϊ��������ģʽ�����Ͷ����䲻��Զ��
		�������Ӧ�𡣱���ȡAAM=0������Ϊ��������ģʽ��
	(7) ��������ռ���ڴ�����:0x006100-0x00617F
*******************************************************************************************/
    ECanaMboxes.MBOX0.MSGID.all = 0x9555AAA0; 							//			(P595)
    ECanaMboxes.MBOX1.MSGID.all = 0x9555AAA1; 
    ECanaMboxes.MBOX2.MSGID.all = 0x9555AAA2; 
    ECanaMboxes.MBOX3.MSGID.all = 0x9555AAA3; 
    ECanaMboxes.MBOX4.MSGID.all = 0x9555AAA4; 
    ECanaMboxes.MBOX5.MSGID.all = 0x9555AAA5; 
    ECanaMboxes.MBOX6.MSGID.all = 0x9555AAA6; 
    ECanaMboxes.MBOX7.MSGID.all = 0x9555AAA7; 
    ECanaMboxes.MBOX8.MSGID.all = 0x9555AAA8; 
    ECanaMboxes.MBOX9.MSGID.all = 0x9555AAA9; 
    ECanaMboxes.MBOX10.MSGID.all = 0x9555AAAA; 
    ECanaMboxes.MBOX11.MSGID.all = 0x9555AAAB; 
    ECanaMboxes.MBOX12.MSGID.all = 0x9555AAAC; 
    ECanaMboxes.MBOX13.MSGID.all = 0x9555AAAD; 
    ECanaMboxes.MBOX14.MSGID.all = 0x9555AAAE; 
    ECanaMboxes.MBOX15.MSGID.all = 0x9555AAAF; 
    
/*******************************************************************************************
����������Ϣ��ʶ���Ĵ���(MSGID)��ֵ˵����

	(1) Ϊ�˱������ݹ۲�,��������˽���������Ϣ�뷢��������Ϣ���¶�Ӧ��ȵ�������
	(2) һ����Ч�Ľ���������Ϣ��ʶ���Ĵ���MSGID���洢����Ϣ��������ĳһ������������Ϣ
	    ��ʶ���Ĵ�������Ϣ��ȡ������������������һ֡����(������ʶ���Ĵ���MSGID���洢
		����Ϣ)������֮�󣬽��ն˽����յ���ÿһ�������������Ϣ��ʶ�����������������
		Ϣ��ʶ�����ݽ��бȽϣ������ȣ�����յ��ı�ʶ��������λ�������ֽ�д���Ӧ�Ľ�
		������,��ĳһRAM����������ȣ�����һ֡���ݶ���������洢��
	(3) ������һ���������䣬����Ϣ��ʶ�����������������������Ϣ��ʶ��������û��һ����
		�ȣ�������������佫�ղ����κ����ݡ� 
	(4) ����������Ϣ��ʶ���Ĵ���MSGID�ĳ�ʼ��û�д����ϵ�Ҫ��
	(5) ��������ռ���ڴ�����:0x006180-0x0061FF

		�����16��ָ���16���������䣨MBOX16 - 31������Ϣ��ʶ���Ĵ���MSGID��ֵ��Ϊ�˶���
	��ĸ����и�ֱ�۵���⣬��������2�����飺

	(1) ���������뷢�������ƥ�����飺
		��MBOX16����Ϣ��Ϊ0x9555AAAF, MBOX31����Ϣ��Ϊ0x9555AAA0.�������г���֮����ͣ����
		������RAM����(View->Memory->0x00006100)�����Է���ԭ�������17������MBOX16����32��
		������������˵�����
	(2) ���������뷢������Ĳ�ƥ�����飺
		�����ɽ����������Ϣ���ó��뷢���������Ϣ��ƥ�䣬�������г���֮����ͣ�����Է�����
		Щ���䱣��ǰ��ĳ�ʼ�����ݡ�
*******************************************************************************************/
    ECanaMboxes.MBOX16.MSGID.all = 0x9555AAA0; 
    ECanaMboxes.MBOX17.MSGID.all = 0x9555AAA1; 
    ECanaMboxes.MBOX18.MSGID.all = 0x9555AAA2; 
    ECanaMboxes.MBOX19.MSGID.all = 0x9555AAA3; 
    ECanaMboxes.MBOX20.MSGID.all = 0x9555AAA4; 
    ECanaMboxes.MBOX21.MSGID.all = 0x9555AAA5; 
    ECanaMboxes.MBOX22.MSGID.all = 0x9555AAA6; 
    ECanaMboxes.MBOX23.MSGID.all = 0x9555AAA7; 
    ECanaMboxes.MBOX24.MSGID.all = 0x9555AAA8; 
    ECanaMboxes.MBOX25.MSGID.all = 0x9555AAA9; 
    ECanaMboxes.MBOX26.MSGID.all = 0x9555AAAA; 
    ECanaMboxes.MBOX27.MSGID.all = 0x9555AAAB; 
    ECanaMboxes.MBOX28.MSGID.all = 0x9555AAAC; 
    ECanaMboxes.MBOX29.MSGID.all = 0x9555AAAD; 
    ECanaMboxes.MBOX30.MSGID.all = 0x9555AAAE; 
    ECanaMboxes.MBOX31.MSGID.all = 0x9555AAAF; 

	//ECanaRegs.CANMD.all = 0x00000000;				// ��32���������óɷ�������
    ECanaRegs.CANMD.all = 0xFFFF0000; 			// CANMD: ����ָ��Ĵ�����			(P571)		
			// ��CANMD.x=1����Ӧ���䶨��Ϊ�������䣬��CANMD.x=0����Ӧ���䶨��Ϊ�������䡣
			// ����0-15����Ϊ���ͣ�16-31����Ϊ���ա�
    
    ECanaRegs.CANME.all = 0xFFFFFFFF;	// CANME: ����ʹ�ܼĴ���					(P571)	
    		// ������ʹ�ܺ���Ӧ����ı�ʶ��д���������ֹ����˱�ʶ��д�����
    		// ������ʹ��֮ǰ	

/*******************************************************************************************
��Ϣ���ƼĴ���(MSGCTRL)ʹ��˵����

	(1) ������32λ��Ϣ���ƼĴ���(MSGCTRL)��3����;�������ֽ������������ȼ���Զ��֡����
	(2) MSGCTRL.12-MSGCTRL.8Ϊ�������ȼ�λTPL4:0��
		��4λ��λֵ���Ӵ�С�����˸����䷢�͵����ȼ��������ȼ���ͬʱ�����нϴ���ŵ�����
		�Ƚ��з��Ͳ�����TPLֻ���ڷ������䣬���Ҳ���SCC(16����)ģʽ��ʹ�á�����û�ж���4λ
		�������ã�����������ã���Ӱ��16������Ϣ�ķ��͡�
	(3) MSGCTRL.3-MSGCTRL.0Ϊ���ݳ��ȴ������λDLC3:0��
		���������з��ͻ���յ������ֽ��������ֻ������8���ֽڡ���������8���ֽڡ�
	(4) MSGCTRL.4ΪԶ�˷�������λ RTR��
		��RTR=1ʱ�����ڽ������䣬���TRS��־����λ����ᷢ��һ��Զ��֡������ͬһ���������
		��Ӧ������֡��һ��Զ��֡�����ͳ�ȥ�������TRSλ�ͻر�CANģ����0�����ڷ������䣬���
		TRS��־����λ����ᷢ��һ��Զ��֡�����ǻ�����һ�����������Ӧ������֡������ϵͳ��λ
		ʱ��RTR Ϊһ���״̬��������û��Զ��֡���󣬹�����RTR=0��
*******************************************************************************************/
    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;				// �������ݳ���Ϊ8���ֽڣ�
    ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;				// ���ֻ��Ϊ8���ֽڡ�
    ECanaMboxes.MBOX2.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX3.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX4.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX5.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX6.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX7.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX8.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX9.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX10.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX11.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX12.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX13.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX14.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX15.MSGCTRL.bit.DLC = 8;
    
		
    ECanaMboxes.MBOX0.MSGCTRL.bit.RTR = 0;  			// ������Զ��֡����			   
    ECanaMboxes.MBOX1.MSGCTRL.bit.RTR = 0;  
    ECanaMboxes.MBOX2.MSGCTRL.bit.RTR = 0;  
    ECanaMboxes.MBOX3.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX4.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX5.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX6.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX7.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX8.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX9.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX10.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX11.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX12.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX13.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX14.MSGCTRL.bit.RTR = 0;
    ECanaMboxes.MBOX15.MSGCTRL.bit.RTR = 0;
    
/*	��WriteMDLH()��̬���ݴ���
    // Write to the mailbox RAM field of MBOX0 - 15
    ECanaMboxes.MBOX0.MDL.all = 0x9555AAA0;
    ECanaMboxes.MBOX0.MDH.all = 0x89ABCDEF;

    ECanaMboxes.MBOX1.MDL.all = 0x9555AAA1;
    ECanaMboxes.MBOX1.MDH.all = 0x89ABCDEF;
   
    ECanaMboxes.MBOX2.MDL.all = 0x9555AAA2;
    ECanaMboxes.MBOX2.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX3.MDL.all = 0x9555AAA3;
    ECanaMboxes.MBOX3.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX4.MDL.all = 0x9555AAA4;
    ECanaMboxes.MBOX4.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX5.MDL.all = 0x9555AAA5;
    ECanaMboxes.MBOX5.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX6.MDL.all = 0x9555AAA6;
    ECanaMboxes.MBOX6.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX7.MDL.all = 0x9555AAA7;
    ECanaMboxes.MBOX7.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX8.MDL.all = 0x9555AAA8;
    ECanaMboxes.MBOX8.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX9.MDL.all = 0x9555AAA9;
    ECanaMboxes.MBOX9.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX10.MDL.all = 0x9555AAAA;
    ECanaMboxes.MBOX10.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX11.MDL.all = 0x9555AAAB;
    ECanaMboxes.MBOX11.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX12.MDL.all = 0x9555AAAC;
    ECanaMboxes.MBOX12.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX13.MDL.all = 0x9555AAAD;
    ECanaMboxes.MBOX13.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX14.MDL.all = 0x9555AAAE;
    ECanaMboxes.MBOX14.MDH.all = 0x89ABCDEF;
 
    ECanaMboxes.MBOX15.MDL.all = 0x9555AAAF;
    ECanaMboxes.MBOX15.MDH.all = 0x89ABCDEF;
*/

    EALLOW;
    ECanaRegs.CANMIM.all = 0xFFFFFFFF;		// CANMIM�������ж����μĴ�����			(P589)			
			// �����ж�ʹ�ܡ�
			// ������д�������Ĵ���(������λ��)���ʲ���ҪӰ�ӼĴ�����							  

 			// Ҫ������ı����üĴ���:
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;   // CANMC,�����ƼĴ���			(P578)
    ECanaShadow.CANMC.bit.CCR = 1;            
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
    EDIS;
    
			// ͨ���Ⱥ�CCE����λ���ȴ�CPU׼��������üĴ�����ֵ
    do 
    {
      ECanaShadow.CANES.all = ECanaRegs.CANES.all;	// CANES�������״̬�Ĵ���		(P582)
    } while(ECanaShadow.CANES.bit.CCE != 1 );  
			// ��CPU�����üĴ���������д����ʱ��CCE=1������CCE=0������3��ָ���
			// ����Ϊ����CCE=1ʱ��ִ�������ָ��,����ȴ�CPU�������üĴ�����ֵ��    
    	
    		/******* Configure the eCAN timing  ����eCANʱ�ӡ�********/
    EALLOW;
    ECanaShadow.CANBTC.all = ECanaRegs.CANBTC.all;	// CANBTC: λ��ʱ���üĴ���		(P580)
    ECanaShadow.CANBTC.bit.BRPREG = 9;
			// ������Ԥ��������BRPREG���� CAN ʱ��Ƶ�������¹�ϵ��
			// 		CAN clock = SYSCLKOUT/(BRPREG + 1)
			// CAN clock �ĵ���Ϊʱ������TQ������(����)�����ϵΪ��
			// 		TQ = ��1/SYSCLKOUT��*BRP  
			// ���� SYSCLKOUT Ϊ CPU ʱ�ӣ�BRP=BRPREG+1
			// �� SYSCLKOUT=150MHz, BRPREG=9ʱ��CAN ʱ��Ƶ��=150/10=15 MHz.
    ECanaShadow.CANBTC.bit.TSEG2REG = 5 ;
    ECanaShadow.CANBTC.bit.TSEG1REG = 7; 
			// CAN ����������ʽȷ����
			// �����ʣ�bit rate��= SYSCLKOUT/(BRP*(bit-time))
			// ���� SYSCLKOUT Ϊ CPU ʱ�ӣ�BRP=BRPREG+1��bit-time=(TSEG1REG+1)+(TSEG2REG+1)+1
			// �� SYSCLKOUT=150MHz, BRPREG=9, TSEG1REG = 7, TSEG2REG = 5 ʱ��
			// CAN ������=150/(10*15)=1 MHz.										(P601)
    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;

			// ����	EALLOW �µ�ָ�������λ��ʱ���üĴ������á�����3��ָ����CPU��
			// ���任��������	
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.CCR = 0;            
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

    EDIS;

			// ����3��ָ��Ϊ�ȴ�CPU��λ��ʱ���üĴ�����CANBTC�����õĸ��ġ�
    do 
    {
      ECanaShadow.CANES.all = ECanaRegs.CANES.all;	// CANES�������״̬�Ĵ���		(P582)
    } while(ECanaShadow.CANES.bit.CCE != 1 );  
			// ��CPU�����üĴ���������д����ʱ��CCE=1������CCE=0������3��ָ���
			// ����Ϊ: ��CCE=0ʱ���ȴ�CPU�������üĴ�����ֵ;
			//         ��CCE=1ʱ��ִ�������ָ�   


			// ���� eCAN Ϊ�Լ�ģʽ��ʹ�� eCAN ����ǿ���ܡ�
    EALLOW;
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;			// CANMC �����ƼĴ���
/****************************************************************************************
ע�⣺	���� eCAN �Լ�ģʽ֮��ʾ�����ɹ۲쵽������Σ����ǳ���������
		while(ECanaRegs.CANTA.all != 0x0000FFFF ) {}  ָ�̤��������������ָ����
		ʾ�����۲첻��������Ρ�

		����ʵ��ʹ������������ж�:
	(1)	��STM=1�������� eCAN Ϊ�Լ�ģʽ�������£�eCAN���ζ��ⷢ���źš���ʱ��eCAN�ķ���
		�ͽ�����ϵͳ�ڲ���ɡ�
	(2)	��STM=0���ڹر��Լ�ģʽ�������£�eCAN���ⷢ���źţ�ʾ�����ɹ۲쵽����������Ρ�
		��ʱ���ڲ���ֹ���գ�CANTA����ʧЧ���ⲿ���ջ��������δ�ʵ��֮���ٿ���
****************************************************************************************/
  	ECanaShadow.CANMC.bit.STM = 1;    	// ���� eCAN Ϊ�Լ�ģʽ 
    ECanaShadow.CANMC.bit.SCB = 1;    	// eCAN ģʽ���������32λ���䣩��

    ECanaRegs.CANMC.all= ECanaShadow.CANMC.all;
    EDIS;
    
			// ��ʼ����
//    while(1)
	for(;;) 	// ����while(1)������ѭ��������ͨ��,������ֱ��뾯�档�ʸ���for(;;)                               
    {  
		WriteMDLH((Uint16 *)0x6104,(Uint16 *)0x617F);	// �ı䷢����������͵����ݡ�
				// ÿ�����������������:������λ������ǰ�벿��4��8�ֽڵ������ʶ����
				// ��λ�������벿��4��8�ֽڵ����ݡ�����ָ����ı�ÿ��������������
				// ���ݡ�
		
       	ECanaRegs.CANTRS.all = 0x0000FFFF;  			// CANTRS������������λ�Ĵ���
				// �����з�����������TRSλ,����TRS[15:0]ȫΪ1��16�����俪ʼ���͡�	(P572)

     	while(ECanaRegs.CANTA.all != 0x0000FFFF ) {}  // Wait for all TAn bits to be set..
				// ��� CANTA ��ĳλ����1�����Ӧ�������Ϣ���ɹ����͡�������ָ��ĺ���Ϊ
				// �ȴ�TA[15:0]����Ϊ1�����ȴ������������Ϣ���ͳ�ȥ��

       	ECanaRegs.CANTA.all = 0x0000FFFF;   
       			// ͨ����λ��TAn��׼���б���һ�η������

       	MessageReceivedCount++;

		CompMDLH((Uint16 *)0x6104,(Uint16 *)0x6184);
				// ��eCAN���͵������£���ǰ16������������������16���������������һһ�Ƚϣ�
		  		// �������ɴ��������ErrorCount�ۼƼ�����

		GpioDataRegs.GPFTOGGLE.all = 0xffff;	// F�˿�ȡ��

		delay(6);								// ��ʱ ,�˴�����̽��

				// �������"CompMDLH((Uint16 *)0x6104,(Uint16 *)0x6184);"
				// ָ���������������ݵ���ȷ�ԡ������������εļ�����
/*
       	for(j=0; j<16; j++)          						// ��ȡ16������������Ϣ���������
       	{
          	mailbox_read(j); 								// ������ʵ��jָ������������ݡ�
          	mailbox_check(TestMbox1,TestMbox2,TestMbox3); 	// ���������յ�����
       	}
*/
    }
}

/*******************************************************************************************
��������: WriteXram(Uint16 *StartAddr, Uint16 *EndAddr, Uint16 number)
��������: ��ָ��������ַStartAddr��ʼ��EndAddr������˳��д��number���ݡ����øú���
		  ʱ������*StartAddr++ =number;ָ��ǰ���öϵ㣬�򿪶�Ӧ��Memory���ڣ���F10����
		  �ɿ��� number ����(����)д���Ӧ��Ԫ��
�������: ��һ�β�StartAddrΪһָ�������Ҫд�����ݵ��׵�ַ��
		  �ڶ��β�EndAddrΪһָ�������Ҫд�����ݵĽ�����ַ��
		  �����β�numberΪд������ݡ�
*******************************************************************************************/
void WriteXram(Uint16 *StartAddr, Uint16 *EndAddr, Uint16 number)
{
	while(StartAddr < EndAddr)
    { 
       	*StartAddr++ =number;
       	//*StartAddr++ =number++;
    }
    return;
}

/*******************************************************************************************
��������: WriteMDLH(Uint16 *StartAddr, Uint16 *EndAddr)
��������: �Ը��ټ�����TrackcountΪ�������ӵ�һ���������俪ʼ��Trackcount��1����������
		  ��������ĸ�����Ԫ(�ܹ�64���ֵ�Ԫ)��
�������: ��һ�β�Ϊһָ�����StartAddr��ʵ�ε�ַָ��ָ�����ݷ�������һ���������䡣
		  �ڶ��β�Ϊһָ�����EndAddr��ʵ�ε�ַָ��ָ�����ݽӷ��������һ���������䡣
*******************************************************************************************/
void WriteMDLH(Uint16 *StartAddr, Uint16 *EndAddr)
{
	Uint16 i;
	while(StartAddr < EndAddr)
    { 
       	for(i=0;i<4;i++) {*StartAddr++ =Trackcount++;}		// ÿ������4���֡�
		StartAddr=StartAddr+4;								// ָ����һ�����������׵�ַ��
    }
    return;
}

/*******************************************************************************************
��������: CompMDLH(Uint16 *Comp1, Uint16 *Comp2)
��������: ��eCAN���͵������£���ǰ16������������������16���������������һһ�Ƚϣ�
		  �������ɴ��������ErrorCount�ۼƼ�����
�������: ��һ�β�Ϊһָ�����Comp1��ʵ�ε�ַָ��ָ�����ݷ�������
		  �ڶ��β�Ϊһָ�����Comp2��ʵ��ַָ��ָ�����ݽ�������
*******************************************************************************************/
void CompMDLH(Uint16 *Comp1, Uint16 *Comp2)
{
	Uint16 i,j;
	for(i=0;i<16;i++)		// ��ѭ����ȡ16������
	{
		for(j=0;j<4;j++)	// Сѭ����ÿ�麬4���֣�8���ֽڡ�
		{	
			if(*Comp1++ != *Comp2++)  { ErrorCount++;}		
							// ����Ȥ�ɲ���һ����ȣ��򿪹۲촰����ErrorCount���й۲졣
		}
		Comp1=Comp1+4;		// ȷ����һ����ʼ��ַ
		Comp2=Comp2+4;
	}
}

/****************************************************************************************
��������: 	mailbox_read(int16 MBXnbr)
��������: 	�����ɲ���MBXnbrָ������������ݡ�
�����β�: 	MBXnbr ��ʾѡ�������š�

ע	  ��:	������������ΪTIԭ�亯�����������ڵ��ļ��ڷ����������ֵ�ϲ�ȡ�Ƚ�����
			�ʵĶ�̬���ݣ������ȡ�˶�ȫ��ͨ�����ݽ��м��ķ�������ˣ���������
			�����������ǣ�mailbox_read�����Ĺ�˼��ֵ��ѧϰ�ġ�����һ������ָ�����
			�ļ������ķ�����			
****************************************************************************************/
void mailbox_read(int16 MBXnbr)
{
	volatile struct MBOX *Mailbox;	
   		// MBOX ��һ���ṹ������(��DSP281x_Ecan.h ͷ�ļ�)�����ɱ�������4��32λ�Ĵ���
   		// ���Ե�4���������Ա������ɡ����ﶨ��Mailbox��һ�����MBOX�ṹ�����͵Ľ�
   		// ����ָ�������
   	Mailbox = &ECanaMboxes.MBOX0 + MBXnbr;
		// ָ�����Mailbox ȡ��һ���������ĵ�ַ ECanaMboxes.MBOX0 ��Ϊ��ַ�ټ���
		// ƫ����MBXnbr��
   	TestMbox1 = Mailbox->MDL.all; 	// = 0x9555AAAn (n is the MBX number)
   	TestMbox2 = Mailbox->MDH.all; 	// = 0x89ABCDEF (a constant)
   	TestMbox3 = Mailbox->MSGID.all;	// = 0x9555AAAn (n is the MBX number)

} // MSGID of a rcv MBX is transmitted as the MDL data.

/****************************************************************************************
������: 	mailbox_check(int32 T1, int32 T2, int32 T3)
��������: 	���������յ�����
			��� T1!=T3 ���� T2!=0x89ABCDEF��������������������ݲ��ȣ�ִ��ѭ����:
			�����������1; �������������ѭ���塣
�����β�: 	 
****************************************************************************************/
void mailbox_check(int32 T1, int32 T2, int32 T3)
{
	if((T1 != T3) || ( T2 != 0x89ABCDEF))	
    {
    	
       	ErrorCount++;
       	
    }
}

/*******************************************************************************************
��������: delay(Uint16 dly)
��������: ��ʱ����
�������: �β�dly��dlyԽ����ʱԽ��
�������: ��
*******************************************************************************************/
void delay(Uint16 dly)
{
	Uint32 i;
	for(;dly>0;dly--)
	{
		for(i=0;i<100000;i++);
	}
} 

//=========================================================================================
// No more.
//=========================================================================================
