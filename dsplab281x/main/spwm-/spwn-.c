/******************************************************************************************
**ÃèÊö:PWM1-6¿ØÖÆÄæ±äÇÅ6¸ö¿ª¹Ø¹Ü£¬¿ª¹ØÆµÂÊ60KHz
***ADCÖĞ¶ÏÓÉT2Æô¶¯£¬²ÉÑùÆµÂÊ10K(T2)
**ÖĞ¶ÏT1PINT¸üĞÂPWM1¡«6µÄÕ¼¿Õ±È(60k)£¬ÓÃÈ«±È½ÏÆ÷²úÉú3¶ÔPWM²¨£¬GP¶¨Ê±Æ÷1×÷È«±È½Ïµ¥ÔªÊ±»ù
**ÖĞ¶ÏT3PINT¸üĞÂPWM7¡«12µÄÕ¼¿Õ±È(60k)£¬ÓÃÈ«±È½ÏÆ÷²úÉú3¶ÔPWM²¨£¬GP¶¨Ê±Æ÷3×÷È«±È½Ïµ¥ÔªÊ±»ù
**ÖĞ¶ÏT4PINTÉ¨Ãè°´¼ü(10.24 ms)£¬ÊµÏÖËÄÂ·DA×ª»»,°´¼üµÄÖµ(0-9)ÖÃSCI·¢ËÍÊı¾İ*
**PDPINTAÖĞ¶Ï¹ÊÕÏ±£»¤ÊäÈë
**SCIÒÔÖĞ¶Ï·½Ê½½ÓÊÕ,ÓÃÉÏÎ»»ú´®¿Úµ÷ÊÔ³ÌĞòÊµÏÖÓëÎ»»úÍ¨Ñ¶
**EcanÒÔÖĞ¶Ï·½Ê½ÊµÏÖË«»úÍ¨Ñ¶,¹¦ÄÜB¼üÓÃÓÚÆô¶¯·¢ËÍ,½«ÏàÓ¦µÄÆµÂÊÖµ·¢ËÍ¶Ô·½
**¿ÉÖĞ¶ÏÇ¶Ì×
**¹¦ÄÜC¼ü:ÓÃ¼üÅÌÉè¶¨deaoutÆµÂÊ;¹¦ÄÜD¼ü:ÓÃADCINB7Í¨µÀÉè¶¨deaoutÆµÂÊ;
**¹¦ÄÜ¼üE/F,°´E¼üÊä³öÆµÂÊdeboutÓÉ¼üÅÌ0-DÉè¶¨,°´F¼üÊä³öÆµÂÊdeboutÓÉADCINB7Éè¶¨;
// sin±í³¤8192(8K)·ùÖµ0¡«1250, ÆµÂÊµ÷½Ú4Hz¡«2KHz
******************************************************************************************/

#include"DSP281x_Device.h"
#include   "math.h"           //ÓÃµ½ÕıÏÒÖµµÄ¼ÆËã

static  long  x1=0;
static  long  x2=0;
static  long  x3=0;	
static  long  x4=0;
static  long  x5=0;
static  long  x6=0;	
unsigned int LEDReg;
unsigned int LED4=0;    //ÒªÏÔÊ¾µÄËÄÎ»ÆµÂÊÖµ
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
void Write_LED (int LEDReg);//Í¨¹ıSPIÏòLED·¢ËÍÏÔÊ¾Êı¾İ
Uint16 keyNum = 0x0000;  //°´¼ü´ÎÊı
Uint16 RecieveChar; 
unsigned long int  AD1;  
unsigned long int AD2;  ////Êµ¼ÊADÖµ*1000

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

Uint16  sindata[8192];     //´æÕıÏÒ±í
interrupt void  T1pint_isr(void);//ÖĞ¶Ï×Ó³ÌĞòµÄÉùÃ÷
interrupt void  T3pint_isr (void);
interrupt void  T4pint_isr(void);
interrupt void  adc_isr(void);
interrupt void  Scirxinta_isr(void);
interrupt void  Ecan0inta_isr(void);
interrupt void  pdpaint_isr(void);

void  stop_pwm(void);       //Í£Ö¹PWM×Ó³ÌĞò
void  starlpwm(void);       //Æô¶¯PWM×Ó³ÌĞò

#define   PI2   3.1416* 2  //2¦Ğ
#define   FC    60000    //±£´æÔØ²¨ÆµÂÊ60KHz(PWM1¡«6)
#define   FC2   10000     //A£¯D²ÉÑùÆµÂÊ10KHz
#define   sinlen 8192     //8KµÄÕıÏÒ±í³¤¶È

long     Tlpr;            //±£´æTl¶¨Ê±Æ÷µÄÖÜÆÚ¼Ä´æÆ÷µÄÖµ
long     T2pr;
long     T3pr;           //±£´æT2£¬T3¶¨Ê±Æ÷µÄÖÜÆÚ¼Ä´æÆ÷µÄÖµ

unsigned long int    deaout=0;   //EVAÊä³öÆµÂÊ£¬ÔÚT4ÖĞ¶Ï³ÌĞòÖĞÓÉ°´¼ü¸Ä±ä
unsigned long int    dea=0;   //EVAÊä³öÆµÂÊ¶ÔÓ¦²é±íµÈ·ÖÖµ
unsigned long int    dea1=0;   //¶ÔÓ¦²é±íµÈ·ÖÖµÖĞ¼ä±äÁ¿
unsigned long int    debout=0;   //EVBÊä³öÆµÂÊ£¬ÔÚT4ÖĞ¶Ï³ÌĞòÖĞÓÉ°´¼ü¸Ä±ä
unsigned long int    deb=0;   //EVBÊä³öÆµÂÊ¶ÔÓ¦²é±íµÈ·ÖÖµ
unsigned long int    deb1=0;   //¶ÔÓ¦²é±íµÈ·ÖÖµÖĞ¼ä±äÁ¿
int  j;

void EVA_PWM()
{
   	EvaRegs.EXTCONA.bit.INDCOE = 1;  //µ¥¶ÀÊ¹ÄÜ±È½ÏÊä³öÄ£Ê½
    EvaRegs.ACTRA.all = 0x0aaa;     //¿Õ¼äÊ¸Á¿²»¶¯×÷
    EvaRegs.DBTCONA.all = 0x08ec;   //ËÀÇø¶¨Ê±Æ÷Æô¶¯
    EvaRegs.CMPR1 = 0x0006;
    EvaRegs.CMPR2 = 0x0005;
    EvaRegs.CMPR3 = 0x0004;
    EvaRegs.COMCONA.all = 0xa6e0;   //¿Õ¼äÏòÁ¿½ûÖ¹£¬È«±È½ÏÊ¹ÄÜ£¬ÏİÚå½ûÖ¹
}

