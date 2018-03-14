/*******************************************************************************************
ÎÄ¼þ:   ECanSelf.c
¹¦ÄÜ:	²ÉÓÃ×Ô¼ì·½Ê½½øÐÐDSP281x eCAN Êý¾ÝµÄ»ØËÍºÍ½ÓÊÕ¡£		
		±¾²âÊÔ³ÌÐò²»Í£µØ¸ßËÙ»ØËÍÊý¾Ý£¬¶Ô½ÓÊÕµ½µÄÊý¾Ý½øÐÐÐ£Ñé£¬ÓÐ´í¼´Óè±êÊ¶¡£
		MBX0´«ËÍµ½MBX16, MBX1´«ËÍµ½MBX17 ....  ±¾³ÌÐò²ûÃ÷×Ô¼ì·½Ê½µÄÓ¦ÓÃ¡£
		ÁíÍâÔö¼ÓÁËled·¢¹â¶þ¼«¹ÜÏÔÊ¾(DS20¡«DS25)µÄ½Ó¿Ú¡£ÒÔ±ã·´Ó³eCANÍ¨ÐÅµÄ´ÎÊý
*******************************************************************************************/

#include "DSP281x_Device.h"     // DSP281x Í·ÎÄ¼þ°üº¬ÎÄ¼þ
#include "DSP281x_Examples.h"   // DSP281x Ê¾Àý°üº¬ÎÄ¼þ

	// ±¾ÎÄ¼þ½¨Á¢µÄº¯ÊýÔ­ÐÍÉùÃ÷
void mailbox_check(int32 T1, int32 T2, int32 T3);	
	// ¼ì²éÓÊÏä½ÓÊÕµÄÊý¾Ý
void mailbox_read(int16 i); 						
	// ¶Á³öÓÉ²ÎÊýMBXnbrÖ¸³öµÄÓÊÏäµÄÄÚÈÝ¡£
void WriteXram(Uint16 *StartAddr, Uint16 *EndAddr, Uint16 number);
	// ´ÓÖ¸¶¨µÄÍâ´æµØÖ·StartAddr¿ªÊ¼µ½EndAddr½áÊø£¬Ë³ÐòÐ´ÈënumberÊý¾Ý¡£
void WriteMDLH(Uint16 *StartAddr, Uint16 *EndAddr);
	// ÒÔ¸ú×Ù¼ÆÊýÆ÷TrackcountÎª»ùÊý£¬´ÓµÚÒ»¸ö·¢ËÍÓÊÏä¿ªÊ¼ÒÔTrackcount¼Ó1µÝÔöÊý´æÈë
	// ·¢ËÍÓÊÏäµÄ¸÷¸öµ¥Ôª(×Ü¹²64¸ö×Öµ¥Ôª)¡£
void CompMDLH(Uint16 *Comp1, Uint16 *Comp2);
	// ÔÚeCAN»ØËÍµÄÌõ¼þÏÂ£¬½«Ç°16¸ö·¢ËÍÓÊÏäµÄÊý¾ÝÓëºó16¸ö½ÓÊÕÓÊÏäµÄÊý¾ÝÒ»Ò»±È½Ï£¬
	// Èô²»µÈÓÉ´íÎó¼ÆÊýÆ÷ErrorCountÀÛ¼Æ¼ÆÊý¡£
void delay(Uint16 dly);

	// ±¾ÀýµÄÈ«¾Ö±äÁ¿
Uint32  ErrorCount=0;			// ´íÎó¼ÆÊýÆ÷
Uint32  MessageReceivedCount;	// ±¨ÎÄ½ÓÊÕ¼ÆÊýÆ÷

Uint32  TestMbox1 = 0;
Uint32  TestMbox2 = 0;
Uint32  TestMbox3 = 0;

Uint32	Trackcount=0;			// ·¢ËÍÓÊÏä¶¯Ì¬Êý¾Ý¸üÐÂ¸ú×Ù¼ÆÊýÆ÷¡£²ÉÓÃTrackcount¼Ó1
								// µÝÔöÐÎÊ½£¬ÒÀ´Î´æÈë16¸ö·¢ËÍÓÊÏä¹²64¸ö×Ö¡£ÏÂÒ»´Î·¢
								// ËÍÓÊÏäÊý¾Ý¸üÐÂ£¬ÒÔÇ°Ò»¸ö¶¯Ì¬µÄTrackcountÎª»ùÊý¡£

