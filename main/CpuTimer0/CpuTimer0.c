#include "DSP281x_Device.h"
#include "DSP281x_Examples.h"   // DSP281x Examples ͷ�ļ�
#include "math.h"

// �ɱ��ļ������ĺ���ԭ������
void scia_echoback_init(void);				// SCIA�ش���ʼ������
void scia_fifo_init(void);					// SCIA �Ƚ��ȳ�(fifo)��ʼ��
void scia_xmit(int a);						// SCIA���ͺ���
void scia_msg(char *msg);					// ������Ϣ���ͺ���

	// �����õ���ȫ�ּ�����Uint16 LoopCount;
Uint16 LoopCount;
Uint16 ErrorCount;

void main(void)
{
    char *msg;						// ����ָ���������Ϊ�ַ���
// ϵͳ���Ƴ�ʼ��: ���໷(PLL),���Ź�(WatchDog)������ʱ�ӳ�ʼ��	 
   InitSysCtrl();

// ��ʼ��GPIO
   EALLOW;
   GpioMuxRegs.GPFMUX.all=0x0030;	// ѡ��GPIOs SCIA����
   GpioMuxRegs.GPGMUX.all=0x0030;	// ѡ��GPIOs SCIB����
   EDIS;

// �������ж�:
   DINT;
// ��ֹCPU���жϲ��������CPU�жϱ�־��
   IER = 0x0000;
   IFR = 0x0000;
	
// �û��������
    LoopCount = 0;
    ErrorCount = 0;
    scia_fifo_init();	   				// SCIA �Ƚ��ȳ�(fifo)��ʼ��
    scia_echoback_init();  				// SCIA�ش���ʼ������


	msg = "\r\nѧ�ţ�3130102475\0";
	scia_msg(msg);

	msg = "\r\n���ڣ�2015-01-08\0";
	scia_msg(msg);

	msg = "\r\n���ţ���2¥116��17ʵ��̨\0";
	scia_msg(msg);

	msg = "\r\n������һ���ַ���dsp���ش�ѧ�š����ں����� \0";	
    scia_msg(msg);

	for(;;)										// ����ѭ����
    {
       		// �ȴ�һ���ַ�����
       if(SciaRegs.SCIFFRX.bit.RXFIFST >= 1) 
       { 
			SciaRegs.SCIRXBUF.all=SciaRegs.SCIRXBUF.all;
	       	msg = "\r\nѧ�ţ�3130102475\0";
			scia_msg(msg);

			msg = "\r\n���ڣ�2015-01-08\0";
			scia_msg(msg);

			msg = "\r\n���ţ���2¥116��17ʵ��̨\0";
			scia_msg(msg);

			msg = "\r\n������һ���ַ���dsp���ش�ѧ�š����ں����� \0";
    		scia_msg(msg);
       } 	

    }
}
//*******************************************************************************************
// ������  scia_echoback_init()
// ���ܣ� SCIA�ش���ʼ��
// ע�⣺ SCIA����ʱ��ͨ��InitSysCtrl()����������
// DSP281x_SysCtrl.c�ļ��е����໷����InitPll(0xA),��ϵͳ���ʱ�ӣ�
//SYSCLKOUT=(OSCCLK*DIV)/2=(30*10)/2=150MHz��������LOSPCP[2:0]ȡ��λĬ��ֵ2��
//�ʵ�������ʱ�� LSPCLK=150/4=37.5 MHz��
//*******************************************************************************************
void scia_echoback_init()
{   
    SciaRegs.SCICCR.all =0x0007;            // 1λֹͣλ������żУ�飬��ֹ���ͣ�8λ�ַ���
                                            // �첽ģʽ������-��Э�顣
    SciaRegs.SCICTL1.all =0x0003;           // ʹ��TX��RX���ڲ�SCIʱ��(SCICLK)��
                                            // ��ֹ���մ����жϣ���ֹ˯�ߣ���ֹ���ѡ�
    SciaRegs.SCICTL2.all =0x0003;           // ����RXRDY/BRKDT�жϣ�����TXRDY�жϡ�
    SciaRegs.SCICTL2.bit.TXINTENA =1;       // ����������ָ����λ��ķ����ظ�����һ��ָ�
    SciaRegs.SCICTL2.bit.RXBKINTENA =1;  
    SciaRegs.SCIHBAUD    =0x0001;  //���������ã�����������ʱ��LSPCLK = 37.5MHzʱ��������Ϊ9600 ��
    SciaRegs.SCILBAUD    =0x00E7;       
    SciaRegs.SCICTL1.all =0x0023;           // ������һ��SCICTL1����ָ�����
                                            // �����˷���SCI�����λ
}
//*******************************************************************************************
// ������  scia_xmit(int a)
// ���ܣ� ��SCI����һ���ַ�����ʽ����aΪ�����͵�����
//*******************************************************************************************
void scia_xmit(int a)
{
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
    SciaRegs.SCITXBUF=a;
}
//*******************************************************************************************
// ������  scia_msg(char * msg)
// ���ܣ�  ������Ϣ���ͺ������β�Ϊһָ�������msg����Ϣ������׵�ַ�������õ���msg[i]�ĺ���
//          Ϊ��������׵�ַmsg��ƫ�Ƶ�ַi�д洢�����ݡ�ע�⣺msg+i�������msg��ƫ����������
//          ������ʾmsg��ƫ�Ƶ�ַ��Ҫȡ���õ�ַ�д洢���ݻ�������*(msg+i)��
// ע�⣺  ��"//##..." ��ע��ָ��������һ����"//$$..."��ע��ָ��������ͬ���ö�ָ��ñ���
//          i�����������ݣ����ǲ���ָ�����ȡ�����ķ��������������ݸ��١���һ���ַ�������ʱ��
//          *msg ��������'\0'����˿�����'\0'������ַ����е��ַ��Ƿ�����ϡ�
//*******************************************************************************************
void scia_msg(char * msg)
{   
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
/*
    int i;
    i = 0;
    while(msg[i] != '\0')
    {   
        scib_xmit(msg[i]);
        i++;
    }
*/
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

//######################################################
/**/
    while(*msg != '\0')
    {
        scia_xmit(*msg++); 
    }
/**/
//######################################################
}