void EVA_Timer1()
{
   	EvaRegs.EXTCONA.bit.INDCOE = 1;  //µ¥¶ÀÊ¹ÄÜ±È½ÏÊä³öÄ£Ê½
    EvaRegs.GPTCONA.all = 0x0000;   //GP¶¨Ê±Æ÷1±È½ÏÊä³öµÍÓĞĞ§
  	Tlpr=75000000/FC;
	EvaRegs.T1PR=Tlpr;
    EvaRegs.T1CNT = 0x0000;     // ¶¨Ê±Æ÷³õÖµ
    EvaRegs.T1CON.all = 0x0846;//.Á¬ĞøÔö¼õ¼ÆÊı,1·ÖÆµ,Ê¹ÄÜ±È½Ï,ÄÚ²¿Ê±ÖÓ,´ò¿ª¶¨Ê±Æ÷
}

void EVB_PWM()
{
   EvbRegs.EXTCONB.bit.INDCOE = 1;  //µ¥¶ÀÊ¹ÄÜ±È½ÏÊä³öÄ£Ê½
    EvbRegs.ACTRB.all = 0x0aaa;     //¿Õ¼äÊ¸Á¿²»¶¯×÷
    EvbRegs.DBTCONB.all = 0x08ec;   //ËÀÇø¶¨Ê±Æ÷Æô¶¯
    EvbRegs.CMPR4 = 0x0010;
    EvbRegs.CMPR5 = 0x0005;
    EvbRegs.CMPR6 = 0x0004;
    EvbRegs.COMCONB.all = 0xa6e0;   //¿Õ¼äÏòÁ¿½ûÖ¹£¬È«±È½ÏÊ¹ÄÜ£¬ÏİÚå½ûÖ¹
}

void EVB_Timer3()
{
   EvbRegs.EXTCONB.bit.INDCOE = 1;  //µ¥¶ÀÊ¹ÄÜ±È½ÏÊä³öÄ£Ê½
    EvbRegs.GPTCONB.all = 0x0000;   //GP¶¨Ê±Æ÷1±È½ÏÊä³öµÍÓĞĞ§
 	T3pr=75000000/FC;
	EvbRegs.T3PR=T3pr;   
    EvbRegs.T3CNT = 0x0000;     // ¶¨Ê±Æ÷³õÖµ
    EvbRegs.T3CON.all = 0x0846;////Á¬ĞøÔö¼õ¼ÆÊı,1·ÖÆµ,Ê¹ÄÜ±È½Ï,ÄÚ²¿Ê±ÖÓ,´ò¿ª¶¨Ê±Æ÷
}

void IOinit()
{
 	EALLOW;  
 	//½«GPIOAÅäÖÃÎªÍâÉè¿Ú
 	GpioMuxRegs.GPAMUX.all = 0xffff;

    //½«GPIOE0~GPIOE2ÅäÖÃÎªÒ»°ãI/O¿ÚÊä³ö,×÷138ÒëÂë  
 	GpioMuxRegs.GPEMUX.all = GpioMuxRegs.GPEMUX.all&0xfff8; 
	GpioMuxRegs.GPEDIR.all = GpioMuxRegs.GPEDIR.all|0x0007;
    //½«GPIOB8~GPIOB15ÅäÖÃÎªÒ»°ãI/O¿Ú,D0~D7
	GpioMuxRegs.GPBMUX.all = GpioMuxRegs.GPBMUX.all&0x00ff; 
	EDIS;				
}

//SPI³õÊ¼»¯×Ó³ÌĞò
void spi_intial()
 {
 	SpiaRegs.SPICCR.all =0x0047;   // Ê¹SPI´¦ÓÚ¸´Î»·½Ê½, ÏÂ½µÑØ, °ËÎ»Êı¾İ  
	SpiaRegs.SPICTL.all =0x0006;   // Ö÷¿ØÄ£Ê½, Ò»°ãÊ±ÖÓÄ£Ê½,Ê¹ÄÜtalk, ¹Ø±ÕSPIÖĞ¶Ï.
	SpiaRegs.SPIBRR =0x007F;       //ÅäÖÃ²¨ÌØÂÊ
	SpiaRegs.SPICCR.all =SpiaRegs.SPICCR.all|0x0080;  // ÍË³ö¸´Î»×´Ì¬	 
	EALLOW;	
    GpioMuxRegs.GPFMUX.all=0x000F;	// ÉèÖÃÍ¨ÓÃÒı½ÅÎªSPIÒı½Å	 	
    EDIS;
  }

void gpio_init()
{ 
	EALLOW;
	GpioMuxRegs.GPAMUX.bit.TDIRA_GPIOA11=0; //GPIOA11ÉèÖÃÎªÒ»°ãI/O¿Ú
	GpioMuxRegs.GPADIR.bit.GPIOA11=1;		//°ÑGPIOA11ÉèÖÃÎªÊä³ö						    
   
	GpioMuxRegs.GPFMUX.bit.XF_GPIOF14 = 0;  //  GPIOF14ÉèÖÃÎªÒ»°ãI/O¶Ë¿Ú
	GpioMuxRegs.GPFDIR.bit.GPIOF14 = 1;	   //  °ÑGPIOF14ÉèÖÃÎªÊä³ö			
    EDIS;
    GpioDataRegs.GPADAT.bit.GPIOA11=0; 	//GPIOA11=0;¸Ã¶Ë¿ÚÎª74HC595Ëø´æĞÅºÅ
    GpioDataRegs.GPFDAT.bit.GPIOF14 = 1; //  GPIOF14£½1;¸Ã¶Ë¿ÚÎªÊ¹ÄÜĞÅºÅ
}

void Scia_init()
{    
	EALLOW;
 	GpioMuxRegs.GPFMUX.bit.SCITXDA_GPIOF4 = 1;	 //  ÉèÖÃF4ºÍF5ÎªÍ¨ĞÅ¶Ë¿Ú
 	GpioMuxRegs.GPFMUX.bit.SCIRXDA_GPIOF5 = 1;	 
    EDIS;
	SciaRegs.SCICTL2.all = 0x0002;  //  ÔÊĞíRXÖĞ¶Ï
 	SciaRegs.SCILBAUD = 0x00E7;     //  ²¨ÌØÂÊ£½9600
	SciaRegs.SCIHBAUD = 0x0001;  	     
  SciaRegs.SCICCR.all = 0x0007;  	//  1¸öÍ£Ö¹Î»£¬½ûÖ¹Ğ£Ñé£¬8Î»×Ö·û 
                             		//  ½ûÖ¹×Ô²âÊÔ£¬Òì²½¿ÕÏĞÏßĞ­Òé
	SciaRegs.SCICTL1.all = 0x0023;  //  ÍÑÀë¸´Î»×´Ì¬£¬Ê¹ÄÜ½ÓÊÕ·¢ËÍ
}