void main(void)
{
		//	eCAN¿ØÖÆ¼Ä´æÆ÷ÐèÓÃ32Î»Êý¾Ý½øÐÐ¶ÁÐ´·ÃÎÊ£¬Òò´ËÔÚ±¾ÀýÖÐ¹¹½¨ÁËÒ»×éÓ°×Ó¼Ä´æÆ÷£¬
		//	ÕâÖÖÓ°×Ó¼Ä´æÆ÷²ÉÓÃ32Î»Êý¾Ý·ÃÎÊ·½Ê½¶ø·Ç16Î»·½Ê½¡£
	struct ECAN_REGS ECanaShadow; 	
					// ¶¨ÒåECanaShadowÎª¾ßÓÐECAN_REGSÏàÍ¬ÀàÐÍµÄ½á¹¹Ìå±äÁ¿.

		//  ÏµÍ³¿ØÖÆ³õÊ¼»¯:
   	InitSysCtrl();	// ËøÏà»·(PLL),¿´ÃÅ¹·(WatchDog)¼°ÍâÉèÊ±ÖÓ(PeripheralClocks)³õÊ¼»¯¡£
					// InitSysCtrl()º¯ÊýÓÉDSP281x_SysCtrl.cÎÄ¼þ½¨Á¢¡£
  	EALLOW;			// ÔÊÐí·ÃÎÊÊÜ±£»¤µÄ¿Õ¼ä¡£

		//	±¾³ÌÐò²ÉÓÃGPIO¶àÂ·¸´ÓÃÆ÷ÅäÖÃCANÒý½Å
   	EALLOW;
   	GpioMuxRegs.GPFMUX.bit.CANTXA_GPIOF6 = 1;		
   					// µ±CANTXA_GPIOF6=0£¬½«GPIOF6ÅäÖÃ³ÉÆÕÍ¨I/O ¿Ú
					// µ±CANTXA_GPIOF6=1£¬½«GPIOF6ÅäÖÃ³ÉCANTXAÒý½Å
   	GpioMuxRegs.GPFMUX.bit.CANRXA_GPIOF7 = 1;
					// ½«GPIOF7ÅäÖÃ³ÉCANRXAÒý½Å
   	EDIS;
	//³õÊ¼»¯GPIO:
	EALLOW;	// ÔÊÐí·ÃÎÊÊÜ±£»¤µÄ¼Ä´æÆ÷
		//½«GPIOF8~GPIOF13ÅäÖÃÎªÒ»°ãI/O¿Ú£¬Êý×ÖÁ¿Êä³ö·¢¹â¶þ¼«¹ÜÏÔÊ¾(DS20¡«DS25)
 	GpioMuxRegs.GPFMUX.all = 0xc0ff;
    GpioMuxRegs.GPFDIR.all = 0x3f00; 		 
	EDIS;								// ½ûÖ¹·ÃÎÊÊÜ±£»¤µÄ¼Ä´æÆ÷
    GpioDataRegs.GPFDAT.all=0x5555;		// ¼ä¸ôÒ»¸öµãÁÁLED·¢¹â¶þ¼«¹Ü¡£
		
   	DINT;	//	¹ØCPUÖÐ¶Ï 
 
	IER = 0x0000;
	IFR = 0x0000;	// ¹ØCPUÖÐ¶Ï²¢ÇåËùÓÐCPUÖÐ¶Ï±êÊ¶·û¡£ 
	
    MessageReceivedCount = 0;
    ErrorCount = 0;

/*******************************************************************************************
	ÕâÀï¶ÔeCANÓÊÏäRAMÇøÓò0x6100-0x61FFÉèÖÃ³õÖµ0£¬Æä×÷ÓÃÎªÅäºÏ½ÓÊÕÓÊÏä¸³ÖµÊÔÑé¡£²Î¼û
ÏÂÃæ½ÓÊÕÓÊÏä¸³Öµ³ÌÐò¶ÎËµÃ÷¡£
	Êµ²Î"(Uint16 *)0x6100" ÎªWriteXramº¯ÊýÐÎ²Î"Uint16 *StartAddr" È·¶¨ÆðÊ¼µØÖ·Ö¸Õë
	Êµ²Î"(Uint16 *)0x6200" ÎªWriteXramº¯ÊýÐÎ²Î"Uint16 *EndAddr" È·¶¨½áÊøµØÖ·Ö¸Õë
*******************************************************************************************/
	WriteXram((Uint16 *)0x6100,(Uint16 *)0x6200,0x000000);

    
/*******************************************************************************************
32Î»·ÃÎÊ·½·¨½éÉÜ:

		eCAN¿ØÖÆºÍ×´Ì¬¼Ä´æÆ÷ÐèÓÃ32Î»Êý¾Ý½øÐÐ¶ÁÐ´·ÃÎÊ£¬Èç¹ûÐèÒªÐ´µ¥¶ÀµÄÒ»Î»»ò¼¸Î»£¬¿ÉÍ¨
	¹ýÒÔÏÂÈí¼þ²½ÖèÍê³É:

		(1) ÏÈ½¨Á¢Ò»¸ö¾ßÓÐ32Î»ECAN_REGS½á¹¹ÌåÀàÐÍµÄÓ°×Ó¼Ä´æÆ÷±äÁ¿ECanaShadow(¼û±¾ÎÄ¼þ
			µÄµÚÒ»ÌõÖ¸Áî "struct ECAN_REGS ECanaShadow;" )¡£ÓÉÓÚÔÚDSP281x_ECan.hÍ·ÎÄ¼þ
			ÖÐÒÑ¾­ÓÐÖ¸Áî "extern volatile struct ECAN_REGS ECanaRegs;",¹Ê³ÆÖ®ÎªÓ°×Ó¼Ä
			´æÆ÷¡£
			Èç¹ûÐèÒªÐ´µ¥¶ÀµÄÒ»Î»£¬ÀýÈç¶ÔCAN·¢ËÍIO¿ØÖÆ¼Ä´æÆ÷ CANTIOC µÄ TXFUNC Î»ÖÃ1£¬
			¿ÉÍ¨¹ýÒÔÏÂ3²½Íê³É¡£
		(2) ÒÔ32Î»ÐÎÊ½½«Õû¸ö¼Ä´æÆ÷¶ÁÈëÓ°×Ó¼Ä´æÆ÷:
				ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all;
		(3) ¸Ä±äÓ°×Ó¼Ä´æÆ÷ÖÐÒª²Ù×÷µÄÎ»:
    			ECanaShadow.CANTIOC.bit.TXFUNC = 1;
		(4) ½«Ó°×Ó¼Ä´æÆ÷»Ø¿½µ½´ø32Î»Ð´µÄeCAN¼Ä´æÆ÷ÖÐ
			    ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all;

		ÏÂÃæµÄ8ÌõÖ¸ÁîÅäÖÃeCANµÄRX¼°TXÒý½ÅÎªeCAN´«ËÍ·½Ê½
*******************************************************************************************/ 
    EALLOW;
    ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all;	// CANTIOC: CAN·¢ËÍIO¿ØÖÆ¼Ä´æÆ÷
    ECanaShadow.CANTIOC.bit.TXFUNC = 1;					// CANTXÒý½ÅÓÃÓÚCAN·¢ËÍ²Ù×÷	 (P590)
    ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all;

    ECanaShadow.CANRIOC.all = ECanaRegs.CANRIOC.all;	// CANRIOC: CAN½ÓÊÕIO¿ØÖÆ¼Ä´æÆ÷
    ECanaShadow.CANRIOC.bit.RXFUNC = 1;					// CANRXÒý½ÅÓÃÓÚCAN½ÓÊÕ²Ù×÷
    ECanaRegs.CANRIOC.all = ECanaShadow.CANRIOC.all;
    EDIS;
     
    ECanaRegs.CANME.all = 0;							// CANME:ÓÊÏäÊ¹ÄÜ¼Ä´æÆ÷¡£	 (P571)
    		// ¹ØËùÓÐÓÊÏä¡£ÓÉÓÚÊÇÐ´ÈëÕû¸ö¼Ä´æÆ÷(¶ø²»ÊÇÎ»Óò)£¬¹Ê²»ÐèÒªÓ°×Ó¼Ä´æÆ÷¡£

/*******************************************************************************************	
·¢ËÍÓÊÏäÏûÏ¢±êÊ¶·û¼Ä´æÆ÷(MSGID)¸³ÖµËµÃ÷£º

	(1) ±¾ÀýÍ¨¹ýºóÃæµÄÖ¸Áî"ECanaShadow.CANMC.bit.SCB = 1;"ÉèÖÃÎªeCAN·½Ê½£¬Ê¹ÓÃ32¸öÓÊÏä¡£
		²¢Í¨¹ýÖ¸Áî"ECanaRegs.CANMD.all = 0xFFFF0000;"½«Ç°16¸öÓÊÏäÉèÖÃÎªÊä³ö(ÓëµÍÎ»¶ÔÓ¦)£¬
		ºó16¸öÓÊÏäÉèÖÃÎªÊäÈë(Óë¸ßÎ»¶ÔÓ¦)¡£
	(2) Ò»¸öÓÐÓÃµÄ·¢ËÍÓÊÏäµÄ±êÊ¶·ûµÄ¸³Öµ±ØÐëÓëÄ³Ò»¸ö½ÓÊÕÓÊÏäµÄ±êÊ¶·ûµÄ¸³ÖµÏàµÈ(Æ¥Åä)£¬Êý
		¾ÝÍ¨ÐÅÊ±£¬Õâ¶ÔÓÊÏä²ÅÄÜ¹»ÎÕÊÖÁªÂç¡£Òò´Ë¿É°Ñ±êÊ¶·ûÊÓ×÷Í¨ÐÅÁªÂçÂë¡£
	(3) MSGIDÊÇÒ»¸ö32Î»µÄ¼Ä´æÆ÷£¬´Ó×î¸ßÎ»MSGID.31¿ªÊ¼£¬Ë³Ðò3Î»·Ö±ðÊÇ£º±êÊ¶·ûÀ©Õ¹Î» IDE,
		½ÓÊÕÆÁ±ÎÊ¹ÄÜÎ» AME ¼°×Ô¶¯Ó¦´ðÄ£Ê½Î»AAM¡£MSGID.28-MSGID.0 Îª´æ·ÅÍ¨ÐÅÁªÂçÂëµÄÏûÏ¢
		±êÊ¶·ûÎ»¡£
	(4) ±êÊ¶·ûÓÐ±ê×¼ºÍÀ©Õ¹Á½ÖÖÄ£Ê½£¬±ê×¼Õ¼ÓÃMSGID.28-18£¬À©Õ¹Õ¼ÓÃMSGID.28-0¡£µ±
		IDE=0Ê±£¬Ñ¡ÓÃ±ê×¼Ä£Ê½¡£µ±IDE=1Ê±£¬Ñ¡ÓÃÀ©Õ¹Ä£Ê½¡£±¾ÀýÎªÀ©Õ¹Ä£Ê½¡£
	(5) ½ÓÊÕÆÁ±ÎÊ¹ÄÜÎ» AME Ö»ÓÃÓÚ½ÓÊÕÏä£¬ÔÚAME=1Ê±ÓëÈ«¾Ö½ÓÊÕÆÁ±Î¼Ä´æÆ÷(CANGAM)ÁªºÏÊ¹ÓÃ¡£
		CANGAM.28-0ÊÇ¶ÔÓ¦ÓÚMSGID.28-0µÄÈ«¾Ö½ÓÊÕÆÁ±ÎÎ»£¬ÈôÄ³Î»ÖÃ1£¬Ôò¶ÔÓ¦ÆÁ±ÎMSGID.28-0µÄ
		Ä³Î»¡£±¾ÀýÈ¡AME=0£¬²»²ÉÓÃ½ÓÊÕÆÁ±Î¡£
	(6) ×Ô¶¯Ó¦´ðÄ£Ê½Î» AAM ½ö½ö¶ÔÅäÖÃÎª·¢ËÍ²Ù×÷µÄÏûÏ¢ÓÊÏäÓÐÐ§£¬¶ÔÓÚ½ÓÊÕÓÊÏä£¬¸ÃÎ»ÎÞÐ§¡£
		µ± AAM=1Ê±£¬Îª·¢ËÍ×Ô¶¯Ó¦´ðÄ£Ê½¡£´ËÊ±£¬Èô·¢ËÍ¶ËÊÕµ½Ò»¸öÆ¥ÅäµÄÔ¶¶ËÇëÇó£¬CANÄ£¿éÍ¨
		¹ý·¢ËÍ¸ÃÓÊÏäµÄÄÚÈÝÀ´Ó¦´ðÔ¶¶ËÇëÇó¡£µ± AAM=0Ê±£¬ÎªÕý³£·¢ËÍÄ£Ê½¡£·¢ËÍ¶ËÓÊÏä²»¶ÔÔ¶¶Ë
		ÇëÇó½øÐÐÓ¦´ð¡£±¾ÀýÈ¡AAM=0£¬ÉèÖÃÎªÕý³£·¢ËÍÄ£Ê½¡£
	(7) ·¢ËÍÓÊÏäÕ¼ÓÃÄÚ´æÇøÓò:0x006100-0x00617F
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
½ÓÊÕÓÊÏäÏûÏ¢±êÊ¶·û¼Ä´æÆ÷(MSGID)¸³ÖµËµÃ÷£º

	(1) ÎªÁË±ãÓÚÊý¾Ý¹Û²ì,ÕâÀï²ÉÓÃÁË½ÓÊÕÓÊÏäÏûÏ¢Óë·¢ËÍÓÊÏäÏûÏ¢ÉÏÏÂ¶ÔÓ¦ÏàµÈµÄ×ö·¨¡£
	(2) Ò»¸öÓÐÐ§µÄ½ÓÊÕÓÊÏäÏûÏ¢±êÊ¶·û¼Ä´æÆ÷MSGIDËù´æ´¢µÄÏûÏ¢£¬±ØÐëÓëÄ³Ò»¸ö·¢ËÍÓÊÏäÏûÏ¢
	    ±êÊ¶·û¼Ä´æÆ÷µÄÏûÏ¢ÏàµÈ¡£ÕâÑù£¬µ±·¢ËÍÓÊÏäµÄÒ»Ö¡Êý¾Ý(°üÀ¨±êÊ¶·û¼Ä´æÆ÷MSGIDËù´æ´¢
		µÄÏûÏ¢)±»·¢ËÍÖ®ºó£¬½ÓÊÕ¶Ë½«½ÓÊÕµ½µÄÃ¿Ò»¸ö·¢ËÍÓÊÏäµÄÏûÏ¢±êÊ¶·ûÊý¾ÝÓë½ÓÊÕÓÊÏäÏû
		Ï¢±êÊ¶·ûÊý¾Ý½øÐÐ±È½Ï£¬Èç¹ûÏàµÈ£¬Ôò½ÓÊÕµ½µÄ±êÊ¶·û¡¢¿ØÖÆÎ»¼°Êý¾Ý×Ö½ÚÐ´Èë¶ÔÓ¦µÄ½Ó
		ÊÕÓÊÏä,¼´Ä³Ò»RAMÇøÓò¡£Èç¹û²»µÈ£¬ÔòÕâÒ»Ö¡Êý¾Ý¶ªÆú£¬²»Óè´æ´¢¡£
	(3) Èô´æÔÚÒ»¸ö½ÓÊÕÓÊÏä£¬ÆäÏûÏ¢±êÊ¶·ûµÄÊý¾ÝÓë¸÷¸ö·¢ËÍÓÊÏäÏûÏ¢±êÊ¶·ûµÄÊý¾ÝÃ»ÓÐÒ»¸öÏà
		µÈ£¬ÔòÕâ¸ö½ÓÊÕÓÊÏä½«ÊÕ²»µ½ÈÎºÎÊý¾Ý¡£ 
	(4) ½ÓÊÕÓÊÏäÏûÏ¢±êÊ¶·û¼Ä´æÆ÷MSGIDµÄ³õÊ¼»¯Ã»ÓÐ´ÎÐòÉÏµÄÒªÇó¡£
	(5) ½ÓÊÕÓÊÏäÕ¼ÓÃÄÚ´æÇøÓò:0x006180-0x0061FF

		ÏÂÃæµÄ16ÌõÖ¸Áî¶Ô16¸ö½ÓÊÕÓÊÏä£¨MBOX16 - 31£©µÄÏûÏ¢±êÊ¶·û¼Ä´æÆ÷MSGID¸³Öµ¡£ÎªÁË¶ÔÉÏ
	ÃæµÄ¸ÅÄîÓÐ¸üÖ±¹ÛµÄÀí½â£¬¿É×öÏÂÃæ2¸öÊÔÑé£º

	(1) ½ÓÊÕÓÊÏäÓë·¢ËÍÓÊÏäµÄÆ¥ÅäÊÔÑé£º
		½«MBOX16µÄÏûÏ¢¸ÄÎª0x9555AAAF, MBOX31µÄÏûÏ¢¸ÄÎª0x9555AAA0.±àÒëÔËÐÐ³ÌÐòÖ®ºóÔÝÍ££¬¹Û
		²ìÓÊÏäRAMÇøÓò(View->Memory->0x00006100)£¬¿ÉÒÔ·¢ÏÖÔ­À´´æÈëµÚ17ºÅÓÊÏäMBOX16¼°µÚ32ºÅ
		ÓÊÏäµÄÊý¾Ý×÷ÁËµ÷»»¡£
	(2) ½ÓÊÕÓÊÏäÓë·¢ËÍÓÊÏäµÄ²»Æ¥ÅäÊÔÑé£º
		½«Èô¸É½ÓÊÕÓÊÏäµÄÏûÏ¢ÉèÖÃ³ÉÓë·¢ËÍÓÊÏäµÄÏûÏ¢²»Æ¥Åä£¬±àÒëÔËÐÐ³ÌÐòÖ®ºóÔÝÍ£¡£¿ÉÒÔ·¢ÏÖÕâ
		Ð©ÓÊÏä±£ÁôÇ°ÃæµÄ³õÊ¼»¯Êý¾Ý¡£
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

	//ECanaRegs.CANMD.all = 0x00000000;				// ½«32¸öÓÊÏäÉèÖÃ³É·¢ËÍÓÊÏä
    ECanaRegs.CANMD.all = 0xFFFF0000; 			// CANMD: ÓÊÏäÖ¸Ïò¼Ä´æÆ÷¡£			(P571)		
			// µ±CANMD.x=1£¬¶ÔÓ¦ÓÊÏä¶¨ÒåÎª½ÓÊÕÓÊÏä£¬µ±CANMD.x=0£¬¶ÔÓ¦ÓÊÏä¶¨ÒåÎª·¢ËÍÓÊÏä¡£
			// ÅäÖÃ0-15ÓÊÏäÎª·¢ËÍ£¬16-31ÓÊÏäÎª½ÓÊÕ¡£
    
    ECanaRegs.CANME.all = 0xFFFFFFFF;	// CANME: ÓÊÏäÊ¹ÄÜ¼Ä´æÆ÷					(P571)	
    		// ¶ÔÓÊÏäÊ¹ÄÜºó£¬ÏàÓ¦ÓÊÏäµÄ±êÊ¶·ûÐ´Èë²Ù×÷±»½ûÖ¹¡£Òò´Ë±êÊ¶·ûÐ´Èë±ØÐë
    		// ÔÚÓÊÏäÊ¹ÄÜÖ®Ç°	

/*******************************************************************************************
ÏûÏ¢¿ØÖÆ¼Ä´æÆ÷(MSGCTRL)Ê¹ÓÃËµÃ÷£º

	(1) ÓÊÏäÖÐ32Î»ÏûÏ¢¿ØÖÆ¼Ä´æÆ÷(MSGCTRL)ÓÐ3ÖÖÓÃÍ¾£º¶¨Òå×Ö½ÚÊý£¬·¢ËÍÓÅÏÈ¼¶¼°Ô¶³ÌÖ¡¹ÜÀí¡£
	(2) MSGCTRL.12-MSGCTRL.8Îª·¢ËÍÓÅÏÈ¼¶Î»TPL4:0¡£
		Õâ4Î»µÄÎ»Öµ£¬´Ó´óµ½Ð¡¾ö¶¨ÁË¸ÃÓÊÏä·¢ËÍµÄÓÅÏÈ¼¶¡£µ±ÓÅÏÈ¼¶ÏàÍ¬Ê±£¬¾ßÓÐ½Ï´óÐòºÅµÄÓÊÏä
		ÏÈ½øÐÐ·¢ËÍ²Ù×÷¡£TPLÖ»ÓÃÓÚ·¢ËÍÓÊÏä£¬¶øÇÒ²»ÔÚSCC(16ÓÊÏä)Ä£Ê½ÖÐÊ¹ÓÃ¡£±¾ÀýÃ»ÓÐ¶ÔÕâ4Î»
		½øÐÐÉèÖÃ£¬²ÉÓÃËæ»úÅäÖÃ£¬²»Ó°Ïì16ÓÊÏäÏûÏ¢µÄ·¢ËÍ¡£
	(3) MSGCTRL.3-MSGCTRL.0ÎªÊý¾Ý³¤¶È´úÂë¹ÜÀíÎ»DLC3:0¡£
		Ëü¾ö¶¨½øÐÐ·¢ËÍ»ò½ÓÊÕµÄÊý¾Ý×Ö½ÚÊý£¬×î´óÖ»ÄÜÉèÖÃ8¸ö×Ö½Ú¡£±¾ÀýÉèÖÃ8¸ö×Ö½Ú¡£
	(4) MSGCTRL.4ÎªÔ¶¶Ë·¢ËÍÇëÇóÎ» RTR¡£
		µ±RTR=1Ê±£¬¶ÔÓÚ½ÓÊÕÓÊÏä£¬Èç¹ûTRS±êÖ¾±»ÖÃÎ»£¬Ôò»á·¢ËÍÒ»¸öÔ¶³ÌÖ¡²¢ÇÒÓÃÍ¬Ò»¸öÓÊÏä½ÓÊÕ
		ÏàÓ¦µÄÊý¾ÝÖ¡¡£Ò»µ©Ô¶³ÌÖ¡±»·¢ËÍ³öÈ¥£¬ÓÊÏäµÄTRSÎ»¾Í»Ø±»CANÄ£¿éÇå0¡£¶ÔÓÚ·¢ËÍÓÊÏä£¬Èç¹û
		TRS±êÖ¾±»ÖÃÎ»£¬Ôò»á·¢ËÍÒ»¸öÔ¶³ÌÖ¡£¬µ«ÊÇ»áÓÃÁíÒ»¸öÓÊÏä½ÓÊÕÏàÓ¦µÄÊý¾ÝÖ¡¡£ÓÉÓÚÏµÍ³¸´Î»
		Ê±£¬RTR ÎªÒ»Ëæ»ú×´Ì¬£¬¶ø±¾ÀýÃ»ÓÐÔ¶³ÌÖ¡ÇëÇó£¬¹ÊÉèÖÃRTR=0¡£
*******************************************************************************************/
    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;				// ÉèÖÃÊý¾Ý³¤¶ÈÎª8¸ö×Ö½Ú£¬
    ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;				// ×î¸ßÖ»ÄÜÎª8¸ö×Ö½Ú¡£
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
    
		
    ECanaMboxes.MBOX0.MSGCTRL.bit.RTR = 0;  			// ÉèÖÃÎÞÔ¶³ÌÖ¡ÇëÇó¡£			   
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
    
/*	ÓÃWriteMDLH()¶¯Ì¬Êý¾Ý´úÌæ
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
    ECanaRegs.CANMIM.all = 0xFFFFFFFF;		// CANMIM£ºÓÊÏäÖÐ¶ÏÆÁ±Î¼Ä´æÆ÷¡£			(P589)			
			// ÓÊÏäÖÐ¶ÏÊ¹ÄÜ¡£
			// ÓÉÓÚÊÇÐ´ÈëÕû¸ö¼Ä´æÆ÷(¶ø²»ÊÇÎ»Óò)£¬¹Ê²»ÐèÒªÓ°×Ó¼Ä´æÆ÷¡£							  

 			// ÒªÇóÔÊÐí¸Ä±äÅäÖÃ¼Ä´æÆ÷:
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;   // CANMC,Ö÷¿ØÖÆ¼Ä´æÆ÷			(P578)
    ECanaShadow.CANMC.bit.CCR = 1;            
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
    EDIS;
    
			// Í¨¹ýµÈºòCCE±»ÖÃÎ»£¬µÈ´ýCPU×¼Óè¸ü¸ÄÅäÖÃ¼Ä´æÆ÷µÄÖµ
    do 
    {
      ECanaShadow.CANES.all = ECanaRegs.CANES.all;	// CANES£º´íÎóºÍ×´Ì¬¼Ä´æÆ÷		(P582)
    } while(ECanaShadow.CANES.bit.CCE != 1 );  
			// µ±CPU¶ÔÅäÖÃ¼Ä´æÆ÷½øÐÐÁËÐ´²Ù×÷Ê±£¬CCE=1£¬·ñÔòCCE=0¡£ÉÏÃæ3ÌõÖ¸ÁîµÄ
			// º¬ÒåÎª£ºµ±CCE=1Ê±£¬Ö´ÐÐÏÂÃæµÄÖ¸Áî,·ñÔòµÈ´ýCPU¸ü¸ÄÅäÖÃ¼Ä´æÆ÷µÄÖµ¡£    
    	
    		/******* Configure the eCAN timing  ÅäÖÃeCANÊ±ÖÓ¡£********/
    EALLOW;
    ECanaShadow.CANBTC.all = ECanaRegs.CANBTC.all;	// CANBTC: Î»¶¨Ê±ÅäÖÃ¼Ä´æÆ÷		(P580)
    ECanaShadow.CANBTC.bit.BRPREG = 9;
			// ²¨ÌØÂÊÔ¤¶¨±êÆ÷£¨BRPREG£©Óë CAN Ê±ÖÓÆµÂÊÓÐÒÔÏÂ¹ØÏµ£º
			// 		CAN clock = SYSCLKOUT/(BRPREG + 1)
			// CAN clock µÄµ¹ÊýÎªÊ±¼äÁ¿£¨TQ£©³¤¶È(ÖÜÆÚ)£¬Æä¹ØÏµÎª£º
			// 		TQ = £¨1/SYSCLKOUT£©*BRP  
			// ÆäÖÐ SYSCLKOUT Îª CPU Ê±ÖÓ£¬BRP=BRPREG+1
			// µ± SYSCLKOUT=150MHz, BRPREG=9Ê±£¬CAN Ê±ÖÓÆµÂÊ=150/10=15 MHz.
    ECanaShadow.CANBTC.bit.TSEG2REG = 5 ;
    ECanaShadow.CANBTC.bit.TSEG1REG = 7; 
			// CAN ²¨ÌØÂÊÓÉÏÂÊ½È·¶¨£º
			// ²¨ÌØÂÊ£¨bit rate£©= SYSCLKOUT/(BRP*(bit-time))
			// ÆäÖÐ SYSCLKOUT Îª CPU Ê±ÖÓ£¬BRP=BRPREG+1£¬bit-time=(TSEG1REG+1)+(TSEG2REG+1)+1
			// µ± SYSCLKOUT=150MHz, BRPREG=9, TSEG1REG = 7, TSEG2REG = 5 Ê±£¬
			// CAN ²¨ÌØÂÊ=150/(10*15)=1 MHz.										(P601)
    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;

			// ÉÏÃæ	EALLOW ÏÂµÄÖ¸Áî¸ü¸ÄÁËÎ»¶¨Ê±ÅäÖÃ¼Ä´æÆ÷ÅäÖÃ¡£ÏÂÃæ3ÌõÖ¸ÁîÏòCPU·¢
			// ³ö±ä»»ÅäÖÃÇëÇó¡£	
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.CCR = 0;            
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

    EDIS;

			// ÏÂÃæ3ÌõÖ¸ÁîÎªµÈ´ýCPU¶ÔÎ»¶¨Ê±ÅäÖÃ¼Ä´æÆ÷£¨CANBTC£©ÅäÖÃµÄ¸ü¸Ä¡£
    do 
    {
      ECanaShadow.CANES.all = ECanaRegs.CANES.all;	// CANES£º´íÎóºÍ×´Ì¬¼Ä´æÆ÷		(P582)
    } while(ECanaShadow.CANES.bit.CCE != 1 );  
			// µ±CPU¶ÔÅäÖÃ¼Ä´æÆ÷½øÐÐÁËÐ´²Ù×÷Ê±£¬CCE=1£¬·ñÔòCCE=0¡£ÉÏÃæ3ÌõÖ¸ÁîµÄ
			// º¬ÒåÎª: µ±CCE=0Ê±£¬µÈ´ýCPU¸ü¸ÄÅäÖÃ¼Ä´æÆ÷µÄÖµ;
			//         µ±CCE=1Ê±£¬Ö´ÐÐÏÂÃæµÄÖ¸Áî¡£   


			// ÅäÖÃ eCAN Îª×Ô¼ìÄ£Ê½£¬Ê¹ÄÜ eCAN µÄÔöÇ¿¹¦ÄÜ¡£
    EALLOW;
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;			// CANMC Ö÷¿ØÖÆ¼Ä´æÆ÷
/****************************************************************************************
×¢Òâ£º	ÆÁ±Î eCAN ×Ô¼ìÄ£Ê½Ö®ºó£¬Ê¾²¨Æ÷¿É¹Û²ìµ½Êä³ö²¨ÐÎ£¬µ«ÊÇ³ÌÐòÔÚÏÂÃæ
		while(ECanaRegs.CANTA.all != 0x0000FFFF ) {}  Ö¸Áî´¦Ì¤²½¡£²»ÆÁ±ÎÕâÌõÖ¸ÁîÔò
		Ê¾²¨Æ÷¹Û²ì²»µ½Êä³ö²¨ÐÎ¡£

		¸ù¾ÝÊµ¼ÊÊ¹ÓÃÇé¿öÓÐÒÔÏÂÅÐ¶Ï:
	(1)	µ±STM=1£¬¼´ÅäÖÃ eCAN Îª×Ô¼ìÄ£Ê½µÄÌõ¼þÏÂ£¬eCANÆÁ±Î¶ÔÍâ·¢ËÍÐÅºÅ¡£´ËÊ±£¬eCANµÄ·¢ËÍ
		ºÍ½ÓÊÕÔÚÏµÍ³ÄÚ²¿Íê³É¡£
	(2)	µ±STM=0£¬ÔÚ¹Ø±Õ×Ô¼ìÄ£Ê½µÄÌõ¼þÏÂ£¬eCAN¶ÔÍâ·¢ËÍÐÅºÅ£¬Ê¾²¨Æ÷¿É¹Û²ìµ½Êý¾ÝÊä³ö²¨ÐÎ¡£
		´ËÊ±£¬ÄÚ²¿½ûÖ¹½ÓÊÕ£¬CANTA¹¦ÄÜÊ§Ð§¡£Íâ²¿½ÓÊÕ»úµÄÇé¿öÈçºÎ´ýÊµÑéÖ®ºóÔÙ¿´¡£
****************************************************************************************/
  	ECanaShadow.CANMC.bit.STM = 1;    	// ÅäÖÃ eCAN Îª×Ô¼ìÄ£Ê½ 
    ECanaShadow.CANMC.bit.SCB = 1;    	// eCAN Ä£Ê½£¨ÔÊÐí·ÃÎÊ32Î»ÓÊÏä£©¡£

    ECanaRegs.CANMC.all= ECanaShadow.CANMC.all;
    EDIS;
    
			// ¿ªÊ¼´«ËÍ
//    while(1)
	for(;;) 	// ÈôÓÃwhile(1)×÷ÎÞÏÞÑ­»·£¬±àÒëÍ¨¹ý,µ«»á³öÏÖ±àÒë¾¯¸æ¡£¹Ê¸ÄÓÃfor(;;)                               
    {  
		WriteMDLH((Uint16 *)0x6104,(Uint16 *)0x617F);	// ¸Ä±ä·¢ËÍÓÊÏä´ý·¢ËÍµÄÊý¾Ý¡£
				// Ã¿¸öÓÊÏäÓÉÁ½²¿·Ö×é³É:ËüÃÇÊÇÎ»ÓÚÓÊÏäÇ°°ë²¿µÄ4×Ö8×Ö½ÚµÄÓÊÏä±êÊ¶·û£¬
				// ¼°Î»ÓÚÓÊÏäºó°ë²¿µÄ4×Ö8×Ö½ÚµÄÊý¾Ý¡£ÕâÌõÖ¸Áî½ö¸Ä±äÃ¿¸öÓÊÏäÊý¾ÝÇøµÄ
				// Êý¾Ý¡£
		
       	ECanaRegs.CANTRS.all = 0x0000FFFF;  			// CANTRS£º·¢ËÍÇëÇóÖÃÎ»¼Ä´æÆ÷
				// ¶ÔËùÓÐ·¢ËÍÓÊÏäÉèÖÃTRSÎ»,¼´ÖÃTRS[15:0]È«Îª1¡£16¸öÓÊÏä¿ªÊ¼·¢ËÍ¡£	(P572)

     	while(ECanaRegs.CANTA.all != 0x0000FFFF ) {}  // Wait for all TAn bits to be set..
				// Èç¹û CANTA µÄÄ³Î»±»ÖÃ1£¬Ôò¶ÔÓ¦ÓÊÏäµÄÏûÏ¢±»³É¹¦·¢ËÍ¡£ÕâÁ½ÌõÖ¸ÁîµÄº¬ÒåÎª
				// µÈ´ýTA[15:0]¾ù±äÎª1£¬¼´µÈ´ýËùÓÐÓÊÏäµÄÐÅÏ¢·¢ËÍ³öÈ¥¡£

       	ECanaRegs.CANTA.all = 0x0000FFFF;   
       			// Í¨¹ýÖÃÎ»ÇåTAn£¬×¼±¸ÅÐ±ðÏÂÒ»´Î·¢ËÍÓë·ñ¡£

       	MessageReceivedCount++;

		CompMDLH((Uint16 *)0x6104,(Uint16 *)0x6184);
				// ÔÚeCAN»ØËÍµÄÌõ¼þÏÂ£¬½«Ç°16¸ö·¢ËÍÓÊÏäµÄÊý¾ÝÓëºó16¸ö½ÓÊÕÓÊÏäµÄÊý¾ÝÒ»Ò»±È½Ï£¬
		  		// Èô²»µÈÓÉ´íÎó¼ÆÊýÆ÷ErrorCountÀÛ¼Æ¼ÆÊý¡£

		GpioDataRegs.GPFTOGGLE.all = 0xffff;	// F¶Ë¿ÚÈ¡·´

		delay(6);								// ÑÓÊ± ,´Ë´¦¿ÉÉèÌ½µã

				// ÓÃÉÏÃæµÄ"CompMDLH((Uint16 *)0x6104,(Uint16 *)0x6184);"
				// Ö¸Áî¼ì²éÕû¸ö½ÓÊÕÊý¾ÝµÄÕýÈ·ÐÔ¡£´úÌæÏÂÃæÆÁ±ÎµÄ¼ì²é³ÌÐò¡£
/*
       	for(j=0; j<16; j++)          						// ¶ÁÈ¡16¸ö½ÓÊÕÓÊÏäÐÅÏ¢²¢¼ì²éÊý¾Ý
       	{
          	mailbox_read(j); 								// ¶Á³öÓÉÊµ²ÎjÖ¸³öµÄÓÊÏäµÄÄÚÈÝ¡£
          	mailbox_check(TestMbox1,TestMbox2,TestMbox3); 	// ¼ì²éÓÊÏä½ÓÊÕµÄÊý¾Ý
       	}
*/
    }
}

