

; 使用双重 16位乘法计算乘积的和
; int16 X[N] ; 定义数据
; int16 C[N] ; 定义系数(位于低 4M)
; ;数据和系数必须位于偶地址
; ; N必须为偶数
; sum = 0;
; for(i=0; i < N; i++)
; sum = sum + (X[i] * C[i]) >> 5;


X .set  0x8000
C .set  0x8020
sum .set 0x8040 
N 		.set   20
	.def  	_c_int00
      
RESET: .long  _c_int00

   	.text
 
_c_int00:
    	C28OBJ				 	; 使能C28x目标模式
    	C28ADDR 				; 使能 C28x 寻址模式,AMODE=0
   		.c28_amode 				; 告诉汇编器处于 C28x 寻址模式
    	C28MAP 					; 使能M0 和 M1模块的C28x 映射
		MOVL XAR2,#X 				; XAR2指针指向 X
		MOVL XAR7,#C 				; XAR7指针指向 C
		SPM -5 						; 设置乘积右移5位 ">> 5"
		ZAPA 						; 清 ACC, P, OVC
		RPT #((N/2)-1)				; 重复下一条指令N/2 次
		||DMAC P,*XAR2++,*XAR7++ 	; ACC = ACC + (X[i+1] * C[i+1]) >> 5
							; P = P + (X[i] * C[i]) >> 5 i++
		ADDL ACC,@P 				; 执行最后计算
		MOVL @sum,ACC 				;保存结果到 sum