void  CAN_INIT()
{
	struct ECAN_REGS ECanaShadow;  
	EALLOW;
  	GpioMuxRegs.GPFMUX.bit.CANTXA_GPIOF6 = 1; //  ÉèÖÃGPIOF6ÎªCANTX 
  	GpioMuxRegs.GPFMUX.bit.CANRXA_GPIOF7 = 1; //  ÉèÖÃGPIOF7ÎªCANRX    	
	EDIS;
/*eCAN ¿ØÖÆ¼Ä´æÆ÷ĞèÒª32Î»·ÃÎÊ¡£Èç¹ûÏëÏòÒ»¸öµ¥¶ÀÎ»½øĞĞĞ´²Ù×÷£¬±àÒëÆ÷¿ÉÄÜ»áÊ¹Æä½øÈë16Î»·ÃÎÊ¡£Õâ¶ùÒıÓÃÁËÒ»ÖÖ½â¾ö·½·¨£¬¾ÍÊÇÓÃÓ°×Ó¼Ä´æÆ÷ÆÈÊ¹½øĞĞ32Î»·ÃÎÊ¡£ °ÑÕû¸ö¼Ä´æÆ÷¶ÁÈëÒ»¸öÓ°×Ó¼Ä´æÆ÷¡£ Õâ¸ö·ÃÎÊ½«ÊÇ32Î»µÄ¡£ÓÃ32Î»Ğ´²Ù×÷¸Ä±äĞèÒª¸ÄµÄÎ»£¬È»ºó°Ñ¸ÃÖµ¿½±´»ØeCAN¼Ä´æÆ÷*/
    EALLOW; 
   	ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all; //  °ÑCANTIOC¶ÁÈëÓ°×Ó¼Ä´æÆ÷
   	ECanaShadow.CANTIOC.bit.TXFUNC = 1;              //  Íâ²¿Òı½ÅI/OÊ¹ÄÜ±êÖ¾Î»¡£
//  TXFUNC£½1  CANTXÒı½Å±»ÓÃÓÚCAN·¢ËÍ¹¦ÄÜ¡£
//  TXFUNC£½0  CANTXÒı½Å±»×÷ÎªÍ¨ÓÃI/OÒı½Å±»Ê¹ÓÃ 
	ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all; //  °ÑÅäÖÃºÃµÄ¼Ä´æÆ÷Öµ»ØĞ´
    ECanaShadow.CANRIOC.all = ECanaRegs.CANRIOC.all;  //  °ÑCANRIOC¶ÁÓ°×Ó¼Ä´æÆ÷
	ECanaShadow.CANRIOC.bit.RXFUNC = 1;              //  Íâ²¿Òı½ÅI/OÊ¹ÄÜ±êÖ¾Î»¡£
//  RXFUNC£½1  CANRXÒı½Å±»ÓÃÓÚCAN½ÓÊÕ¹¦ÄÜ¡£
//  RXFUNC£½0  CANRXÒı½Å±»×÷ÎªÍ¨ÓÃI/OÒı½Å±»Ê¹ÓÃ¡£
    ECanaRegs.CANRIOC.all = ECanaShadow.CANRIOC.all;  //  °ÑÅäÖÃºÃµÄ¼Ä´æÆ÷Öµ»ØĞ´
    EDIS;
//  ÔÚÅäÖÃÓÊÏäIDÖµÖ®Ç°£¬CANME¶ÔÓ¦µÄÎ»±ØĞë¸´Î»£¬
//  Èç¹ûCANME¼Ä´æÆ÷ÖĞ¶ÔÓ¦µÄÎ»±»ÖÃÎ»£¬ÔòIDĞ´Èë²Ù×÷ÎŞĞ§¡£
	ECanaRegs.CANME.all = 0;                 //  ¸´Î»ËùÓĞµÄÓÊÏä
   	ECanaMboxes.MBOX0.MSGID.all = 0x15100000;  //  ÅäÖÃ·¢ËÍÓÊÏä0µÄID£ºÀ©Õ¹±êÊ¶·û29Î»  
    ECanaMboxes.MBOX16.MSGID.all = 0x15200000; //  È·¶¨½ÓÊÕÓÊÏä16ID 
    //  °ÑÓÊ0¡«15 ÅäÖÃÎª·¢ËÍÓÊÏä £¬ °ÑÓÊÏä16¡«31 ÅäÎª½ÓÊÕÓÊÏä
    ECanaRegs.CANMD.all = 0xFFFF0000; 
    ECanaRegs.CANME.all = 0xFFFFFFFF;         //  CANÄ£¿éÊ¹ÄÜ¶ÔÓ¦µÄÓÊÏä£¬
        
    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;       //  °Ñ·¢ËÍ£¬½ÓÊÕÊı¾İµÄ³¤¶È¶¨ÒåÎª8Î»
    ECanaMboxes.MBOX0.MSGCTRL.bit.RTR = 0;       //  ÎŞÔ¶³ÌÖ¡Çëó  
//  ÒòÎªRTRÎ»ÔÚ¸´Î»ºó×´Ì¬²»¶¨£¬Òò´ËÔÚ³ÌĞò½øĞĞ³õÊ¼»¯µÄÊ±ºò±ØĞë¶Ô¸ÃÎ»¸³Öµ¡£
    ECanaMboxes.MBOX1.MSGCTRL.bit.RTR = 0;
//  °Ñ´ı·¢ËÍµÄÊı¾İĞ´Èë·¢ËÍÓÊÏä 
 	ECanaMboxes.MBOX0.MDL.all = 0x55555555;
	ECanaMboxes.MBOX0.MDH.all = 0x55555501;	 
	EALLOW;
//  ÓÊÏäÖĞ¶ÏÆÁ±Î¼Ä´æÆ÷¡£ÉÏµçºóËùÓĞµÄÖĞ¶ÏÆÁ±ÎÎ»¶¼ÇåÁãÇÒÍ£Ö¹ÖĞ¶ÏÊ¹ÄÜ¡£
//  ÕâĞ©Î»ÔÊĞí¶ÀÁ¢ÆÁ±ÎÈÎºÎÓÊÏäÖĞ¶Ï¡£
	ECanaRegs.CANGIM.all = 0x0001;     //ÖĞ¶Ï0Ê¹ÄÜ
	ECanaRegs.CANMIM.all = 0xFFFFFFFF;  
//  CANMIM .BIT.X£½1  ÓÊÏäÖĞ¶Ï±»Ê¹ÄÜ£¨X£½1¡«31£©
//  CANMIM .BIT.X£½0  ÓÊÏäÖĞ¶Ï±»½ûÖ¹£¨X£½1¡«31£©
	ECanaShadow.CANMC.all = ECanaRegs.CANMC.all; //  °ÑCANMC¶ÁÈëÓ°×Ó¼Ä´æÆ÷
	ECanaShadow.CANMC.bit.CCR = 1;               //  ¸Ä±äÅäÖÃÇëÇóÎ»
	ECanaShadow.CANMC.bit.SCB = 1;  			//eCANÄ£Ê½,ËùÓĞÓÊÏäÊ¹ÄÜ
	ECanaRegs.CANMC.all = ECanaShadow.CANMC.all; //  °ÑÅäÖÃºÃµÄ¼Ä´æÆ÷Öµ»ØĞ´
  	EDIS;
/*CPUÒªÇó¶ÔÅäÖÃ¼Ä´æÆ÷CANBTCºÍSCCµÄ½ÓÊÕÆÁ±Î¼Ä´æÆ÷(CANGAM£¬LAM[0]ºÍLAM[3])½øĞĞĞ´²Ù×÷¡£¶Ô¸ÃÎ»ÖÃÎ»ºó£¬CPU±ØĞëµÈ´ı£¬Ö±µ½CANES¼Ä´æÆ÷µÄCCE±êÖ¾Î»ÔÚËÍÈëCANBTC¼Ä´æÆ÷Ö®Ç°Îª1 */
do 
    {
      ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    } while(ECanaShadow.CANES.bit.CCE != 1 );  //  µ±CCE£½1Ê±¿ÉÒÔ¶ÔCANBTC½øĞĞ²Ù×÷¡£
    //  ÅäÖÃ²¨ÌØÂÊ
    EALLOW;
    ECanaShadow.CANBTC.all = ECanaRegs.CANBTC.all; //  °ÑCANBTC¶ÁÈëÓ°×Ó¼Ä´æÆ÷
    ECanaShadow.CANBTC.bit.BRPREG = 149;   //  (BRP+1)£½150£¬ ×îĞ¡Ê±¼äµ¥Î»TQ£½1us
    ECanaShadow.CANBTC.bit.TSEG2REG = 2;   //  Î»¶¨Ê±bit-time£½(TSEG1+1)+(TSEG1+1)+1
    ECanaShadow.CANBTC.bit.TSEG1REG = 3;   //  bit-time£½8us£¬ ËùÒÔ²¨ÌØÂÊÎª125Kpbs       
    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;  //  °ÑÅäÖÃºÃµÄ¼Ä´æÆ÷Öµ»ØĞ´
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;   //  °ÑCANMC¶ÁÈëÓ°×Ó¼Ä´æÆ÷
    ECanaShadow.CANMC.bit.CCR = 0 ;       //  ÉèÖÃCCR£½0£¬ CPUÇëÇóÕı³£Ä£Ê½
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;   //  °ÑÅäÖÃºÃµÄ¼Ä´æÆ÷Öµ»ØĞ´
    EDIS;
    do
    {
      ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    } while(ECanaShadow.CANES.bit.CCE != 0 );  //  µÈ´ı CCE Î»±»ÇåÁã
//  ÅäÖÃeCANÎª×Ô²âÊÔÄ£Ê½£¬Ê¹ÄÜeCANµÄÔöÇ¿ÌØĞÔ
    EALLOW;
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.STM = 0;        //  ÅäÖÃCAN ÎªÕı³£Ä£Ê½
  	// CANMC.bit.STM£½0£¬Õı³£Ä£Ê½£¬CANMC.bit.STM£½1£¬×Ô²âÊÔÄ£Ê½
    ECanaShadow.CANMC.bit.SCB = 1;        // Ñ¡ÔñHECC¹¤×÷Ä£Ê½
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
    EDIS;
}

