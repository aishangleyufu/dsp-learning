; �з���������: Quot16 = Num16/Den16, Rem16 = Num16%Den16
;���� Num16,��ĸDen16,����Rem16,��Quot16
 	.bss  	Den16,1
	.bss  	Num16,1
	.bss  	Rem16,1
	.bss   	Quot16,1
 	.def  	_c_int00
      
RESET: .long  _c_int00

   	.text
 
_c_int00:
    	C28OBJ				 	; ʹ��C28xĿ��ģʽ
    	C28ADDR 				; ʹ�� C28x Ѱַģʽ,AMODE=0
   		.c28_amode 				; ���߻�������� C28x Ѱַģʽ
    	C28MAP 					; ʹ��M0 �� M1ģ���C28x ӳ��
 
		MOV DP,#8000H>>6		;װ������ҳ��ָ��;����ҳָ��ƫ����6λ
		CLRC TC 				;���ű�־λTC��0
		MOV ACC,@Den16 << 16 	; AH = Den16, AL = 0
		ABSTC ACC 				; ȡ����ֵ, TC = sign��� TC
		MOV T,@AH  			     ;��T�Ĵ����б���Den16
		MOV ACC,@Num16 << 16 	; AH = Num16, AL = 0
		ABSTC ACC 				; ȡ����ֵ, TC = sign���TC
		MOVU ACC,@AH 			; AH = 0, AL = Num16
		RPT #15 ||				; �ظ���һ��ָ��16��
		SUBCU ACC,@T 			; ��Den16������
		MOV @Rem16,AH 			; ���������� Rem16
		MOV ACC,@AL << 16 		; AH = Quot16, AL = 0
		NEGTC ACC 				; �� TC = 1������
		MOV @Quot16,AH 			; �����̵�Quot16
		.end