/*******************************************************************************************
º¯ÊýÃû³Æ: WriteXram(Uint16 *StartAddr, Uint16 *EndAddr, Uint16 number)
º¯Êý¹¦ÄÜ: ´ÓÖ¸¶¨µÄÍâ´æµØÖ·StartAddr¿ªÊ¼µ½EndAddr½áÊø£¬Ë³ÐòÐ´ÈënumberÊý¾Ý¡£µ÷ÓÃ¸Ãº¯Êý
		  Ê±£¬¿ÉÔÚ*StartAddr++ =number;Ö¸ÁîÇ°ÉèÖÃ¶Ïµã£¬´ò¿ª¶ÔÓ¦µÄMemory´°¿Ú£¬°´F10¼ü£¬
		  ¿É¿´³ö number Êý¾Ý(µÝÔö)Ð´Èë¶ÔÓ¦µ¥Ôª¡£
ÊäÈë²ÎÊý: µÚÒ»ÐÎ²ÎStartAddrÎªÒ»Ö¸Õë±äÁ¿£¬ÒªÐ´ÈëÊý¾ÝµÄÊ×µØÖ·¡£
		  µÚ¶þÐÎ²ÎEndAddrÎªÒ»Ö¸Õë±äÁ¿£¬ÒªÐ´ÈëÊý¾ÝµÄ½áÊøµØÖ·¡£
		  µÚÈýÐÎ²ÎnumberÎªÐ´ÈëµÄÊý¾Ý¡£
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
º¯ÊýÃû³Æ: WriteMDLH(Uint16 *StartAddr, Uint16 *EndAddr)
º¯Êý¹¦ÄÜ: ÒÔ¸ú×Ù¼ÆÊýÆ÷TrackcountÎª»ùÊý£¬´ÓµÚÒ»¸ö·¢ËÍÓÊÏä¿ªÊ¼ÒÔTrackcount¼Ó1µÝÔöÊý´æÈë
		  ·¢ËÍÓÊÏäµÄ¸÷¸öµ¥Ôª(×Ü¹²64¸ö×Öµ¥Ôª)¡£
ÊäÈë²ÎÊý: µÚÒ»ÐÎ²ÎÎªÒ»Ö¸Õë±äÁ¿StartAddr£¬Êµ²ÎµØÖ·Ö¸ÕëÖ¸ÏòÊý¾Ý·¢ËÍÇøµÚÒ»¸ö·¢ËÍÓÊÏä¡£
		  µÚ¶þÐÎ²ÎÎªÒ»Ö¸Õë±äÁ¿EndAddr£¬Êµ²ÎµØÖ·Ö¸ÕëÖ¸ÏòÊý¾Ý½Ó·¢ËÍÇø×îºóÒ»¸ö·¢ËÍÓÊÏä¡£
*******************************************************************************************/
void WriteMDLH(Uint16 *StartAddr, Uint16 *EndAddr)
{
	Uint16 i;
	while(StartAddr < EndAddr)
    { 
       	for(i=0;i<4;i++) {*StartAddr++ =Trackcount++;}		// Ã¿×éÓÊÏä4¸ö×Ö¡£
		StartAddr=StartAddr+4;								// Ö¸ÏòÏÂÒ»×éÓÊÏäÊý¾ÝÊ×µØÖ·¡£
    }
    return;
}

