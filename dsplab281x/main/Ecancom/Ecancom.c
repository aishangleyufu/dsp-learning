/******************************************************************************************
**√Ë ˆ:Ecan“‘÷–∂œ∑Ω Ω µœ÷À´ª˙Õ®—∂,π¶ƒ‹Bº¸”√”⁄∆Ù∂Ø∑¢ÀÕ,Ω´œ‡”¶µƒ…Ë∂®÷µ∑¢ÀÕ∂‘∑Ω
*;∑¢ÀÕŒª:LED8°¢LED7°¢LED6£¨Ω” ’Œª£∫LED3°¢LED2°¢LED1, Æ¡˘Ω¯÷∆∑Ω Ω
*÷–∂œT4PINT…®√Ë∞¥º¸(10.24 ms)
******************************************************************************************/

#include"DSP281x_Device.h"

unsigned int LEDReg;
unsigned int LED8=0; //œ‘ æµƒ∑¢ÀÕŒª
unsigned int LED7=0;
unsigned int LED6=0;   
unsigned int LED3=0; //œ‘ æµƒΩ” ’Œª
unsigned int LED2=0;
unsigned int LED1=0;
unsigned int KeyReg1;
unsigned int KeyReg2;
unsigned long int i = 0;
void  IOinit(void);
int   KeyIn1(void);
void  KeyFunction1(unsigned int KeyReg1);
void  KeyFunction2(unsigned int KeyReg2);
void Write_LED (int LEDReg);//Õ®π˝SPIœÚLED∑¢ÀÕœ‘ æ ˝æ›

Uint16 keyNum = 0x0000;  //∞¥º¸¥Œ ˝
Uint16 RecieveChar;
Uint16 transmitChar;

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

interrupt void  T4pint_isr(void);//÷–∂œ◊”≥Ã–Úµƒ…˘√˜
interrupt void  Ecan0inta_isr(void);

void IOinit()
{
 	EALLOW;  
 	//Ω´GPIOA≈‰÷√Œ™Õ‚…Ëø⁄
 	GpioMuxRegs.GPAMUX.all = 0xffff;

    //Ω´GPIOE0~GPIOE2≈‰÷√Œ™“ª∞„I/Oø⁄ ‰≥ˆ,◊˜138“Î¬Î  
 	GpioMuxRegs.GPEMUX.all = GpioMuxRegs.GPEMUX.all&0xfff8; 
	GpioMuxRegs.GPEDIR.all = GpioMuxRegs.GPEDIR.all|0x0007;
    //Ω´GPIOB8~GPIOB15≈‰÷√Œ™“ª∞„I/Oø⁄,D0~D7
	GpioMuxRegs.GPBMUX.all = GpioMuxRegs.GPBMUX.all&0x00ff;
	GpioMuxRegs.GPAMUX.bit.TDIRA_GPIOA11=0; //GPIOA11…Ë÷√Œ™“ª∞„I/Oø⁄
	GpioMuxRegs.GPADIR.bit.GPIOA11=1;		//∞—GPIOA11…Ë÷√Œ™ ‰≥ˆ 
	EDIS;
	GpioDataRegs.GPADAT.bit.GPIOA11=0; 	//GPIOA11=0;∏√∂Àø⁄Œ™74HC595À¯¥Ê–≈∫≈				
}

//SPI≥ı ºªØ◊”≥Ã–Ú
void spi_intial()
 {
 	SpiaRegs.SPICCR.all =0x0047;   //  πSPI¥¶”⁄∏¥Œª∑Ω Ω, œ¬Ωµ—ÿ, ∞ÀŒª ˝æ›  
	SpiaRegs.SPICTL.all =0x0006;   // ÷˜øÿƒ£ Ω, “ª∞„ ±÷”ƒ£ Ω, πƒ‹talk, πÿ±’SPI÷–∂œ.
	SpiaRegs.SPIBRR =0x007F;       //≈‰÷√≤®Ãÿ¬ 
	SpiaRegs.SPICCR.all =SpiaRegs.SPICCR.all|0x0080;  // ÕÀ≥ˆ∏¥Œª◊¥Ã¨	 
	EALLOW;	
    GpioMuxRegs.GPFMUX.all=0x000F;	// …Ë÷√Õ®”√“˝Ω≈Œ™SPI“˝Ω≈	 	
    EDIS;
  }

