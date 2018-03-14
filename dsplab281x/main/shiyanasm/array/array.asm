;复制 Array1 到 Array2:
; int32 Array1[N];
; int32 Array2[N];
; for(i=0; i < N; i++)
; Array2[i] = Array1[i];
Array1 .set  0x8000
Array2 .set  0x8100
N 		.set    100
	.def  	_c_int00
      
RESET: .long  _c_int00

   	.text
 
_c_int00:
    	C28OBJ				 	; 使能C28x目标模式
    	C28ADDR 				; 使能 C28x 寻址模式,AMODE=0
   		.c28_amode 				; 告诉汇编器处于 C28x 寻址模式
    	C28MAP 					; 使能M0 和 M1模块的C28x 映射

		MOVL XAR2,#Array1 		; XAR2 指针指向 Array1
		MOVL XAR3,#Array2 		; XAR3指针指向Array2
		MOV @AR0,#(N-1) 		; 重复loop循环 N次
Loop:
		MOVL ACC,*XAR2++ 		; ACC = Array1[i]
		MOVL *XAR3++,ACC 		; Array2[i] = ACC
		BANZ Loop,AR0-- 		; 若AR0 != 0,跳至 Loop，AR0??
		NOP