void mailbox_read(int16 MBXnbr)
{
  	volatile struct MBOX *Mailbox;
	Mailbox = &ECanaMboxes.MBOX0 + MBXnbr;
	TestMbox1 = Mailbox->MDL.all;    	// ¶Á³öµ±Ç°ÓÊÏäÊı¾İµÍ4×Ö½Ú
	TestMbox2 = Mailbox->MDH.all;       //  ¶Á³öµ±Ç°ÓÊÏäÊı¾İ¸ß4×Ö½Ú
	TestMbox3 = Mailbox->MSGID.all;     //  ¶Á³öµ±Ç°ÓÊÏäID
} 

//ADCÄ£¿éÉÏµçÑÓÊ±
void Adc_PowerUP() 
{ 	
  	AdcRegs.ADCTRL3.bit.ADCBGRFDN = 0x3;  //ADC´øÏ¶ºÍ²Î¿¼µçÂ·¼Óµç
  	for (i=0; i<1000000; i++){}      //ÖÁÉÙ5msÑÓÊ±
  	AdcRegs.ADCTRL3.bit.ADCPWDN = 1; //ADCºËÄ£ÄâµçÂ·¼Óµç
  	for (i=0; i<10000; i++){}        //ÖÁÉÙ20usÑÓÊ±
}

void Adc_Init()
{
    AdcRegs.ADCTRL1.bit.CONT_RUN = 0;      //Æô¶¯-Í£Ö¹/Á¬Ğø×ª»»Ñ¡Ôñ:Æô¶¯-Í£Ö¹·½Ê½
    AdcRegs.ADCTRL1.bit.CPS = 1;           //ºËÊ±ÖÓÔ¤¶¨±êÆ÷:ADC_CLK=ADCLKPS/2=3.125M
  	AdcRegs.ADCTRL1.bit.ACQ_PS = 0xf;      //²É¼¯´°¿Ú´óĞ¡:SH pulse/clock=16
   	AdcRegs.ADCTRL3.bit.ADCCLKPS = 0x5;    //ºËÊ±ÖÓ·ÖÆµ:ADCLKPS=HSPCLK/4=6.25M
	AdcRegs.ADCMAXCONV.all = 0x0000;       //×ª»»Í¨µÀÊı:SEQ1ĞòÁĞµÄÍ¨µÀÊıÎª1
  	AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0xf; //×ª»»Í¨µÀÑ¡Ôñ:ADCINB7
  	 
	AdcRegs.ADCTRL2.bit.EVA_SOC_SEQ1=1;  //SEQl±»EVAµÄ´¥·¢Ô´Æô¶¯
  	AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1=1;  //Ê¹ÄÜÖĞ¶Ï		
}