void  CAN_INIT()
{
	struct ECAN_REGS ECanaShadow;  
	EALLOW;
  	GpioMuxRegs.GPFMUX.bit.CANTXA_GPIOF6 = 1; //  …Ë÷√GPIOF6Œ™CANTX 
  	GpioMuxRegs.GPFMUX.bit.CANRXA_GPIOF7 = 1; //  …Ë÷√GPIOF7Œ™CANRX    	
	EDIS;
/*eCAN øÿ÷∆ºƒ¥Ê∆˜–Ë“™32Œª∑√Œ °£»Áπ˚œÎœÚ“ª∏ˆµ•∂¿ŒªΩ¯–––¥≤Ÿ◊˜£¨±‡“Î∆˜ø…ƒ‹ª· π∆‰Ω¯»Î16Œª∑√Œ °£’‚∂˘“˝”√¡À“ª÷÷Ω‚æˆ∑Ω∑®£¨æÕ «”√”∞◊”ºƒ¥Ê∆˜∆» πΩ¯––32Œª∑√Œ °£ ∞—’˚∏ˆºƒ¥Ê∆˜∂¡»Î“ª∏ˆ”∞◊”ºƒ¥Ê∆˜°£ ’‚∏ˆ∑√Œ Ω´ «32Œªµƒ°£”√32Œª–¥≤Ÿ◊˜∏ƒ±‰–Ë“™∏ƒµƒŒª£¨»ª∫Û∞—∏√÷µøΩ±¥ªÿeCANºƒ¥Ê∆˜*/
    EALLOW; 
   	ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all; //  ∞—CANTIOC∂¡»Î”∞◊”ºƒ¥Ê∆˜
   	ECanaShadow.CANTIOC.bit.TXFUNC = 1;              //  Õ‚≤ø“˝Ω≈I/O πƒ‹±Í÷æŒª°£
//  TXFUNC£Ω1  CANTX“˝Ω≈±ª”√”⁄CAN∑¢ÀÕπ¶ƒ‹°£
//  TXFUNC£Ω0  CANTX“˝Ω≈±ª◊˜Œ™Õ®”√I/O“˝Ω≈±ª π”√ 
	ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all; //  ∞—≈‰÷√∫√µƒºƒ¥Ê∆˜÷µªÿ–¥
    ECanaShadow.CANRIOC.all = ECanaRegs.CANRIOC.all;  //  ∞—CANRIOC∂¡”∞◊”ºƒ¥Ê∆˜
	ECanaShadow.CANRIOC.bit.RXFUNC = 1;              //  Õ‚≤ø“˝Ω≈I/O πƒ‹±Í÷æŒª°£
//  RXFUNC£Ω1  CANRX“˝Ω≈±ª”√”⁄CANΩ” ’π¶ƒ‹°£
//  RXFUNC£Ω0  CANRX“˝Ω≈±ª◊˜Œ™Õ®”√I/O“˝Ω≈±ª π”√°£
    ECanaRegs.CANRIOC.all = ECanaShadow.CANRIOC.all;  //  ∞—≈‰÷√∫√µƒºƒ¥Ê∆˜÷µªÿ–¥
    EDIS;
//  ‘⁄≈‰÷√” œ‰ID÷µ÷Æ«∞£¨CANME∂‘”¶µƒŒª±ÿ–Î∏¥Œª£¨
//  »Áπ˚CANMEºƒ¥Ê∆˜÷–∂‘”¶µƒŒª±ª÷√Œª£¨‘ÚID–¥»Î≤Ÿ◊˜Œﬁ–ß°£
	ECanaRegs.CANME.all = 0;                 //  ∏¥ŒªÀ˘”–µƒ” œ‰
   	ECanaMboxes.MBOX0.MSGID.all = 0x15100000;  //  ≈‰÷√∑¢ÀÕ” œ‰0µƒID£∫¿©’π±Í ∂∑˚29Œª  
    ECanaMboxes.MBOX16.MSGID.all = 0x15200000; //  »∑∂®Ω” ’” œ‰16ID 
    //  ∞—” 0°´15 ≈‰÷√Œ™∑¢ÀÕ” œ‰ £¨ ∞—” œ‰16°´31 ≈‰Œ™Ω” ’” œ‰
    ECanaRegs.CANMD.all = 0xFFFF0000; 
    ECanaRegs.CANME.all = 0xFFFFFFFF;         //  CANƒ£øÈ πƒ‹∂‘”¶µƒ” œ‰£¨
        
    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;       //  ∞—∑¢ÀÕ£¨Ω” ’ ˝æ›µƒ≥§∂»∂®“ÂŒ™8Œª
    ECanaMboxes.MBOX0.MSGCTRL.bit.RTR = 0;       //  Œﬁ‘∂≥Ã÷°«ÎÛ  
//  “ÚŒ™RTRŒª‘⁄∏¥Œª∫Û◊¥Ã¨≤ª∂®£¨“Ú¥À‘⁄≥Ã–ÚΩ¯––≥ı ºªØµƒ ±∫Ú±ÿ–Î∂‘∏√Œª∏≥÷µ°£
    ECanaMboxes.MBOX1.MSGCTRL.bit.RTR = 0;
//  ∞—¥˝∑¢ÀÕµƒ ˝æ›–¥»Î∑¢ÀÕ” œ‰ 
 	ECanaMboxes.MBOX0.MDL.all = 0x55555555;
	ECanaMboxes.MBOX0.MDH.all = 0x55555501;	 
	EALLOW;
//  ” œ‰÷–∂œ∆¡±Œºƒ¥Ê∆˜°£…œµÁ∫ÛÀ˘”–µƒ÷–∂œ∆¡±ŒŒª∂º«Â¡„«“Õ£÷π÷–∂œ πƒ‹°£
//  ’‚–©Œª‘ –Ì∂¿¡¢∆¡±Œ»Œ∫Œ” œ‰÷–∂œ°£
	ECanaRegs.CANGIM.all = 0x0001;     //÷–∂œ0 πƒ‹
	ECanaRegs.CANMIM.all = 0xFFFFFFFF;  
//  CANMIM .BIT.X£Ω1  ” œ‰÷–∂œ±ª πƒ‹£®X£Ω1°´31£©
//  CANMIM .BIT.X£Ω0  ” œ‰÷–∂œ±ªΩ˚÷π£®X£Ω1°´31£©
	ECanaShadow.CANMC.all = ECanaRegs.CANMC.all; //  ∞—CANMC∂¡»Î”∞◊”ºƒ¥Ê∆˜
	ECanaShadow.CANMC.bit.CCR = 1;               //  ∏ƒ±‰≈‰÷√«Î«ÛŒª
	ECanaShadow.CANMC.bit.SCB = 1;  			//eCANƒ£ Ω,À˘”–” œ‰ πƒ‹
	ECanaRegs.CANMC.all = ECanaShadow.CANMC.all; //  ∞—≈‰÷√∫√µƒºƒ¥Ê∆˜÷µªÿ–¥
  	EDIS;
/*CPU“™«Û∂‘≈‰÷√ºƒ¥Ê∆˜CANBTC∫ÕSCCµƒΩ” ’∆¡±Œºƒ¥Ê∆˜(CANGAM£¨LAM[0]∫ÕLAM[3])Ω¯–––¥≤Ÿ◊˜°£∂‘∏√Œª÷√Œª∫Û£¨CPU±ÿ–Îµ»¥˝£¨÷±µΩCANESºƒ¥Ê∆˜µƒCCE±Í÷æŒª‘⁄ÀÕ»ÎCANBTCºƒ¥Ê∆˜÷Æ«∞Œ™1 */
do 
    {
      ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    } while(ECanaShadow.CANES.bit.CCE != 1 );  //  µ±CCE£Ω1 ±ø…“‘∂‘CANBTCΩ¯––≤Ÿ◊˜°£
    //  ≈‰÷√≤®Ãÿ¬ 
    EALLOW;
    ECanaShadow.CANBTC.all = ECanaRegs.CANBTC.all; //  ∞—CANBTC∂¡»Î”∞◊”ºƒ¥Ê∆˜
    ECanaShadow.CANBTC.bit.BRPREG = 149;   //  (BRP+1)£Ω150£¨ ◊Ó–° ±º‰µ•ŒªTQ£Ω1us
    ECanaShadow.CANBTC.bit.TSEG2REG = 2;   //  Œª∂® ±bit-time£Ω(TSEG1+1)+(TSEG1+1)+1
    ECanaShadow.CANBTC.bit.TSEG1REG = 3;   //  bit-time£Ω8us£¨ À˘“‘≤®Ãÿ¬ Œ™125Kpbs       
    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;  //  ∞—≈‰÷√∫√µƒºƒ¥Ê∆˜÷µªÿ–¥
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;   //  ∞—CANMC∂¡»Î”∞◊”ºƒ¥Ê∆˜
    ECanaShadow.CANMC.bit.CCR = 0 ;       //  …Ë÷√CCR£Ω0£¨ CPU«Î«Û’˝≥£ƒ£ Ω
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;   //  ∞—≈‰÷√∫√µƒºƒ¥Ê∆˜÷µªÿ–¥
    EDIS;
    do
    {
      ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    } while(ECanaShadow.CANES.bit.CCE != 0 );  //  µ»¥˝ CCE Œª±ª«Â¡„
//  ≈‰÷√eCANŒ™◊‘≤‚ ‘ƒ£ Ω£¨ πƒ‹eCANµƒ‘ˆ«øÃÿ–‘
    EALLOW;
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.STM = 0;        //  ≈‰÷√CAN Œ™’˝≥£ƒ£ Ω
  	// CANMC.bit.STM£Ω0£¨’˝≥£ƒ£ Ω£¨CANMC.bit.STM£Ω1£¨◊‘≤‚ ‘ƒ£ Ω
    ECanaShadow.CANMC.bit.SCB = 1;        // —°‘ÒHECCπ§◊˜ƒ£ Ω
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
    EDIS;
}

