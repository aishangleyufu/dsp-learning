;���� Array1 �� Array2:
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
    	C28OBJ				 	; ʹ��C28xĿ��ģʽ
    	C28ADDR 				; ʹ�� C28x Ѱַģʽ,AMODE=0
   		.c28_amode 				; ���߻�������� C28x Ѱַģʽ
    	C28MAP 					; ʹ��M0 �� M1ģ���C28x ӳ��

		MOVL XAR2,#Array1 		; XAR2 ָ��ָ�� Array1
		MOVL XAR3,#Array2 		; XAR3ָ��ָ��Array2
		MOV @AR0,#(N-1) 		; �ظ�loopѭ�� N��
Loop:
		MOVL ACC,*XAR2++ 		; ACC = Array1[i]
		MOVL *XAR3++,ACC 		; Array2[i] = ACC
		BANZ Loop,AR0-- 		; ��AR0 != 0,���� Loop��AR0??
		NOP