void LEDdisplay(int LED8,int LED7,int LED6,int LED5,int LED4,int LED3,int LED2,int LED1)
{
         GpioDataRegs.GPADAT.bit.GPIOA11=0; //¸øLACKĞÅºÅÒ»¸öµÍµçÆ½
	   	Write_LED (LED8);//ÏòLED×î¸ßÎ»Ğ´Êı¾İ,ÏÔÊ¾LED8
		Write_LED (LED7);
		Write_LED (LED6);
		Write_LED (LED5);
		Write_LED (LED4);
		Write_LED (LED3);
		Write_LED (LED2);
		Write_LED (LED1);//ÏòLED×îµÍÎ»Ğ´Êı¾İ,ÏÔÊ¾LED1

     	GpioDataRegs.GPADAT.bit.GPIOA11=1; //¸øLACKĞÅºÅÒ»¸ö¸ßµçÆ½ÎªËø´æ74HC595     	
} 

void main(void)
{		
   	asm (" NOP ");
   	asm (" NOP ");
   	InitSysCtrl();      //³õÊ¼»¯ÏµÍ³¿ØÖÆ¼Ä´æÆ÷, Ê±ÖÓÆµÂÊ150M
	EALLOW;				
	SysCtrlRegs.HISPCP.all = 0x0001;//¸ßËÙÊ±ÖÓµÄ¹¤×÷ÆµÂÊ£½SYSCLKOUT/2=75M
	EDIS;
	DINT;	        //¹Ø±Õ×ÜÖĞ¶Ï£¬Çå³ıÖĞ¶Ï±êÖ¾
	IER = 0x0000;   //¹Ø±ÕÍâÎ§ÖĞ¶Ï
	IFR = 0x0000;	//ÇåÖĞ¶Ï±êÖ¾
	spi_intial();   //SPI³õÊ¼»¯×Ó³ÌĞò
	gpio_init();   
	IOinit();	    // I/O³õÊ¼»¯×Ó³ÌĞò	
	EVA_PWM();	
	EVA_Timer1();
	EVB_PWM();	
	EVB_Timer3 ();
  	Scia_init();//³õÊ¼»¯SCl
   	CAN_INIT();

	EALLOW;
	GpioMuxRegs.GPAMUX.all=0x003F;    //EVAPWMl-6
	GpioMuxRegs.GPBMUX.all=0x003F;    //EVBPWM7-12
	EDIS;

//³õÊ¼»¯PIE¿ØÖÆ¼Ä´æÆ÷
	InitPieCtrl();       //DSP281x_PieCtrl£®C
//³õÊ¼»¯PIEÖĞ¶ÏÏòÁ¿±í
	InitPieVectTable();  //DSP281x_PieVect£®C
	LEDdisplay(0,0,0,0,0,0,0,0);

//£®³õÊ¼»¯ADC¼Ä´æÆ÷
 	Adc_PowerUP();
 	Adc_Init();

//Ê¹ÄÜEva¡¢EvbÏàÓ¦Î»
	EvaRegs.COMCONA.bit.FCOMPOE=1;	//±È½Ïµ¥ÔªÊ¹ÄÜ
	EvaRegs.EVAIMRA.bit.T1PINT=1;	//Ê¹ÄÜT1PINTÖĞ¶Ï
	EvaRegs.EVAIFRA.bit.T1PINT=1;	//ÏÈÇå

	EvbRegs.COMCONB.bit.FCOMPOE=1;	//±È½Ïµ¥ÔªÊ¹ÄÜ
	EvbRegs.EVBIMRA.bit.T3PINT=1;	//Ê¹ÄÜT3PINTÖĞ¶Ï
	EvbRegs.EVBIFRA.bit.T3PINT=1;	//ÏÈÇå

//T2£ºÆô¶¯AD×ª»»
	T2pr=75000000/FC2;
	EvaRegs.T2PR=T2pr;	//T2¶¨Ê±Æ÷ÖÜÆÚ
	EvaRegs.GPTCONA.bit.T2TOADC=2;	//ÓÉ¶¨Ê±Æ÷2ÖÜÆÚÖĞ¶ÏÆô¶¯A£¯D×ª»»
	EvaRegs.T2CON.all=0x084C;
	EvaRegs.T2CNT=0x0000;	//T2¼ÆÊıÆ÷Çå

//T4£º°´¼ü £¬³õÊ¼»¯°´¼ü¼ì²âÖĞ¶Ï
	EvbRegs.T4PR=12000;	//T4¶¨Ê±Æ÷ÖÜÆÚ
	EvbRegs.T4CON.all=0x1744;	//x£¯128£¬Ôö¼ÆÊı10£®24 ms
	EvbRegs.T4CNT=0x0000;//T4¼ÆÊıÆ÷Çå

	EvbRegs.EVBIMRB.bit.T4PINT=1;	//Ê¹ÄÜT4PINTÖĞ¶Ï
	EvbRegs.EVBIFRB.bit.T4PINT=1;	//ÏÈÇå

//£®³õÊ¼»¯Õ¼¿Õ±ÈÁĞ±í
	for(j=0;j<8192;j++)	//ÔÚÏß¼ÆËãÕıÏÒ±í
	{
	sindata[j]=625-sin(j*PI2/8192)*625*0.85;
	}

//####£®³õÊ¼»¯Xintf
	InitXintf();	//³õÊ¼»¯XINTF

//####ÖĞ¶ÏÊ¹ÄÜ
	EALLOW;
	PieVectTable.T1PINT=&T1pint_isr;//PIEÖĞ¶ÏÊ¸Á¿±í
	PieVectTable.T3PINT=&T3pint_isr;
	PieVectTable.T4PINT=&T4pint_isr;
	PieVectTable.ADCINT=&adc_isr;
	PieVectTable.RXAINT=&Scirxinta_isr;
	PieVectTable.PDPINTA=&pdpaint_isr;	
	PieVectTable.ECAN0INTA=&Ecan0inta_isr;
	EDIS;

	PieCtrlRegs.PIEIER2.bit.INTx4=1;	//Ê¹ÄÜT1PINT
	PieCtrlRegs.PIEIER4.bit.INTx4=1;	//Ê¹ÄÜT3PINT
	PieCtrlRegs.PIEIER5.bit.INTx1=1;	//Ê¹ÄÜT4PINT
	PieCtrlRegs.PIEIER1.bit.INTx6=1;	//Ê¹ÄÜADCINT
	PieCtrlRegs.PIEIER9.bit.INTx1=1;	//Ê¹ÄÜSCIRXlNTA
	PieCtrlRegs.PIEIER9.bit.INTx5=1;	//Ê¹ÄÜECAN0lNT
	PieCtrlRegs.PIEIER1.bit.INTx1=1;	//Ê¹ÄÜPDPAINT

	IER |=(M_INT1 | M_INT2 | M_INT5 |M_INT9 |M_INT4);//Ê¹ÄÜINTl£¬INT2£¬INT5£¬INT9

	EINT;	//¿ª·ÅÈ«¾ÖÖĞ¶Ï
	ERTM;	//EnableGlobal realtime interrupt DBGM

//Ö÷Ñ­»·
	while (1)
 	{
	dea1=8192*deaout*2/1000;//ÖĞ¼ä±äÁ¿£¬·À¼ÆËãÒç³ö
	dea=dea1*Tlpr/75000;//¼ÆËã²é±íµÈ·ÖÖµ
	deb1=8192*debout*2/1000;
	deb=deb1*T3pr/75000;		
	}
}