/*******************************************************************************************
º¯ÊýÃû³Æ: CompMDLH(Uint16 *Comp1, Uint16 *Comp2)
º¯Êý¹¦ÄÜ: ÔÚeCAN»ØËÍµÄÌõ¼þÏÂ£¬½«Ç°16¸ö·¢ËÍÓÊÏäµÄÊý¾ÝÓëºó16¸ö½ÓÊÕÓÊÏäµÄÊý¾ÝÒ»Ò»±È½Ï£¬
		  Èô²»µÈÓÉ´íÎó¼ÆÊýÆ÷ErrorCountÀÛ¼Æ¼ÆÊý¡£
ÊäÈë²ÎÊý: µÚÒ»ÐÎ²ÎÎªÒ»Ö¸Õë±äÁ¿Comp1£¬Êµ²ÎµØÖ·Ö¸ÕëÖ¸ÏòÊý¾Ý·¢ËÍÇø¡£
		  µÚ¶þÐÎ²ÎÎªÒ»Ö¸Õë±äÁ¿Comp2£¬Êµ²ÎÖ·Ö¸ÕëÖ¸ÏòÊý¾Ý½ÓÊÕÇø¡£
*******************************************************************************************/
void CompMDLH(Uint16 *Comp1, Uint16 *Comp2)
{
	Uint16 i,j;
	for(i=0;i<16;i++)		// ´óÑ­»·£¬È¡16×éÓÊÏä
	{
		for(j=0;j<4;j++)	// Ð¡Ñ­»·£¬Ã¿×éº¬4¸ö×Ö£¬8¸ö×Ö½Ú¡£
		{	
			if(*Comp1++ != *Comp2++)  { ErrorCount++;}		
							// ÓÐÐËÈ¤¿É²âÊÔÒ»ÏÂÏàµÈ£¬´ò¿ª¹Û²ì´°ÊäÈëErrorCount½øÐÐ¹Û²ì¡£
		}
		Comp1=Comp1+4;		// È·¶¨ÏÂÒ»×éÆðÊ¼µØÖ·
		Comp2=Comp2+4;
	}
}

