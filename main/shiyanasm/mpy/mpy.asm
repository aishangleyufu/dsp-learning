

; ʹ��˫�� 16λ�˷�����˻��ĺ�
; int16 X[N] ; ��������
; int16 C[N] ; ����ϵ��(λ�ڵ� 4M)
; ;���ݺ�ϵ������λ��ż��ַ
; ; N����Ϊż��
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
    	C28OBJ				 	; ʹ��C28xĿ��ģʽ
    	C28ADDR 				; ʹ�� C28x Ѱַģʽ,AMODE=0
   		.c28_amode 				; ���߻�������� C28x Ѱַģʽ
    	C28MAP 					; ʹ��M0 �� M1ģ���C28x ӳ��
		MOVL XAR2,#X 				; XAR2ָ��ָ�� X
		MOVL XAR7,#C 				; XAR7ָ��ָ�� C
		SPM -5 						; ���ó˻�����5λ ">> 5"
		ZAPA 						; �� ACC, P, OVC
		RPT #((N/2)-1)				; �ظ���һ��ָ��N/2 ��
		||DMAC P,*XAR2++,*XAR7++ 	; ACC = ACC + (X[i+1] * C[i+1]) >> 5
							; P = P + (X[i] * C[i]) >> 5 i++
		ADDL ACC,@P 				; ִ��������
		MOVL @sum,ACC 				;�������� sum