interrupt  void  T1pint_isr(void)	//ÖĞ¶ÏT1PINT¸üĞÂPWMl¡«6µÄÕ¼¿Õ±È(60k)
{
	static  long  Ia=0;
	static  long  resa=0;
//####  ¸üĞÂpwm1¡«6Õ¼¿Õ±ÈEVA
	x1=Ia*dea;
	x1=resa+x1;//AÏà½Ç¶È
	Ia++;	//Ê±¼ä
	if(x1>(sinlen-1))	//Èç¹û³¬³öÕıÏÒ±í³¤£¬Ôò¼õÈ¥³¬³öÕıÏÒ±í³¤¶È
	{
	resa=x1-sinlen;	//AÏà½Ç¶È·¶Î§ÏŞÖÆÔÚ0¡«360
	x1=resa;
	Ia=1;	//ÖØĞÂ¿ªÊ¼¼ÆÊı£¬t=0
	}
	x2=x1-2731;	//x2=x1-sinlen£¯3£»BÏà
	x3=x1-5461;	//x3=xl-sinlen*2£¯3£»CÏà
	if(x2<0)
	x2=x2+sinlen;	//BÏà½Ç¶È·¶Î§ÏŞÖÆÔÚ0¡«360¡ã
	if(x3<0)	
	x3=x3+sinlen;	//CÏà½Ç¶È·¶Î§ÏŞÖÆÔÚ0¡«360¡ã
	EvaRegs.CMPR1=sindata[x1];	//Ë¢ĞÂÕ¼¿Õ±È
	EvaRegs.CMPR2=sindata[x2];
	EvaRegs.CMPR3=sindata[x3];
	//####  ¸´Î»
	EvaRegs.EVAIFRA.bit.T1PINT=1;	//ÖĞ¶Ï±êÖ¾Çå
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP2; //ÔÊĞíÏÂ´ÎÖĞ¶Ï
	return;
}

interrupt void  T3pint_isr (void)
	{
	static  long  Ib=0;
	static  long  resb=0;

//####  ¸üĞÂpwm7¡«11Õ¼¿Õ±ÈEVB
	x4=Ib*deb;
	x4=resb+x4;//AÏà½Ç¶È
	Ib++;	//Ê±¼ä
	if(x4>(sinlen-1))	//Èç¹û³¬³öÕıÏÒ±í³¤¶È£¬Ôò¼õÈ¥³¬³öÕıÏÒ±í³¤¶È
	{
	resb=x4-sinlen;	//AÏà½Ç¶È·¶Î§ÏŞÖÆÔÚ0¡«360
	x4=resb;
	Ib=1;	//ÖØĞÂ¿ªÊ¼¼ÆÊı£¬t=0
	}
	x5=x4-2731;	//x5=x4-sinlen£¯3£»BÏà
	x6=x4-5461;	//x6=x4-sinlen*2£¯3£»CÏà
	if(x5<0)
	x5=x5+sinlen;	//BÏà½Ç¶È·¶Î§ÏŞÖÆÔÚ0¡«360¡ã
	if(x6<0)	
	x6=x6+sinlen;		//CÏà½Ç¶È·¶Î§ÏŞÖÆÔÚ0¡«360¡ã
	EvbRegs.CMPR4=sindata[x4];	//Ë¢ĞÂÕ¼¿Õ±È
	EvbRegs.CMPR5=sindata[x5];
	EvbRegs.CMPR6=sindata[x6];
	//####  ¸´Î»
	EvbRegs.EVBIFRA.bit.T3PINT=1;	//ÖĞ¶Ï±êÖ¾Çå
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP4; //ÔÊĞíÏÂ´ÎÖĞ¶Ï
	return;
	}

interrupt void   T4pint_isr(void)
	{
		DAC();  //ËÄÂ·DA×ª»»
 	    if (KeyIn1() == 1)     // µ÷ÓÃ²é¼üK1~K8×Ó³ÌĞò
 	    {
 	    keyNum=keyNum+1; 	 
	    KeyFunction1(KeyReg1);
		SciaRegs.SCITXBUF=LEDReg+48;  //·¢ËÍ¼üÖµ,×ª»»³ÉASCIIÂë0¡«9
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
 		LEDdisplay(15,11,17,19,LED4,LED3,LED2,LED1); //ÏÔÊ¾FB-Ãğ****

		EvbRegs.EVBIFRB.bit.T4PINT=1;	//ÖĞ¶Ï±êÖ¾Çå
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //ÔÊĞíÏÂ´ÎÖĞ¶Ï
		return;
		}
		deaout=LED4*1000+LED3*100+LED2*10+LED1;
 		LEDdisplay(15,10,17,19,LED4,LED3,LED2,LED1); //ÏÔÊ¾FA-Ãğ****
	
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//ÖĞ¶Ï±êÖ¾Çå
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //ÔÊĞíÏÂ´ÎÖĞ¶Ï
		return;
 	    }