/****************************************************************************************
º¯ÊýÃû³Æ: 	mailbox_read(int16 MBXnbr)
º¯Êý¹¦ÄÜ: 	¶Á³öÓÉ²ÎÊýMBXnbrÖ¸³öµÄÓÊÏäµÄÄÚÈÝ¡£
ÊäÈëÐÎ²Î: 	MBXnbr ±íÊ¾Ñ¡ÔñµÄÓÊÏäºÅ¡£

×¢	  Òâ:	ÏÂÃæÁ½¸öº¯ÊýÎªTIÔ­Åäº¯Êý¡£ÓÉÓÚÏÖÔÚµÄÎÄ¼þÔÚ·¢ËÍÓÊÏäµÄÅäÖµÉÏ²ÉÈ¡±È½ÏÌùºÏ
			¼ÊµÄ¶¯Ì¬Êý¾Ý£¬ÁíÍâ²ÉÈ¡ÁË¶ÔÈ«²¿Í¨ÐÅÊý¾Ý½øÐÐ¼ì²âµÄ·½·¨¡£Òò´Ë£¬²»ÓÃÕâÁ½
			¸öº¯Êý¡£µ«ÊÇ£¬mailbox_readº¯ÊýµÄ¹¹Ë¼ÊÇÖµµÃÑ§Ï°µÄ¡£ËüÊÇÒ»¸ö¹ØÓÚÖ¸Õë±äÁ¿
			µÄ¼òµ¥Ã÷ÎúµÄ·¶Àý¡£			
****************************************************************************************/
void mailbox_read(int16 MBXnbr)
{
	volatile struct MBOX *Mailbox;	
   		// MBOX ÊÇÒ»¸ö½á¹¹ÌåÀàÐÍ(¼ûDSP281x_Ecan.h Í·ÎÄ¼þ)£¬ËüÓÉ±íÃ÷ÓÊÏä4¸ö32Î»¼Ä´æÆ÷
   		// ÌØÐÔµÄ4¸öÁªºÏÌå³ÉÔ±±äÁ¿×é³É¡£ÕâÀï¶¨ÒåMailboxÊÇÒ»¸ö¾ßÐMBOX½á¹¹ÌåÀàÐÍµÄ½á
   		// ¹¹ÌåÖ¸Õë±äÁ¿¡£
   	Mailbox = &ECanaMboxes.MBOX0 + MBXnbr;
		// Ö¸Õë±äÁ¿Mailbox È¡µÚÒ»¸ö·¢ËÍÓÊäµÄµØÖ· ECanaMboxes.MBOX0 ×÷Îª»ùÖ·ÔÙ¼ÓÉÏ
		// Æ«ÒÆÁ¿MBXnbr¡£
   	TestMbox1 = Mailbox->MDL.all; 	// = 0x9555AAAn (n is the MBX number)
   	TestMbox2 = Mailbox->MDH.all; 	// = 0x89ABCDEF (a constant)
   	TestMbox3 = Mailbox->MSGID.all;	// = 0x9555AAAn (n is the MBX number)

} // MSGID of a rcv MBX is transmitted as the MDL data.