void mailbox_read(int16 MBXnbr)
{
  	volatile struct MBOX *Mailbox;
	Mailbox = &ECanaMboxes.MBOX0 + MBXnbr;
	TestMbox1 = Mailbox->MDL.all;    	// ∂¡≥ˆµ±«∞” œ‰ ˝æ›µÕ4◊÷Ω⁄
	TestMbox2 = Mailbox->MDH.all;       //  ∂¡≥ˆµ±«∞” œ‰ ˝æ›∏ﬂ4◊÷Ω⁄
	TestMbox3 = Mailbox->MSGID.all;     //  ∂¡≥ˆµ±«∞” œ‰ID
} 

void LEDdisplay(int LED8,int LED7,int LED6,int LED5,int LED4,int LED3,int LED2,int LED1)
{
   
    GpioDataRegs.GPADAT.bit.GPIOA11=0; //∏¯LACK–≈∫≈“ª∏ˆµÕµÁ∆Ω
	Write_LED (LED8);//œÚLED◊Ó∏ﬂŒª–¥ ˝æ›,œ‘ æLED8
	Write_LED (LED7);
	Write_LED (LED6);
	Write_LED (LED5);
	Write_LED (LED4);
	Write_LED (LED3);
	Write_LED (LED2);
	Write_LED (LED1);//œÚLED◊ÓµÕŒª–¥ ˝æ›,œ‘ æLED1

    GpioDataRegs.GPADAT.bit.GPIOA11=1; //∏¯LACK–≈∫≈“ª∏ˆ∏ﬂµÁ∆ΩŒ™À¯¥Ê74HC595     	
} 

