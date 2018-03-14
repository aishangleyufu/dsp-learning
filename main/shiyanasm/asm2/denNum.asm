; 有符号数计算: Quot16 = Num16/Den16, Rem16 = Num16%Den16
;分子 Num16,分母Den16,余数Rem16,商Quot16
 	.bss  	Den16,1
	.bss  	Num16,1
	.bss  	Rem16,1
	.bss   	Quot16,1
 	.def  	_c_int00
      
RESET: .long  _c_int00

   	.text
 
_c_int00:
    	C28OBJ				 	; 使能C28x目标模式
    	C28ADDR 				; 使能 C28x 寻址模式,AMODE=0
   		.c28_amode 				; 告诉汇编器处于 C28x 寻址模式
    	C28MAP 					; 使能M0 和 M1模块的C28x 映射
 
		MOV DP,#8000H>>6		;装载数据页面指针;数据页指针偏移量6位
		CLRC TC 				;符号标志位TC清0
		MOV ACC,@Den16 << 16 	; AH = Den16, AL = 0
		ABSTC ACC 				; 取绝对值, TC = sign异或 TC
		MOV T,@AH  			     ;在T寄存器中保存Den16
		MOV ACC,@Num16 << 16 	; AH = Num16, AL = 0
		ABSTC ACC 				; 取绝对值, TC = sign异或TC
		MOVU ACC,@AH 			; AH = 0, AL = Num16
		RPT #15 ||				; 重复下一条指令16次
		SUBCU ACC,@T 			; 用Den16条件减
		MOV @Rem16,AH 			; 保存余数到 Rem16
		MOV ACC,@AL << 16 		; AH = Quot16, AL = 0
		NEGTC ACC 				; 若 TC = 1，则求负
		MOV @Quot16,AH 			; 保存商到Quot16
		.end