/****************************************************************************************
º¯Êý³Æ: 	mailbox_check(int32 T1, int32 T2, int32 T3)
º¯Êý¹¦ÄÜ: 	¼ì²éÓÊÏä½ÓÊÕµÄÊý¾Ý
			Èç¹û T1!=T3 »òÕß T2!=0x89ABCDEF£¬¼´·¢ËÍÊý¾ÝÓë½ÓÊÕÊý¾Ý²»µÈ£¬Ö´ÐÐÑ­»·Ìå:
			´íÎó¼ÆÊýÆ÷¼Ó1; Èô¶¼ÏàµÈÔòÌø¹ýÑ­»·Ìå¡£
ÊäÈëÐÎ²Î: 	 
****************************************************************************************/
void mailbox_check(int32 T1, int32 T2, int32 T3)
{
	if((T1 != T3) || ( T2 != 0x89ABCDEF))	
    {
    	
       	ErrorCount++;
       	
    }
}

/*******************************************************************************************
º¯ÊýÃû³Æ: delay(Uint16 dly)
º¯Êý¹¦ÄÜ: ÑÓÊ±º¯Êý
ÊäÈë²ÎÊý: ÐÎ²Îdly£¬dlyÔ½´óÑÓÊ±Ô½¾Ã
Êä³ö²ÎÊý: ÎÞ
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