void main(void)
{		
    asm (" NOP ");
   	InitSysCtrl();      //≥ı ºªØœµÕ≥øÿ÷∆ºƒ¥Ê∆˜,  ±÷”∆µ¬ 150M
	DINT;	        //πÿ±’◊‹÷–∂œ£¨«Â≥˝÷–∂œ±Í÷æ
	IER = 0x0000;   //πÿ±’Õ‚Œß÷–∂œ
	IFR = 0x0000;	//«Â÷–∂œ±Í÷æ
	spi_intial();   //SPI≥ı ºªØ◊”≥Ã–Ú   
	IOinit();	    // I/O≥ı ºªØ◊”≥Ã
   	CAN_INIT();	//≥ı ºªØSCl
	InitPieCtrl(); //≥ı ºªØPIEøÿ÷∆ºƒ¥Ê∆˜ 
	InitPieVectTable(); //≥ı ºªØPIE÷–∂œœÚ¡ø±Ì
	LEDdisplay(0,0,0,19,19,0,0,0);

//T4£∫∞¥º¸ £¨≥ı ºªØ∞¥º¸ºÏ≤‚÷–∂œ
	EvbRegs.T4PR=12000;	//T4∂® ±∆˜÷‹∆⁄
	EvbRegs.T4CON.all=0x1744;	//x£Ø128£¨‘ˆº∆ ˝10£Æ24 ms
	EvbRegs.T4CNT=0x0000;//T4º∆ ˝∆˜«Â

	EvbRegs.EVBIMRB.bit.T4PINT=1;	// πƒ‹T4PINT÷–∂œ
	EvbRegs.EVBIFRB.bit.T4PINT=1;	//œ»«Â±Í÷æ
	InitXintf();	//≥ı ºªØXINTF
//####÷–∂œ πƒ‹
	EALLOW;
	PieVectTable.T4PINT=&T4pint_isr;//PIE÷–∂œ ∏¡ø±Ì	
	PieVectTable.ECAN0INTA=&Ecan0inta_isr;
	EDIS;
	PieCtrlRegs.PIEIER5.bit.INTx1=1;	// πƒ‹T4PINT	
	PieCtrlRegs.PIEIER9.bit.INTx5=1;	// πƒ‹ECAN0lNT

	IER |=( M_INT5 |M_INT9 );// πƒ‹INT5£¨INT9

	EINT;	//ø™∑≈»´æ÷÷–∂œ
	ERTM;	// πƒ‹»´æ÷ µ ±÷–∂œ

//÷˜—≠ª∑
   for(;;){;}
}