//¹¦ÄÜE¼ü:ÓÃ¼üÅÌÉè¶¨deaoutÆµÂÊ;¹¦ÄÜF¼ü:ÓÃADCINB7Í¨µÀÉè¶¨deaoutÆµÂÊ;
//¹¦ÄÜC¼ü:ÓÃ¼üÅÌÉè¶¨deboutÆµÂÊ;¹¦ÄÜD¼ü:ÓÃADCINB7Í¨µÀÉè¶¨deboutÆµÂÊ;

		if (KeyIn2() == 1)     // µ÷ÓÃ²é¼üK8~K16×Ó³ÌĞò
 	    {
 	    KeyFunction2(KeyReg2);
		SciaRegs.SCITXBUF=LEDReg+48;  //·¢ËÍ¼üÖµ£¬×ª»»³ÉASCIIÂë0¡«9
		if(LEDReg==0x0E) 		//ÅĞÊÇ·ñÎª¹¦ÄÜE¼ü
		{
		flag=0;
		LEDdisplay(15,10,17,19,LED4,LED3,LED2,LED1); //ÏÔÊ¾FA-Ãğ****
		deaout=LED4*1000+LED3*100+LED2*10+LED1;   //ÆµÂÊÉè¶¨Öµ
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//ÖĞ¶Ï±êÖ¾Çå
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //ÔÊĞíÏÂ´ÎÖĞ¶Ï
		return;
		}

		if(LEDReg==0x0F)		////ÅĞÊÇ·ñÎª¹¦ÄÜF¼ü
		{
	 	flag=0;
		//¹¦ÄÜF¼ü:ÓÃADCINB7Í¨µÀÉè¶¨deaoutÆµÂÊ				
		deaout=AD1;
		LEDdisplay((AD2/1000)+20,(AD2%1000)/100,(AD2%100)/10,19,
        AD1/1000,(AD1%1000)/100,(AD1%100)/10,AD1%10);
			
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//ÖĞ¶Ï±êÖ¾Çå
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //ÔÊĞíÏÂ´ÎÖĞ¶Ï
		return;	
		}	
	
	if(LEDReg==0x0C) 	//ÅĞÊÇ·ñÎª¹¦ÄÜC¼ü
		{
		flag=1;          //ÉèÖÃ¹¦ÄÜC¼ü±êÖ¾Î»
		LEDdisplay(15,11,17,19,LED4,LED3,LED2,LED1); //ÏÔÊ¾FB-Ãğ****
		debout=LED4*1000+LED3*100+LED2*10+LED1;   //ÆµÂÊÉè¶¨Öµ

		EvbRegs.EVBIFRB.bit.T4PINT=1;	//ÖĞ¶Ï±êÖ¾Çå
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //ÔÊĞíÏÂ´ÎÖĞ¶Ï
		return;
		}

		if(LEDReg==0x0D)		//ÅĞÊÇ·ñÎª¹¦ÄÜD¼ü
		{
		//¹¦ÄÜD¼ü:ÓÃADCINB7Í¨µÀÉè¶¨deboutÆµÂÊ;			
		debout=AD1;
		LEDdisplay((AD2/1000)+20,(AD2%1000)/100,(AD2%100)/10,19,
        AD1/1000,(AD1%1000)/100,(AD1%100)/10,AD1%10);
			
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//ÖĞ¶Ï±êÖ¾Çå
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //ÔÊĞíÏÂ´ÎÖĞ¶Ï
		return;	
		}

	if(LEDReg==0x0B)		////ÅĞÊÇ·ñÎª¹¦ÄÜB¼ü CAN·¢ËÍ
		{	 
	 	if	(flag==1)
		{
		ECanaMboxes.MBOX0.MDL.all = 0x55555502;//·¢ËÍ±êÖ¾
		ECanaMboxes.MBOX0.MDH.all = debout;
		ECanaRegs.CANTRS.all=0x0001; //·¢ËÍ0ÓÊÏä 
		while(ECanaRegs.CANTA.all!=0x0001) {};
		ECanaRegs.CANTA.all=0xFFFF;		//Ğ´1Çå.	
		}
		if	(flag==0)
		{
		ECanaMboxes.MBOX0.MDL.all = 0x55555501;//·¢ËÍ±êÖ¾
		ECanaMboxes.MBOX0.MDH.all = deaout;
		ECanaRegs.CANTRS.all=0x0001;//·¢ËÍ0ÓÊÏä 
		while(ECanaRegs.CANTA.all!=0x0001) {};
		ECanaRegs.CANTA.all=0xFFFF;		//Ğ´1Çå
		}	
	 
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//ÖĞ¶Ï±êÖ¾Çå
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //ÔÊĞíÏÂ´ÎÖĞ¶Ï
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
		LEDdisplay(15,11,17,19,LED4,LED3,LED2,LED1); //ÏÔÊ¾FB-Ãğ****
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//ÖĞ¶Ï±êÖ¾Çå
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //ÔÊĞíÏÂ´ÎÖĞ¶Ï
		return; 
		}
		deaout=LED4*1000+LED3*100+LED2*10+LED1;   //ÆµÂÊÉè¶¨Öµ
		LEDdisplay(15,10,17,19,LED4,LED3,LED2,LED1); //ÏÔÊ¾FA-Ãğ****
 	     } 
	EvbRegs.EVBIFRB.bit.T4PINT=1;	//ÖĞ¶Ï±êÖ¾Çå
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //ÔÊĞíÏÂ´ÎÖĞ¶Ï
	return; 
	}

interrupt void   adc_isr(void)
	{	
   	   AD1=AdcRegs.ADCRESULT0 >> 4;
	   AD2=(AD1*3*1000)/4095;   //Êµ¼ÊADÖµ*1000		

	   AdcRegs.ADCST.bit.INT_SEQ1_CLR=1;//ÇåÖĞ¶Ï±êÖ¾
       PieCtrlRegs.PIEACK.all=PIEACK_GROUP1; //ÔÊĞíÏÂ´ÎÖĞ¶Ï       		                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
	return;
	}

interrupt void  Scirxinta_isr(void)
	{
	  EINT;  //ÔÊĞíÖĞ¶ÏÇ¶Ì×
	 RecieveChar=SciaRegs.SCIRXBUF.bit.RXDT;
	 SciaRegs.SCITXBUF = RecieveChar;
 	 while(SciaRegs.SCICTL2.bit.TXRDY ==  0){}		
   	 while(SciaRegs.SCICTL2.bit.TXEMPTY ==  0){}
      PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //ÔÊĞíÏÂ´ÎÖĞ¶Ï	
	return;
	}