//*******************************************************************************************
// ������  scib_fifo_init()
// ���ܣ�  SCIB FIFO�Ƚ��ȳ�(FIFO)��ʼ���ӳ���
//*******************************************************************************************
void scia_fifo_init()
{
    SciaRegs.SCIFFTX.all=0xE040;
        // SCIFFTX[15]=SCIRST=1 ��       SCI FOFO���¿�ʼ���ͺͽ��ա�
        // SCIFFTX[14]=SCIFFENA=1��      ʹ��SCI FOFO��ǿ�͹��ܡ�
        // SCIFFTX[13]=TXFIFO Reset=1��  ����ʹ�ܷ���FIFO�Ĳ�����
        // SCIFFTX[12:8]=TXFFST��        ֻ��λ��Ϊ0ʱ������FIFO�ա�
        // SCIFFTX[7]=TXFFINT Flag�� ֻ��λ��Ϊ0ʱ��������TXFIFO�жϡ�
        // SCIFFTX[6]= TXFFINT CLR=1��   �����7λTXFFINT�ı�־��
        // SCIFFTX[5]=TXFFIENA=0��       ��ֹ����TXFFILƥ���TXFIFO�жϡ�
        // SCIFFTX[4:0]=TXFFIL=00000b��  TXFFIL4- TXFFIL0����FIFO�жϼ�λ��0��
    SciaRegs.SCIFFRX.all=0x204f;
        // SciaRegs[15]=RXFFOVF��        ֻ��λ��Ϊ0ʱ������FIFO�������
        // SciaRegs[14]= RXFFOVF CLR=0��д0ʱ����RXFFOVF��Ӱ�죻
        //                              д1ʱ����RXFFOVF��־λ
        // SciaRegs[13]=RXFIFO Reset=1������ʹ�ܽ���FIFO�Ĳ�����
        // SciaRegs[12:8]=RXFIFST��      ֻ��λ��Ϊ0ʱ������FIFO�ա�д��Ӱ�졣
        // SciaRegs[7]=RXFFINT Flag��    ֻ��λ��Ϊ0ʱ��û�в���RXFIFO�жϣ�
        //                              Ϊ1ʱ������RXFIFO�жϡ�
        // SciaRegs[6]= RXFFINT CLR=1��  ���7λRXFFINT Flag��־λ��
        // SciaRegs[5]=RXFFIENA=0��      ��ֹ����RXFFILƥ���RXFIFO�жϡ�
        // SciaRegs[4:0]=RXFFIL=01111b�������Ѿ���ֹ����RXFFILƥ���RXFIFO�жϣ�����ԭ
        //                              �����RXFFILλ��������������ġ�
    SciaRegs.SCIFFCT.all=0x0;
        // SCIFFCT[13]=CDC=0��           ��ֹ�Զ����У׼��
        // SCIFFCT[7:0]=FFTXDLY=0��      ��FIFO���ͻ�������������λ�Ĵ���֮�����ʱΪ0��
}


//===========================================================================================
// No more.
//===========================================================================================