interrupt void   T4pint_isr(void)
	{	
 	    if (KeyIn1() == 1)     // µ˜”√≤Èº¸K1~K8◊”≥Ã–Ú
 	    {
 	    keyNum=keyNum+1; 	 
	    KeyFunction1(KeyReg1);
	
		if(keyNum>3)
		{
		keyNum=keyNum-3;
		LED8=LEDReg;
		}
 	    if(keyNum==1)
		{
		LED8=LEDReg;
		}
		 if(keyNum==2)
		{
		LED7=LEDReg;
		}
		 if(keyNum==3)
		{
		LED6=LEDReg;
		} 	
		transmitChar=LED8*256+LED7*16+LED6;//◊™ªª≥… ÆΩ¯÷∆µƒ÷µ
 		LEDdisplay(LED8,LED7,LED6,19,19,LED3,LED2,LED1); //œ‘ æ∑¢ÀÕŒª-√√-Ω” ’Œª****

		EvbRegs.EVBIFRB.bit.T4PINT=1;	//÷–∂œ±Í÷æ«Â
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //‘ –Ìœ¬¥Œ÷–∂œ
		return;
		}

		if (KeyIn2() == 1)     // µ˜”√≤Èº¸K8~K16◊”≥Ã–Ú
 	    {
 	    KeyFunction2(KeyReg2);	

	if(LEDReg==0x0B)		////≈– «∑ÒŒ™π¶ƒ‹Bº¸ CAN∑¢ÀÕ
		{	 	
		ECanaMboxes.MBOX0.MDL.all = 0x55555502;//∑¢ÀÕ±Í÷æ
		ECanaMboxes.MBOX0.MDH.all = transmitChar;
		ECanaRegs.CANTRS.all=0x0001; //∑¢ÀÕ0” œ‰ 
		while(ECanaRegs.CANTA.all!=0x0001) {};
		ECanaRegs.CANTA.all=0xFFFF;		//–¥1«Â.	
	 
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//÷–∂œ±Í÷æ«Â
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //‘ –Ìœ¬¥Œ÷–∂œ
		return;
		}
 	   	keyNum=keyNum+1;		
		if(keyNum>3)
		{
		keyNum=keyNum-3;
		LED8=LEDReg;
		}
 	    if(keyNum==1)
		{
		LED8=LEDReg;
		}
		 if(keyNum==2)
		{
		LED7=LEDReg;
		}
		 if(keyNum==3)
		{
		LED6=LEDReg;
		}
 		
		transmitChar=LED8*256+LED7*16+LED6;//◊™ªª≥… ÆΩ¯÷∆µƒ÷µ
 		LEDdisplay(LED8,LED7,LED6,19,19,LED3,LED2,LED1); //œ‘ æ∑¢ÀÕŒª-√√-Ω” ’Œª
		EvbRegs.EVBIFRB.bit.T4PINT=1;	//÷–∂œ±Í÷æ«Â
		PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //‘ –Ìœ¬¥Œ÷–∂œ
		return; 		
	}
	EvbRegs.EVBIFRB.bit.T4PINT=1;	//÷–∂œ±Í÷æ«Â
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP5; //‘ –Ìœ¬¥Œ÷–∂œ
	return;
}
interrupt void Ecan0inta_isr(void)	//Ω” ’–≈œ¢–¸π“÷–∂œ
{
	if(ECanaRegs.CANRMP.all==0x00010000)
	 {
	ECanaRegs.CANRMP.all=0xFFFF0000;
 	mailbox_read(16);
	RecieveChar=TestMbox2;
	LED3=RecieveChar/256;
	LED2=(RecieveChar%256)/16;
	LED1=RecieveChar%16;
	LEDdisplay(LED8,LED7,LED6,19,19,LED3,LED2,LED1);	
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //‘ –Ìœ¬¥Œ÷–∂œ	
	return;
	}
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP9; //‘ –Ìœ¬¥Œ÷–∂œ
	return;
}	