interrupt void Ecan0inta_isr(void)	//½ÓÊÕĞÅÏ¢Ğü¹ÒÖĞ¶Ï
	{
	if(ECanaRegs.CANRMP.all==0x00010000)
	{
	ECanaRegs.CANRMP.all=0xFFFF0000;
 	mailbox_read(16);
	if(	TestMbox1==0x55555501)
		{
		deaout=TestMbox2;
		LEDdisplay(15,10,17,19, deaout/1000,(deaout%1000)/100,(deaout%100)/10,deaout%10);	
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //ÔÊĞíÏÂ´ÎÖĞ¶Ï	
		return;
		}
	if(	TestMbox1==0x55555502)
		{
		debout=TestMbox2;
		LEDdisplay(15,11,17,19, debout/1000,(debout%1000)/100,(debout%100)/10,debout%10);
	
		 PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //ÔÊĞíÏÂ´ÎÖĞ¶Ï	
		return;
		}
	}
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //ÔÊĞíÏÂ´ÎÖĞ¶Ï
	return;
	}

	interrupt void  pdpaint_isr(void)
	{
	return;
	}

int KeyIn1(void)
{
 	 EALLOW;  
    //½«GPIOB8~GPIOB15ÅäÖÃÎªÊäÈë,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all&0x00ff;
     EDIS;    
    GpioDataRegs.GPEDAT.all = 0xfff8;     //Ñ¡Í¨KEYµÍ8Î»
    for (i=0; i<100; i++){}               //ÑÓÊ±
    //ÅĞK1~K8ÊÇ·ñ°´ÏÂ
    if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
    {
    	for (i=0; i<40000; i++){}        //ÑÓÊ±Ïû¶¶
        if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
        {
	   KeyReg1=GpioDataRegs.GPBDAT.all|0x00ff ;
    		while ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff) //ÅĞÊÇ·ñËÉ¿ª
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
    //½«GPIOB8~GPIOB15ÅäÖÃÎªÊäÈë,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all&0x00ff;
     EDIS;    
    GpioDataRegs.GPEDAT.all = 0xfff9;     //Ñ¡Í¨KEY¸ß8Î»
    for (i=0; i<100; i++){}               //ÑÓÊ±
    //ÅĞK8~K16ÊÇ·ñ°´ÏÂ
    if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
    {
    	for (i=0; i<40000; i++){}        //ÑÓÊ±Ïû¶¶
        if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
        {
	   KeyReg2=GpioDataRegs.GPBDAT.all|0x00ff ;
    		while ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff) //ÅĞÊÇ·ñËÉ¿ª
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

//MAX5742  ËÄÍ¨µÀDAC×ª»»,SPICCRĞèÇĞ»»Êı¾İÎ»Êı£¬·Ö±ğÖ¸Ê¾deaout,debout,AD1,ÆäÖµ¡´4095
	
void  DAC(void)
{  
 	SpiaRegs.SPICCR.all =0x00cf;   // Ê¹SPI·¢ËÍ16Î»Êı¾İ
 	 asm (" NOP ");
   	asm (" NOP ");
 	GpioDataRegs.GPFDAT.bit.GPIOF14 = 0;   //Æ¬Ñ¡DAC  
	 SpiaRegs.SPITXBUF = 0xf010;    // ³õÊ¼»¯Ö¸Áî£ºDAC_A-DÍ¨µÀÊ¹ÄÜ
   	while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){} 	//·¢ËÍÊÇ·ñÍê³É
   SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;		//¶Á³öÇå±êÖ¾
    GpioDataRegs.GPFDAT.bit.GPIOF14 = 1;	//Æ¬Ñ¡¸ßµçÆ½ÑÓÊ±
    	for(i = 0; i<10; i++){}
   	GpioDataRegs.GPFDAT.bit.GPIOF14 = 0;
	SpiaRegs.SPITXBUF =deaout;          //  DAC_AÍ¨µÀÖÃÊı
    	while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){}
   	 	SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;
    		GpioDataRegs.GPFDAT.bit.GPIOF14 = 1; 
    		for(i=0; i<10; i++){}

	GpioDataRegs.GPFDAT.bit.GPIOF14 = 0;
	SpiaRegs.SPITXBUF =debout|0x1000;     //  DAC_BÍ¨µÀÖÃÊı
    	while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){}
  	 	SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;
   		GpioDataRegs.GPFDAT.bit.GPIOF14 = 1; 
    		for(i=0; i<10; i++){}

		GpioDataRegs.GPFDAT.bit.GPIOF14 = 0;
	SpiaRegs.SPITXBUF =AD1|0x2000;     //  DAC_CÍ¨µÀÖÃÊı
    	while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){}
   	 	SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;
    		GpioDataRegs.GPFDAT.bit.GPIOF14 = 1; 
    		for(i=0; i<10; i++){}		

		GpioDataRegs.GPFDAT.bit.GPIOF14 = 0;
	SpiaRegs.SPITXBUF =AD1|0x3000;          //  DAC_DÍ¨µÀÖÃÊı
    	while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){}
   	 	SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF;
    		GpioDataRegs.GPFDAT.bit.GPIOF14 = 1; 
    		for(i=0; i<10; i++){}

	SpiaRegs.SPICCR.all =0x00c7; // Ê¹SPI´¦ÓÚ¸´Î»·½Ê½,ÏÂ½µÑØ,8Î»Êı¾İ,ÇĞ»»ÎªÏÔÊ¾Ä£Ê½   
}

//Í¨¹ıSPIÏòLED·¢ËÍÏÔÊ¾Êı¾İ
void Write_LED (int LEDReg)
{
Uint16 LEDcode[30]={0xc000,0xf900,0xA400,0xB000,0x9900,0x9200,0x8200,0xF800,
                    0x8000,0x9000,0x8800,0x8300,0xc600,0xa100,0x8600,0x8e00,
                    0x8c00,0xbf00,0xa700,0xff00,0x4000,0x7900,0x2400,0x3000,
                    0x1900,0x1200,0x0200,0x7800,0x0000,0x1000};//¹²Ñô×ÖĞÎÂë0~f, P£¬£­£¬L£¬"Ãğ",0.~9.	
	 		SpiaRegs.SPITXBUF =LEDcode[LEDReg]; //¸øÊıÂë¹ÜËÍÊı
    	 	 while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){} 		
    		SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF; //¶Á³öÇå±êÖ¾
}
//=========================================================================================
// No more.
//=========================================================================================