int KeyIn1(void)
{
 	 EALLOW;  
    //Ω´GPIOB8~GPIOB15≈‰÷√Œ™ ‰»Î,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all&0x00ff;
     EDIS;    
    GpioDataRegs.GPEDAT.all = 0xfff8;     //—°Õ®KEYµÕ8Œª
    for (i=0; i<100; i++){}               //—” ±
    //≈–K1~K8 «∑Ò∞¥œ¬
    if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
    {
    	for (i=0; i<40000; i++){}        //—” ±œ˚∂∂
        if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
        {
	   KeyReg1=GpioDataRegs.GPBDAT.all|0x00ff ;
    		while ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff) //≈– «∑ÒÀ…ø™
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
    //Ω´GPIOB8~GPIOB15≈‰÷√Œ™ ‰»Î,D0~D7
	GpioMuxRegs.GPBDIR.all = GpioMuxRegs.GPBDIR.all&0x00ff;
     EDIS;    
    GpioDataRegs.GPEDAT.all = 0xfff9;     //—°Õ®KEY∏ﬂ8Œª
    for (i=0; i<100; i++){}               //—” ±
    //≈–s8~s16 «∑Ò∞¥œ¬
    if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
    {
    	for (i=0; i<40000; i++){}        //—” ±œ˚∂∂
        if ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff)
        {
	   KeyReg2=GpioDataRegs.GPBDAT.all|0x00ff ;
    		while ((GpioDataRegs.GPBDAT.all|0x00ff)!=0xffff) //≈– «∑ÒÀÕø™
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

//Õ®π˝SPIœÚLED∑¢ÀÕœ‘ æ ˝æ›
void Write_LED (int LEDReg)
{
Uint16 LEDcode[30]={0xc000,0xf900,0xA400,0xB000,0x9900,0x9200,0x8200,0xF800,
                    0x8000,0x9000,0x8800,0x8300,0xc600,0xa100,0x8600,0x8e00,
                    0x8c00,0xbf00,0xa700,0xff00,0x4000,0x7900,0x2400,0x3000,
                    0x1900,0x1200,0x0200,0x7800,0x0000,0x1000};//π≤—Ù◊÷–Œ¬Î0~f, P£¨£≠£¨L£¨"√",0.~9.	
	 		SpiaRegs.SPITXBUF =LEDcode[LEDReg]; //∏¯ ˝¬Îπ‹ÀÕ ˝
    	 	 while(SpiaRegs.SPISTS.bit.INT_FLAG != 1){} 		
    		SpiaRegs.SPIRXBUF = SpiaRegs.SPIRXBUF; //∂¡≥ˆ«Â±Í÷æ
}
//=========================================================================================
// No more.
//=========================================================================================
